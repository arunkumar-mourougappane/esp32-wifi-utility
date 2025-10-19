#include "web_task.h"

#ifdef USE_RTOS
#ifdef USE_WEBSERVER

#include "rtos_manager.h"
#include "mutex_manager.h"
#include "wifi_task.h"
#include "web_server.h"
#include "wifi_manager.h"
#include "ap_manager.h"
#include "iperf_manager.h"
#include "latency_analyzer.h"
#include "channel_analyzer.h"

// ==========================================
// GLOBAL WEB TASK INSTANCE
// ==========================================
static WebTask* webTask = nullptr;

// ==========================================
// WEB TASK IMPLEMENTATION
// ==========================================

WebTask::WebTask(const char* name, uint32_t stackSize, TaskPriority priority, BaseType_t core)
    : TaskBase(name, stackSize, priority, core),
      currentState(WebTaskState::UNINITIALIZED),
      requestStartTime(0),
      currentRequestId(0),
      nextRequestId(1),
      requestQueue(nullptr) {
    
    // Create request queue
    requestQueue = xQueueCreate(WEB_REQUEST_QUEUE_LENGTH, sizeof(WebTaskRequest));
    if (requestQueue == nullptr) {
        Serial.println("[WebTask] ERROR: Failed to create request queue");
    }
    
    // Initialize response buffer
    memset(responseBuffer, 0, WEB_RESPONSE_BUFFER_SIZE);
}

WebTask::~WebTask() {
    if (requestQueue != nullptr) {
        vQueueDelete(requestQueue);
        requestQueue = nullptr;
    }
}

void WebTask::setup() {
    Serial.println("[WebTask] Initializing...");
    
    if (requestQueue == nullptr) {
        Serial.println("[WebTask] ERROR: Request queue not created");
        transitionState(WebTaskState::ERROR);
        return;
    }
    
    // Transition to IDLE state
    transitionState(WebTaskState::IDLE);
    
    Serial.println("[WebTask] Initialized successfully");
    Serial.printf("[WebTask] - Request queue: %d slots\n", WEB_REQUEST_QUEUE_LENGTH);
    Serial.printf("[WebTask] - Response buffer: %d bytes\n", WEB_RESPONSE_BUFFER_SIZE);
    Serial.printf("[WebTask] - Core: %d (WiFi Core)\n", xPortGetCoreID());
}

void WebTask::loop() {
    // Process request queue
    processRequestQueue();
    
    // Small delay to prevent tight loop
    vTaskDelay(pdMS_TO_TICKS(10));
}

void WebTask::cleanup() {
    Serial.println("[WebTask] Cleaning up...");
    transitionState(WebTaskState::UNINITIALIZED);
}

// ==========================================
// REQUEST QUEUE PROCESSING
// ==========================================

void WebTask::processRequestQueue() {
    WebTaskRequest request;
    
    // Check if there's a request in the queue (non-blocking)
    if (xQueueReceive(requestQueue, &request, 0) == pdTRUE) {
        transitionState(WebTaskState::PROCESSING_REQUEST);
        requestStartTime = millis();
        currentRequestId = request.requestId;
        
        // Process the request
        bool success = processRequest(request);
        
        if (!success) {
            Serial.printf("[WebTask] WARNING: Request %u processing failed\n", request.requestId);
        }
        
        // Return to IDLE state
        transitionState(WebTaskState::IDLE);
        currentRequestId = 0;
    }
}

bool WebTask::processRequest(const WebTaskRequest& request) {
    // Route request to appropriate handler
    switch (request.type) {
        case WebRequestType::PAGE_RENDER:
            return handlePageRender(request);
            
        case WebRequestType::SCAN_TRIGGER:
            return handleScanTrigger(request);
            
        case WebRequestType::STATUS_UPDATE:
            return handleStatusUpdate(request);
            
        case WebRequestType::ANALYSIS_START:
            return handleAnalysisStart(request);
            
        case WebRequestType::IPERF_START:
            return handleIperfStart(request);
            
        case WebRequestType::IPERF_STOP:
            return handleIperfStop(request);
            
        case WebRequestType::LATENCY_START:
            return handleLatencyStart(request);
            
        case WebRequestType::LATENCY_STOP:
            return handleLatencyStop(request);
            
        case WebRequestType::CHANNEL_SCAN:
            return handleChannelScan(request);
            
        default:
            Serial.printf("[WebTask] ERROR: Unknown request type %d\n", (int)request.type);
            return false;
    }
}

// ==========================================
// REQUEST HANDLERS
// ==========================================

bool WebTask::handlePageRender(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    // Route to appropriate page renderer
    switch (request.pageType) {
        case WebPageType::ROOT:
            return renderRootPage(request.server);
            
        case WebPageType::STATUS:
            return renderStatusPage(request.server);
            
        case WebPageType::SCAN:
            return renderScanPage(request.server);
            
        case WebPageType::SCAN_DETAILS:
            return renderScanDetailsPage(request.server, request.parameter);
            
        case WebPageType::NETWORK_ANALYSIS:
            return renderNetworkAnalysisPage(request.server);
            
        case WebPageType::CHANNEL_ANALYSIS:
            return renderChannelAnalysisPage(request.server);
            
        case WebPageType::LATENCY:
            return renderLatencyPage(request.server);
            
        case WebPageType::IPERF:
            return renderIperfPage(request.server);
            
        case WebPageType::IPERF_RESULTS:
            return renderIperfResultsPage(request.server);
            
        case WebPageType::NOT_FOUND:
            return renderNotFoundPage(request.server);
            
        default:
            Serial.printf("[WebTask] ERROR: Unknown page type %d\n", (int)request.pageType);
            return renderNotFoundPage(request.server);
    }
}

bool WebTask::handleScanTrigger(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    Serial.println("[WebTask] Triggering WiFi scan...");
    
    // Queue WiFi scan request to WiFi Task
    bool queued = queueScanRequest(true);  // Async scan
    
    if (queued) {
        // Send immediate response - scan will complete asynchronously
        sendQuickResponse(request.server, "WiFi scan started. Results will be available shortly.", true);
        Serial.println("[WebTask] Scan request queued successfully");
    } else {
        sendQuickResponse(request.server, "Failed to queue WiFi scan. Please try again.", false);
        Serial.println("[WebTask] ERROR: Failed to queue scan request");
    }
    
    return queued;
}

bool WebTask::handleStatusUpdate(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    // Render status page (same as STATUS page render)
    return renderStatusPage(request.server);
}

bool WebTask::handleAnalysisStart(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    Serial.println("[WebTask] Starting network analysis...");
    
    // Start channel analysis (non-blocking)
    startChannelMonitoring(30);  // 30 second interval
    sendQuickResponse(request.server, "Channel analysis started. Check channel analysis page for results.", true);
    
    return true;
}

bool WebTask::handleIperfStart(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    Serial.println("[WebTask] Starting iPerf server...");
    
    IperfConfig config = getDefaultConfig();
    config.mode = IPERF_SERVER;
    bool started = startIperfServer(config);
    
    if (started) {
        sendQuickResponse(request.server, "iPerf server started. Use iPerf client to test.", true);
    } else {
        sendQuickResponse(request.server, "Failed to start iPerf server. Check connection.", false);
    }
    
    return started;
}

bool WebTask::handleIperfStop(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    Serial.println("[WebTask] Stopping iPerf test...");
    
    stopIperfTest();
    sendQuickResponse(request.server, "iPerf test stopped.", true);
    
    return true;
}

bool WebTask::handleLatencyStart(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    Serial.println("[WebTask] Starting latency test...");
    
    // Check if we have a target (could be passed in parameter)
    String target = request.parameter.isEmpty() ? "8.8.8.8" : request.parameter;
    
    LatencyConfig config;
    config.target_host = target;
    config.packet_count = 10;
    config.interval_ms = 1000;
    config.timeout_ms = 5000;
    config.test_type = LATENCY_UDP_ECHO;
    config.packet_size = 64;
    config.target_port = 0;
    config.continuous_mode = false;
    
    bool started = startLatencyTest(config);
    
    if (started) {
        String msg = "Latency test started to ";
        msg += target;
        sendQuickResponse(request.server, msg.c_str(), true);
    } else {
        sendQuickResponse(request.server, "Failed to start latency test. Check connection.", false);
    }
    
    return started;
}

bool WebTask::handleLatencyStop(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    Serial.println("[WebTask] Stopping latency test...");
    
    stopLatencyTest();
    sendQuickResponse(request.server, "Latency test stopped.", true);
    
    return true;
}

bool WebTask::handleChannelScan(const WebTaskRequest& request) {
    if (request.server == nullptr) {
        Serial.println("[WebTask] ERROR: Server pointer is null");
        return false;
    }
    
    Serial.println("[WebTask] Starting channel scan...");
    
    startChannelMonitoring(30);  // 30 second interval
    sendQuickResponse(request.server, "Channel scan started.", true);
    
    return true;
}

// ==========================================
// PAGE RENDERERS
// ==========================================
// Note: These delegate to the original web_server.cpp handlers
// to maintain compatibility and avoid code duplication

bool WebTask::renderRootPage(WebServer* server) {
    handleRoot();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderStatusPage(WebServer* server) {
    handleStatus();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderScanPage(WebServer* server) {
    handleScan();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderScanDetailsPage(WebServer* server, const String& parameter) {
    // The handleScanDetails() function in web_server.cpp reads the network index from server->arg()
    // We need to ensure the parameter is available
    handleScanDetails();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderNetworkAnalysisPage(WebServer* server) {
    handleNetworkAnalysis();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderChannelAnalysisPage(WebServer* server) {
    handleChannelAnalysis();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderLatencyPage(WebServer* server) {
    handleLatency();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderIperfPage(WebServer* server) {
    handleIperf();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderIperfResultsPage(WebServer* server) {
    handleIperfResults();  // Call original handler from web_server.cpp
    return true;
}

bool WebTask::renderNotFoundPage(WebServer* server) {
    handleNotFound();  // Call original handler from web_server.cpp
    return true;
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

void WebTask::sendQuickResponse(WebServer* server, const char* message, bool success) {
    if (server == nullptr) return;
    
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta http-equiv='refresh' content='2;url=/'>"; // Redirect after 2 seconds
    html += "<title>";
    html += success ? "Success" : "Error";
    html += "</title>";
    html += "<style>body{font-family:Arial;margin:40px;text-align:center;}";
    html += ".message{padding:20px;border-radius:5px;display:inline-block;}";
    html += ".success{background-color:#d4edda;color:#155724;border:1px solid #c3e6cb;}";
    html += ".error{background-color:#f8d7da;color:#721c24;border:1px solid #f5c6cb;}";
    html += "</style></head><body>";
    html += "<div class='message ";
    html += success ? "success" : "error";
    html += "'>";
    html += message;
    html += "</div>";
    html += "<p><a href='/'>Back to Home</a></p>";
    html += "</body></html>";
    
    server->send(success ? 200 : 500, "text/html", html);
}

bool WebTask::checkTimeout(unsigned long startTime, unsigned long timeoutMs) {
    return (millis() - startTime) >= timeoutMs;
}

// ==========================================
// STATE MANAGEMENT
// ==========================================

void WebTask::transitionState(WebTaskState newState) {
    if (currentState != newState) {
        Serial.printf("[WebTask] State: %s -> %s\n", 
                     stateToString(currentState), 
                     stateToString(newState));
        currentState = newState;
    }
}

const char* WebTask::stateToString(WebTaskState state) {
    switch (state) {
        case WebTaskState::UNINITIALIZED:       return "UNINITIALIZED";
        case WebTaskState::IDLE:                return "IDLE";
        case WebTaskState::PROCESSING_REQUEST:  return "PROCESSING_REQUEST";
        case WebTaskState::ERROR:               return "ERROR";
        default:                                return "UNKNOWN";
    }
}

// ==========================================
// PUBLIC API
// ==========================================

bool WebTask::queueRequest(const WebTaskRequest& request) {
    if (requestQueue == nullptr) {
        Serial.println("[WebTask] ERROR: Request queue not initialized");
        return false;
    }
    
    // Try to send to queue with 100ms timeout
    if (xQueueSend(requestQueue, &request, pdMS_TO_TICKS(100)) == pdTRUE) {
        return true;
    } else {
        Serial.println("[WebTask] WARNING: Request queue full, dropping request");
        return false;
    }
}

uint32_t WebTask::getQueuedRequestCount() const {
    if (requestQueue == nullptr) return 0;
    return uxQueueMessagesWaiting(requestQueue);
}

// ==========================================
// GLOBAL WEB TASK MANAGEMENT
// ==========================================

bool initializeWebTask() {
    if (webTask != nullptr) {
        Serial.println("[WebTask] Already initialized");
        return true;
    }
    
    // Create WebTask instance with HIGH priority on Core 0 (WiFi Core)
    // Note: Using static_cast to avoid HIGH macro conflict
    webTask = new WebTask("WebTask", 8192, static_cast<TaskPriority>(0x1), 0);
    if (webTask == nullptr) {
        Serial.println("[WebTask] ERROR: Failed to create WebTask instance");
        return false;
    }
    
    // Start the task
    if (!webTask->start()) {
        Serial.println("[WebTask] ERROR: Failed to start WebTask");
        delete webTask;
        webTask = nullptr;
        return false;
    }
    
    Serial.println("[WebTask] Started successfully");
    return true;
}

void shutdownWebTask() {
    if (webTask == nullptr) {
        Serial.println("[WebTask] Not running");
        return;
    }
    
    Serial.println("[WebTask] Shutting down...");
    webTask->stop();
    delete webTask;
    webTask = nullptr;
    Serial.println("[WebTask] Shutdown complete");
}

bool isWebTaskRunning() {
    return webTask != nullptr && webTask->isRunning();
}

bool queueWebRequest(WebRequestType type, 
                     WebPageType pageType,
                     const String& parameter,
                     WebServer* server) {
    if (webTask == nullptr) {
        Serial.println("[WebTask] ERROR: WebTask not initialized");
        return false;
    }
    
    WebTaskRequest request;
    request.type = type;
    request.pageType = pageType;
    request.parameter = parameter;
    request.server = server;
    request.requestId = millis();  // Simple request ID
    request.timestamp = millis();
    request.completed = false;
    
    return webTask->queueRequest(request);
}

bool queuePageRender(WebPageType pageType, WebServer* server, const String& parameter) {
    return queueWebRequest(WebRequestType::PAGE_RENDER, pageType, parameter, server);
}

bool queueScanTrigger(WebServer* server) {
    return queueWebRequest(WebRequestType::SCAN_TRIGGER, WebPageType::ROOT, "", server);
}

bool queueStatusUpdate(WebServer* server) {
    return queueWebRequest(WebRequestType::STATUS_UPDATE, WebPageType::STATUS, "", server);
}

WebTaskState getWebTaskState() {
    if (webTask == nullptr) {
        return WebTaskState::UNINITIALIZED;
    }
    return webTask->getState();
}

uint32_t getWebTaskQueuedRequests() {
    if (webTask == nullptr) {
        return 0;
    }
    return webTask->getQueuedRequestCount();
}

#endif // USE_WEBSERVER
#endif // USE_RTOS
