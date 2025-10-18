#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "task_base.h"
#include "queue_manager.h"
#include "config.h"

#ifdef USE_RTOS

// ==========================================
// CONSTANTS
// ==========================================
#define WIFI_OPERATION_QUEUE_LENGTH 10
#define WIFI_SCAN_TIMEOUT_MS 15000          // 15 seconds max for scan
#define WIFI_CONNECT_TIMEOUT_MS 30000       // 30 seconds max for connection
#define WIFI_DISCONNECT_TIMEOUT_MS 5000     // 5 seconds max for disconnection
#define MAX_CACHED_NETWORKS 50              // Maximum networks to cache from scan

// ==========================================
// WIFI OPERATION STRUCTURES
// ==========================================

/**
 * @brief WiFi operation types
 */
enum class WiFiOperationType {
    SCAN,           ///< Perform WiFi network scan
    CONNECT,        ///< Connect to network
    DISCONNECT,     ///< Disconnect from network
    START_AP,       ///< Start Access Point
    STOP_AP,        ///< Stop Access Point
    RECONNECT       ///< Reconnect to last network
};

/**
 * @brief WiFi operation request structure
 */
struct WiFiOperationRequest {
    WiFiOperationType type;
    String ssid;                ///< Network SSID (for CONNECT, START_AP)
    String password;            ///< Network password (for CONNECT, START_AP)
    uint32_t requestId;         ///< Unique request identifier
    uint32_t timestamp;         ///< Request timestamp
    bool async;                 ///< True for async operations with event notification
};

/**
 * @brief Cached WiFi scan result
 */
struct WiFiScanResult {
    String ssid;
    uint8_t bssid[6];
    int32_t rssi;
    uint8_t channel;
    wifi_auth_mode_t encryptionType;
    bool hidden;
};

/**
 * @brief WiFi connection state
 */
enum class WiFiState {
    UNINITIALIZED,  ///< WiFi not initialized
    IDLE,           ///< WiFi initialized, no operations
    SCANNING,       ///< Scan in progress
    CONNECTING,     ///< Connection attempt in progress
    CONNECTED,      ///< Successfully connected to network
    DISCONNECTING,  ///< Disconnection in progress
    AP_MODE,        ///< Operating as Access Point
    ERROR           ///< Error state
};

// ==========================================
// WIFI TASK CLASS
// ==========================================

/**
 * @brief WiFi Manager Task
 * 
 * Manages WiFi operations asynchronously using event-driven architecture.
 * All WiFi operations are non-blocking and notify subscribers via events.
 */
class WiFiTask : public TaskBase {
private:
    // State management
    WiFiState currentState;
    WiFiState previousState;
    unsigned long operationStartTime;
    uint32_t currentOperationId;
    
    // Scan result caching
    WiFiScanResult cachedResults[MAX_CACHED_NETWORKS];
    uint16_t cachedNetworkCount;
    unsigned long lastScanTime;
    bool scanInProgress;
    int16_t asyncScanId;  // ID returned by WiFi.scanNetworks(true)
    
    // Connection parameters
    String lastSSID;
    String lastPassword;
    
    // Operation queue
    QueueHandle_t operationQueue;
    
public:
    /**
     * @brief Construct a new WiFi Task
     */
    WiFiTask();
    
    /**
     * @brief Destroy the WiFi Task
     */
    virtual ~WiFiTask();
    
    /**
     * @brief Get current WiFi state
     * @return Current WiFiState
     */
    WiFiState getState() const { return currentState; }
    
    /**
     * @brief Get number of cached networks from last scan
     * @return Network count
     */
    uint16_t getCachedNetworkCount() const { return cachedNetworkCount; }
    
    /**
     * @brief Get cached scan result by index
     * @param index Network index (0-based)
     * @param result Output parameter for scan result
     * @return true if index valid and result copied
     */
    bool getCachedNetwork(uint16_t index, WiFiScanResult& result);
    
    /**
     * @brief Check if currently connected to network
     * @return true if connected
     */
    bool isConnected() const;
    
    /**
     * @brief Queue an operation request (internal use by public API)
     * @param request Operation request to queue
     * @return true if successfully queued
     */
    bool queueOperation(const WiFiOperationRequest& request);
    
protected:
    /**
     * @brief Setup function called once when task starts
     */
    void setup() override;
    
    /**
     * @brief Main task loop - processes WiFi operations and events
     */
    void loop() override;
    
    /**
     * @brief Cleanup function called when task stops
     */
    void cleanup() override;
    
private:
    /**
     * @brief Process WiFi operation requests from queue
     */
    void processOperationQueue();
    
    /**
     * @brief Process ongoing async WiFi scan
     */
    void processAsyncScan();
    
    /**
     * @brief Process WiFi connection state
     */
    void processConnectionState();
    
    /**
     * @brief Handle WiFi scan operation
     * @param request Operation request
     */
    void handleScanOperation(const WiFiOperationRequest& request);
    
    /**
     * @brief Handle WiFi connect operation
     * @param request Operation request
     */
    void handleConnectOperation(const WiFiOperationRequest& request);
    
    /**
     * @brief Handle WiFi disconnect operation
     * @param request Operation request
     */
    void handleDisconnectOperation(const WiFiOperationRequest& request);
    
    /**
     * @brief Handle start AP operation
     * @param request Operation request
     */
    void handleStartAPOperation(const WiFiOperationRequest& request);
    
    /**
     * @brief Handle stop AP operation
     * @param request Operation request
     */
    void handleStopAPOperation(const WiFiOperationRequest& request);
    
    /**
     * @brief Cache scan results from completed scan
     */
    void cacheScanResults();
    
    /**
     * @brief Send WiFi event to event queue
     * @param event WiFi event to send
     */
    void sendWiFiEvent(const WiFiEvent& event);
    
    /**
     * @brief Transition to new WiFi state
     * @param newState Target state
     */
    void transitionState(WiFiState newState);
    
    /**
     * @brief Check for operation timeout
     * @return true if current operation timed out
     */
    bool checkOperationTimeout();
    
    /**
     * @brief Get timeout duration for current operation
     * @return Timeout in milliseconds
     */
    uint32_t getOperationTimeout() const;
};

// ==========================================
// GLOBAL INSTANCE
// ==========================================
extern WiFiTask* wifiTask;

// ==========================================
// PUBLIC API
// ==========================================

/**
 * @brief Initialize and start the WiFi task
 * @return true if initialization successful
 */
bool initializeWiFiTask();

/**
 * @brief Stop and cleanup the WiFi task
 */
void shutdownWiFiTask();

/**
 * @brief Check if WiFi task is running
 * @return true if task is running
 */
bool isWiFiTaskRunning();

/**
 * @brief Queue a WiFi scan request
 * @param async If true, returns immediately and sends event when complete
 * @return true if request queued successfully
 */
bool queueScanRequest(bool async = true);

/**
 * @brief Queue a WiFi connect request
 * @param ssid Network SSID
 * @param password Network password
 * @param async If true, returns immediately and sends event when complete
 * @return true if request queued successfully
 */
bool queueConnectRequest(const String& ssid, const String& password, bool async = true);

/**
 * @brief Queue a WiFi disconnect request
 * @param async If true, returns immediately and sends event when complete
 * @return true if request queued successfully
 */
bool queueDisconnectRequest(bool async = true);

/**
 * @brief Queue a start AP request
 * @param ssid AP SSID
 * @param password AP password
 * @param async If true, returns immediately and sends event when complete
 * @return true if request queued successfully
 */
bool queueStartAPRequest(const String& ssid, const String& password, bool async = true);

/**
 * @brief Queue a stop AP request
 * @param async If true, returns immediately and sends event when complete
 * @return true if request queued successfully
 */
bool queueStopAPRequest(bool async = true);

/**
 * @brief Get current WiFi state
 * @return Current WiFiState
 */
WiFiState getWiFiTaskState();

/**
 * @brief Get number of cached networks
 * @return Network count from last scan
 */
uint16_t getWiFiCachedNetworkCount();

/**
 * @brief Get cached network by index (thread-safe)
 * @param index Network index (0-based)
 * @param result Output parameter for scan result
 * @return true if index valid and result retrieved
 */
bool getWiFiCachedNetwork(uint16_t index, WiFiScanResult& result);

/**
 * @brief Check if WiFi is currently connected
 * @return true if connected to a network
 */
bool isWiFiTaskConnected();

#endif // USE_RTOS
