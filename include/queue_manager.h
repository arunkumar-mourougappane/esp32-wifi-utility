#pragma once

#include <Arduino.h>
#include "config.h"

#ifdef USE_RTOS
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <IPAddress.h>

// ==========================================
// QUEUE MANAGER
// ==========================================

/**
 * @file queue_manager.h
 * @brief RTOS Queue Management System
 * 
 * Provides centralized creation, management, and monitoring of all
 * FreeRTOS queues used for inter-task communication.
 * 
 * @version 4.1.0
 * @date 2025-10-17
 */

// ==========================================
// QUEUE CONFIGURATION
// ==========================================

#define COMMAND_QUEUE_LENGTH 10
#define WIFI_EVENT_QUEUE_LENGTH 20
#define ANALYSIS_RESULT_QUEUE_LENGTH 5
#define WEB_REQUEST_QUEUE_LENGTH 8
#define STATUS_QUEUE_LENGTH 15

#define QUEUE_WAIT_TIMEOUT_MS 100

// ==========================================
// QUEUE DATA STRUCTURES
// ==========================================

/**
 * @brief Command request structure
 * 
 * Represents a command to be executed asynchronously.
 */
struct CommandRequest {
    enum CommandType {
        WIFI_SCAN,
        WIFI_CONNECT,
        WIFI_DISCONNECT,
        AP_START,
        AP_STOP,
        CHANNEL_ANALYSIS,
        LATENCY_TEST,
        IPERF_TEST,
        STATUS_REQUEST,
        HELP_REQUEST,
        UNKNOWN
    } type;
    
    String commandString;      ///< Original command string
    String argument;           ///< Command argument (e.g., SSID)
    String secondArgument;     ///< Second argument (e.g., password)
    uint32_t requestId;        ///< Unique request identifier
    uint32_t timestamp;        ///< Request timestamp
};

/**
 * @brief WiFi event structure
 * 
 * Represents WiFi state changes and notifications.
 */
struct WiFiEvent {
    enum EventType {
        SCAN_STARTED,
        SCAN_COMPLETE,
        CONNECT_STARTED,
        CONNECTED,
        DISCONNECTED,
        AP_STARTED,
        AP_STOPPED,
        CLIENT_CONNECTED,
        CLIENT_DISCONNECTED,
        IP_ASSIGNED
    } type;
    
    union {
        struct {
            int networkCount;
            uint32_t scanDuration;
        } scanResult;
        
        struct {
            uint8_t mac[6];
            int8_t rssi;
        } clientInfo;
        
        struct {
            uint8_t ip[4];  // IP address as bytes instead of IPAddress object
        } ipInfo;
        
        uint8_t raw[16];  // For other data
    } data;
    
    uint32_t timestamp;
};

/**
 * @brief Analysis result structure
 * 
 * Contains results from long-running analysis operations.
 */
struct AnalysisResult {
    enum ResultType {
        CHANNEL_SCAN,
        LATENCY_TEST,
        IPERF_TEST,
        NETWORK_QUALITY
    } type;
    
    void* resultData;          ///< Pointer to specific result struct
    size_t dataSize;           ///< Size of result data in bytes
    bool success;              ///< Operation success status
    String errorMessage;       ///< Error message if failed
    uint32_t timestamp;        ///< Result timestamp
};

/**
 * @brief Web request structure
 * 
 * Represents an HTTP request to be processed.
 */
struct WebRequest {
    enum RequestType {
        PAGE_RENDER,
        SCAN_TRIGGER,
        STATUS_UPDATE,
        ANALYSIS_START,
        API_CALL
    } type;
    
    String path;               ///< Request path (e.g., "/scan")
    String queryParams;        ///< Query parameters
    void* serverRequest;       ///< Pointer to AsyncWebServerRequest
    uint32_t timestamp;        ///< Request timestamp
};

/**
 * @brief Status update structure
 * 
 * System-wide status messages and notifications.
 */
struct StatusUpdate {
    enum ComponentType {
        WIFI,
        WEB_SERVER,
        ANALYSIS,
        SYSTEM,
        LED
    } component;
    
    String message;            ///< Status message
    
    enum Level {
        INFO,
        WARNING,
        ERROR,
        SUCCESS
    } level;
    
    uint32_t timestamp;        ///< Update timestamp
};

// ==========================================
// QUEUE HANDLES
// ==========================================

extern QueueHandle_t commandQueue;
extern QueueHandle_t wifiEventQueue;
extern QueueHandle_t analysisResultQueue;
extern QueueHandle_t webRequestQueue;
extern QueueHandle_t statusQueue;

// ==========================================
// QUEUE STATISTICS
// ==========================================

/**
 * @brief Queue statistics structure
 */
struct QueueStatistics {
    uint32_t itemsSent;        ///< Total items sent to queue
    uint32_t itemsReceived;    ///< Total items received from queue
    uint32_t sendFailures;     ///< Number of failed send attempts
    uint32_t receiveFailures;  ///< Number of failed receive attempts
    uint32_t currentItems;     ///< Current number of items in queue
    uint32_t peakItems;        ///< Maximum items ever in queue
    uint32_t overflows;        ///< Number of queue overflow events
};

// ==========================================
// CORE FUNCTIONS
// ==========================================

/**
 * @brief Initialize all RTOS queues
 * 
 * Creates all FreeRTOS queues used for inter-task communication.
 * 
 * @return true if all queues created successfully, false otherwise
 */
bool initializeQueueManager();

/**
 * @brief Shutdown queue manager
 * 
 * Deletes all queues and frees resources.
 * 
 * @return true if shutdown successful, false otherwise
 */
bool shutdownQueueManager();

/**
 * @brief Get queue count
 * 
 * @return Number of created queues
 */
uint32_t getQueueCount();

/**
 * @brief Check queue health
 * 
 * Verifies all queues are valid and not overflowing.
 * 
 * @return true if queues are healthy, false otherwise
 */
bool checkQueueHealth();

// ==========================================
// COMMAND QUEUE FUNCTIONS
// ==========================================

/**
 * @brief Send command to command queue
 * 
 * @param command Command request to send
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if sent successfully, false if timeout or error
 */
bool sendCommand(const CommandRequest& command, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Receive command from command queue
 * 
 * @param command Output parameter for received command
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if received successfully, false if timeout or empty
 */
bool receiveCommand(CommandRequest& command, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Get number of pending commands
 * 
 * @return Number of commands waiting in queue
 */
uint32_t getPendingCommandCount();

/**
 * @brief Get command queue statistics
 * 
 * @return Queue statistics structure
 */
QueueStatistics getCommandQueueStatistics();

// ==========================================
// WIFI EVENT QUEUE FUNCTIONS
// ==========================================

/**
 * @brief Send WiFi event to event queue
 * 
 * @param event WiFi event to send
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if sent successfully, false if timeout or error
 */
bool sendWiFiEvent(const WiFiEvent& event, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Receive WiFi event from event queue
 * 
 * @param event Output parameter for received event
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if received successfully, false if timeout or empty
 */
bool receiveWiFiEvent(WiFiEvent& event, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Get number of pending WiFi events
 * 
 * @return Number of events waiting in queue
 */
uint32_t getPendingWiFiEventCount();

/**
 * @brief Get WiFi event queue statistics
 * 
 * @return Queue statistics structure
 */
QueueStatistics getWiFiEventQueueStatistics();

// ==========================================
// ANALYSIS RESULT QUEUE FUNCTIONS
// ==========================================

/**
 * @brief Send analysis result to result queue
 * 
 * @param result Analysis result to send
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if sent successfully, false if timeout or error
 */
bool sendAnalysisResult(const AnalysisResult& result, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Receive analysis result from result queue
 * 
 * @param result Output parameter for received result
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if received successfully, false if timeout or empty
 */
bool receiveAnalysisResult(AnalysisResult& result, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Get number of pending analysis results
 * 
 * @return Number of results waiting in queue
 */
uint32_t getPendingAnalysisResultCount();

/**
 * @brief Get analysis result queue statistics
 * 
 * @return Queue statistics structure
 */
QueueStatistics getAnalysisResultQueueStatistics();

// ==========================================
// WEB REQUEST QUEUE FUNCTIONS
// ==========================================

/**
 * @brief Send web request to request queue
 * 
 * @param request Web request to send
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if sent successfully, false if timeout or error
 */
bool sendWebRequest(const WebRequest& request, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Receive web request from request queue
 * 
 * @param request Output parameter for received request
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if received successfully, false if timeout or empty
 */
bool receiveWebRequest(WebRequest& request, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Get number of pending web requests
 * 
 * @return Number of requests waiting in queue
 */
uint32_t getPendingWebRequestCount();

/**
 * @brief Get web request queue statistics
 * 
 * @return Queue statistics structure
 */
QueueStatistics getWebRequestQueueStatistics();

// ==========================================
// STATUS QUEUE FUNCTIONS
// ==========================================

/**
 * @brief Send status update to status queue
 * 
 * @param status Status update to send
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if sent successfully, false if timeout or error
 */
bool sendStatusUpdate(const StatusUpdate& status, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Receive status update from status queue
 * 
 * @param status Output parameter for received status
 * @param timeoutMs Maximum time to wait (milliseconds)
 * @return true if received successfully, false if timeout or empty
 */
bool receiveStatusUpdate(StatusUpdate& status, uint32_t timeoutMs = QUEUE_WAIT_TIMEOUT_MS);

/**
 * @brief Get number of pending status updates
 * 
 * @return Number of status updates waiting in queue
 */
uint32_t getPendingStatusCount();

/**
 * @brief Get status queue statistics
 * 
 * @return Queue statistics structure
 */
QueueStatistics getStatusQueueStatistics();

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

/**
 * @brief Print all queue statistics
 * 
 * Prints statistics for all queues to serial console.
 */
void printQueueStatistics();

/**
 * @brief Reset queue statistics
 * 
 * Resets all statistics counters to zero.
 */
void resetQueueStatistics();

/**
 * @brief Flush all queues
 * 
 * Removes all items from all queues.
 * 
 * @warning This will discard all pending messages!
 */
void flushAllQueues();

/**
 * @brief Check if queue is full
 * 
 * @param queueHandle Handle to queue to check
 * @return true if queue is full, false otherwise
 */
bool isQueueFull(QueueHandle_t queueHandle);

/**
 * @brief Check if queue is empty
 * 
 * @param queueHandle Handle to queue to check
 * @return true if queue is empty, false otherwise
 */
bool isQueueEmpty(QueueHandle_t queueHandle);

#endif // USE_RTOS
