#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "task_base.h"
#include "queue_manager.h"
#include "config.h"

#ifdef USE_RTOS
#ifdef USE_WEBSERVER

// ==========================================
// CONSTANTS
// ==========================================
// Note: WEB_REQUEST_QUEUE_LENGTH is defined in queue_manager.h
#define WEB_PAGE_RENDER_TIMEOUT_MS 100      // 100ms max for page rendering
#define WEB_TRIGGER_TIMEOUT_MS 50           // 50ms max for operation triggers
#define WEB_RESPONSE_BUFFER_SIZE 4096       // Pre-allocated buffer for HTML generation

// ==========================================
// WEB REQUEST STRUCTURES
// ==========================================

/**
 * @brief Web request types
 */
enum class WebRequestType {
    PAGE_RENDER,        ///< Render static HTML page
    SCAN_TRIGGER,       ///< Trigger WiFi scan operation
    STATUS_UPDATE,      ///< Generate status update page
    ANALYSIS_START,     ///< Start analysis operation
    IPERF_START,        ///< Start iPerf test
    IPERF_STOP,         ///< Stop iPerf test
    LATENCY_START,      ///< Start latency test
    LATENCY_STOP,       ///< Stop latency test
    CHANNEL_SCAN        ///< Channel scan operation
};

/**
 * @brief Page type for PAGE_RENDER requests
 */
enum class WebPageType {
    ROOT,               ///< Root/home page
    STATUS,             ///< Status page
    SCAN,               ///< Scan results page
    SCAN_DETAILS,       ///< Network details page
    NETWORK_ANALYSIS,   ///< Network analysis page
    CHANNEL_ANALYSIS,   ///< Channel analysis page
    LATENCY,            ///< Latency test page
    IPERF,              ///< iPerf test page
    IPERF_RESULTS,      ///< iPerf results page
    NOT_FOUND           ///< 404 page
};

/**
 * @brief Web request structure for WebTask
 */
struct WebTaskRequest {
    WebRequestType type;
    WebPageType pageType;       ///< For PAGE_RENDER requests
    String parameter;           ///< Optional parameter (e.g., network index for details)
    WebServer* server;          ///< Pointer to web server for response
    uint32_t requestId;         ///< Unique request identifier
    uint32_t timestamp;         ///< Request timestamp
    bool completed;             ///< Request completion flag
};

/**
 * @brief Web task state
 */
enum class WebTaskState {
    UNINITIALIZED,      ///< Task not initialized
    IDLE,               ///< Task running, waiting for requests
    PROCESSING_REQUEST, ///< Processing web request
    ERROR               ///< Error state
};

// ==========================================
// WEB TASK CLASS
// ==========================================

/**
 * @brief Web Server Task
 * 
 * Manages web requests asynchronously to prevent blocking during long operations.
 * All web requests are queued and processed in the task, allowing concurrent
 * request handling and integration with other tasks (WiFi, Analysis, etc.).
 */
class WebTask : public TaskBase {
private:
    // State management
    WebTaskState currentState;
    unsigned long requestStartTime;
    uint32_t currentRequestId;
    uint32_t nextRequestId;
    
    // Request queue
    QueueHandle_t requestQueue;
    
    // Response buffer
    char responseBuffer[WEB_RESPONSE_BUFFER_SIZE];
    
    // Request processing
    void processRequestQueue();
    bool processRequest(const WebTaskRequest& request);
    
    // Request handlers
    bool handlePageRender(const WebTaskRequest& request);
    bool handleScanTrigger(const WebTaskRequest& request);
    bool handleStatusUpdate(const WebTaskRequest& request);
    bool handleAnalysisStart(const WebTaskRequest& request);
    bool handleIperfStart(const WebTaskRequest& request);
    bool handleIperfStop(const WebTaskRequest& request);
    bool handleLatencyStart(const WebTaskRequest& request);
    bool handleLatencyStop(const WebTaskRequest& request);
    bool handleChannelScan(const WebTaskRequest& request);
    
    // Page renderers
    bool renderRootPage(WebServer* server);
    bool renderStatusPage(WebServer* server);
    bool renderScanPage(WebServer* server);
    bool renderScanDetailsPage(WebServer* server, const String& parameter);
    bool renderNetworkAnalysisPage(WebServer* server);
    bool renderChannelAnalysisPage(WebServer* server);
    bool renderLatencyPage(WebServer* server);
    bool renderIperfPage(WebServer* server);
    bool renderIperfResultsPage(WebServer* server);
    bool renderNotFoundPage(WebServer* server);
    
    // Helper functions
    void sendHTMLHeader(WebServer* server, const char* title);
    void sendHTMLFooter(WebServer* server);
    void sendQuickResponse(WebServer* server, const char* message, bool success = true);
    bool checkTimeout(unsigned long startTime, unsigned long timeoutMs);
    
    // State transitions
    void transitionState(WebTaskState newState);
    const char* stateToString(WebTaskState state);
    
protected:
    /**
     * @brief Task setup - Initialize web request handling
     */
    void setup() override;
    
    /**
     * @brief Task main loop - Process web requests
     */
    void loop() override;
    
    /**
     * @brief Task cleanup
     */
    void cleanup() override;

public:
    /**
     * @brief Constructor
     * @param name Task name
     * @param stackSize Stack size in bytes (default 8192)
     * @param priority Task priority (default 1 = HIGH)
     * @param core Core to run on (default 0 - WiFi Core)
     */
    WebTask(const char* name, 
            uint32_t stackSize = 8192,
            TaskPriority priority = static_cast<TaskPriority>(1),
            BaseType_t core = 0);
    
    /**
     * @brief Destructor
     */
    virtual ~WebTask();
    
    /**
     * @brief Queue a web request (thread-safe)
     * @param request Request to queue
     * @return true if request queued successfully
     */
    bool queueRequest(const WebTaskRequest& request);
    
    /**
     * @brief Get current task state
     * @return Current state
     */
    WebTaskState getState() const { return currentState; }
    
    /**
     * @brief Get request queue fill level
     * @return Number of requests in queue
     */
    uint32_t getQueuedRequestCount() const;
};

// ==========================================
// GLOBAL WEB TASK MANAGEMENT
// ==========================================

/**
 * @brief Initialize Web Task
 * @return true if initialization successful
 */
bool initializeWebTask();

/**
 * @brief Shutdown Web Task
 */
void shutdownWebTask();

/**
 * @brief Check if Web Task is running
 * @return true if running
 */
bool isWebTaskRunning();

/**
 * @brief Queue a web request (convenience function)
 * @param type Request type
 * @param pageType Page type (for PAGE_RENDER)
 * @param parameter Optional parameter
 * @param server Web server pointer
 * @return true if request queued successfully
 */
bool queueWebRequest(WebRequestType type, 
                     WebPageType pageType,
                     const String& parameter,
                     WebServer* server);

/**
 * @brief Queue a page render request
 * @param pageType Page to render
 * @param server Web server pointer
 * @param parameter Optional parameter
 * @return true if request queued successfully
 */
bool queuePageRender(WebPageType pageType, WebServer* server, const String& parameter = "");

/**
 * @brief Queue a scan trigger request
 * @param server Web server pointer
 * @return true if request queued successfully
 */
bool queueScanTrigger(WebServer* server);

/**
 * @brief Queue a status update request
 * @param server Web server pointer
 * @return true if request queued successfully
 */
bool queueStatusUpdate(WebServer* server);

/**
 * @brief Get Web Task state
 * @return Current state
 */
WebTaskState getWebTaskState();

/**
 * @brief Get queued request count
 * @return Number of requests in queue
 */
uint32_t getWebTaskQueuedRequests();

#endif // USE_WEBSERVER
#endif // USE_RTOS
