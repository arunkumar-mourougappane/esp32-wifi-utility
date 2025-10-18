#include "wifi_task.h"

#ifdef USE_RTOS

#include "mutex_manager.h"
#include "rtos_manager.h"
#include <WiFi.h>

// ==========================================
// GLOBAL INSTANCE
// ==========================================
WiFiTask* wifiTask = nullptr;

// ==========================================
// WIFI TASK IMPLEMENTATION
// ==========================================

WiFiTask::WiFiTask()
    : TaskBase("WiFiTask", 8192, TaskPriority::PRIORITY_HIGH, 0),  // Pin to Core 0 (WiFi Core)
      currentState(WiFiState::UNINITIALIZED),
      previousState(WiFiState::UNINITIALIZED),
      operationStartTime(0),
      currentOperationId(0),
      cachedNetworkCount(0),
      lastScanTime(0),
      scanInProgress(false),
      asyncScanId(-1) {
    
    // Create operation queue
    operationQueue = xQueueCreate(WIFI_OPERATION_QUEUE_LENGTH, sizeof(WiFiOperationRequest));
    if (operationQueue == nullptr) {
        Serial.println("✗ Failed to create WiFi operation queue");
    }
}

WiFiTask::~WiFiTask() {
    if (operationQueue != nullptr) {
        vQueueDelete(operationQueue);
        operationQueue = nullptr;
    }
}

void WiFiTask::setup() {
    MutexLock lock(serialMutex, "WiFiTask::setup");
    
    Serial.println("[RTOS] WiFi Task initialized on Core 0 (WiFi Core)");
    
    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    
    transitionState(WiFiState::IDLE);
    
    // Send initialization event
    WiFiEvent event;
    event.type = WiFiEvent::SCAN_STARTED;  // Using as generic init event
    event.timestamp = millis();
    sendWiFiEvent(event);
}

void WiFiTask::loop() {
    // Process operation queue
    processOperationQueue();
    
    // Process async scan if in progress
    if (scanInProgress) {
        processAsyncScan();
    }
    
    // Process connection state
    if (currentState == WiFiState::CONNECTING) {
        processConnectionState();
    }
    
    // Check for operation timeout
    if (currentState != WiFiState::IDLE && currentState != WiFiState::CONNECTED) {
        if (checkOperationTimeout()) {
            MutexLock lock(serialMutex, "WiFiTask::loop timeout");
            Serial.println("✗ WiFi operation timeout!");
            
            // Send error event
            WiFiEvent event;
            event.type = WiFiEvent::DISCONNECTED;  // Using as generic error event
            event.timestamp = millis();
            sendWiFiEvent(event);
            
            // Return to idle
            transitionState(WiFiState::IDLE);
        }
    }
    
    // Small delay to prevent tight loop
    vTaskDelay(pdMS_TO_TICKS(50));
}

void WiFiTask::cleanup() {
    MutexLock lock(serialMutex, "WiFiTask::cleanup");
    Serial.println("\nWiFi Task shutting down...");
    
    // Disconnect if connected
    if (WiFi.isConnected()) {
        WiFi.disconnect(true);
    }
}

void WiFiTask::processOperationQueue() {
    WiFiOperationRequest request;
    
    // Non-blocking check for operations (don't wait)
    if (xQueueReceive(operationQueue, &request, 0) == pdTRUE) {
        currentOperationId = request.requestId;
        operationStartTime = millis();
        
        // Process based on operation type
        switch (request.type) {
            case WiFiOperationType::SCAN:
                handleScanOperation(request);
                break;
                
            case WiFiOperationType::CONNECT:
                handleConnectOperation(request);
                break;
                
            case WiFiOperationType::DISCONNECT:
                handleDisconnectOperation(request);
                break;
                
            case WiFiOperationType::START_AP:
                handleStartAPOperation(request);
                break;
                
            case WiFiOperationType::STOP_AP:
                handleStopAPOperation(request);
                break;
                
            case WiFiOperationType::RECONNECT:
                // Reconnect using last credentials
                if (lastSSID.length() > 0) {
                    WiFiOperationRequest connectRequest = request;
                    connectRequest.type = WiFiOperationType::CONNECT;
                    connectRequest.ssid = lastSSID;
                    connectRequest.password = lastPassword;
                    handleConnectOperation(connectRequest);
                }
                break;
        }
    }
}

void WiFiTask::processAsyncScan() {
    if (asyncScanId < 0) {
        return;
    }
    
    // Check if scan is complete
    int16_t scanResult = WiFi.scanComplete();
    
    if (scanResult == WIFI_SCAN_RUNNING) {
        // Still scanning, check timeout
        return;
    }
    
    if (scanResult >= 0) {
        // Scan complete - cache results
        cacheScanResults();
        
        // Send SCAN_COMPLETE event
        WiFiEvent event;
        event.type = WiFiEvent::SCAN_COMPLETE;
        event.data.scanResult.networkCount = cachedNetworkCount;
        event.data.scanResult.scanDuration = millis() - operationStartTime;
        event.timestamp = millis();
        sendWiFiEvent(event);
        
        {
            MutexLock lock(serialMutex, "WiFiTask::processAsyncScan");
            Serial.printf("✓ Scan complete: %d networks found in %lu ms\n",
                         cachedNetworkCount, event.data.scanResult.scanDuration);
        }
        
        // Clean up
        WiFi.scanDelete();
        scanInProgress = false;
        asyncScanId = -1;
        transitionState(WiFiState::IDLE);
        
    } else if (scanResult == WIFI_SCAN_FAILED) {
        MutexLock lock(serialMutex, "WiFiTask::processAsyncScan fail");
        Serial.println("✗ WiFi scan failed!");
        
        scanInProgress = false;
        asyncScanId = -1;
        transitionState(WiFiState::IDLE);
    }
}

void WiFiTask::processConnectionState() {
    if (WiFi.status() == WL_CONNECTED) {
        // Successfully connected
        IPAddress ip = WiFi.localIP();
        
        // Send CONNECTED event
        WiFiEvent event;
        event.type = WiFiEvent::CONNECTED;
        event.timestamp = millis();
        sendWiFiEvent(event);
        
        // Send IP_ASSIGNED event
        WiFiEvent ipEvent;
        ipEvent.type = WiFiEvent::IP_ASSIGNED;
        ipEvent.data.ipInfo.ip[0] = ip[0];
        ipEvent.data.ipInfo.ip[1] = ip[1];
        ipEvent.data.ipInfo.ip[2] = ip[2];
        ipEvent.data.ipInfo.ip[3] = ip[3];
        ipEvent.timestamp = millis();
        sendWiFiEvent(ipEvent);
        
        {
            MutexLock lock(serialMutex, "WiFiTask::processConnectionState");
            Serial.printf("✓ Connected to %s, IP: %s\n", 
                         lastSSID.c_str(), ip.toString().c_str());
        }
        
        transitionState(WiFiState::CONNECTED);
        
    } else if (WiFi.status() == WL_CONNECT_FAILED || 
               WiFi.status() == WL_NO_SSID_AVAIL) {
        // Connection failed
        WiFiEvent event;
        event.type = WiFiEvent::DISCONNECTED;
        event.timestamp = millis();
        sendWiFiEvent(event);
        
        {
            MutexLock lock(serialMutex, "WiFiTask::processConnectionState fail");
            Serial.printf("✗ Failed to connect to %s\n", lastSSID.c_str());
        }
        
        transitionState(WiFiState::IDLE);
    }
}

void WiFiTask::handleScanOperation(const WiFiOperationRequest& request) {
    if (scanInProgress) {
        MutexLock lock(serialMutex, "WiFiTask::handleScanOperation busy");
        Serial.println("✗ Scan already in progress");
        return;
    }
    
    transitionState(WiFiState::SCANNING);
    
    // Send SCAN_STARTED event
    WiFiEvent event;
    event.type = WiFiEvent::SCAN_STARTED;
    event.timestamp = millis();
    sendWiFiEvent(event);
    
    {
        MutexLock lock(serialMutex, "WiFiTask::handleScanOperation");
        Serial.println("⌛ Starting async WiFi scan...");
    }
    
    // Start async scan
    asyncScanId = WiFi.scanNetworks(true);  // true = async mode
    scanInProgress = true;
    lastScanTime = millis();
}

void WiFiTask::handleConnectOperation(const WiFiOperationRequest& request) {
    if (currentState == WiFiState::CONNECTING || currentState == WiFiState::CONNECTED) {
        MutexLock lock(serialMutex, "WiFiTask::handleConnectOperation busy");
        Serial.println("✗ Already connected or connecting");
        return;
    }
    
    transitionState(WiFiState::CONNECTING);
    
    // Save credentials
    lastSSID = request.ssid;
    lastPassword = request.password;
    
    // Send CONNECT_STARTED event
    WiFiEvent event;
    event.type = WiFiEvent::CONNECT_STARTED;
    event.timestamp = millis();
    sendWiFiEvent(event);
    
    {
        MutexLock lock(serialMutex, "WiFiTask::handleConnectOperation");
        Serial.printf("⌛ Connecting to %s...\n", request.ssid.c_str());
    }
    
    // Start connection attempt
    WiFi.begin(request.ssid.c_str(), request.password.c_str());
}

void WiFiTask::handleDisconnectOperation(const WiFiOperationRequest& request) {
    if (!WiFi.isConnected()) {
        MutexLock lock(serialMutex, "WiFiTask::handleDisconnectOperation");
        Serial.println("ℹ Not connected to any network");
        transitionState(WiFiState::IDLE);
        return;
    }
    
    transitionState(WiFiState::DISCONNECTING);
    
    {
        MutexLock lock(serialMutex, "WiFiTask::handleDisconnectOperation");
        Serial.println("⌛ Disconnecting from network...");
    }
    
    // Disconnect
    WiFi.disconnect(true);
    
    // Send DISCONNECTED event
    WiFiEvent event;
    event.type = WiFiEvent::DISCONNECTED;
    event.timestamp = millis();
    sendWiFiEvent(event);
    
    {
        MutexLock lock(serialMutex, "WiFiTask::handleDisconnectOperation done");
        Serial.println("✓ Disconnected from network");
    }
    
    transitionState(WiFiState::IDLE);
}

void WiFiTask::handleStartAPOperation(const WiFiOperationRequest& request) {
    {
        MutexLock lock(serialMutex, "WiFiTask::handleStartAPOperation");
        Serial.printf("⌛ Starting AP: %s...\n", request.ssid.c_str());
    }
    
    // Switch to AP mode
    WiFi.mode(WIFI_AP);
    
    bool success = WiFi.softAP(request.ssid.c_str(), request.password.c_str());
    
    if (success) {
        // Send AP_STARTED event
        WiFiEvent event;
        event.type = WiFiEvent::AP_STARTED;
        event.timestamp = millis();
        sendWiFiEvent(event);
        
        IPAddress ip = WiFi.softAPIP();
        {
            MutexLock lock(serialMutex, "WiFiTask::handleStartAPOperation success");
            Serial.printf("✓ AP started: %s, IP: %s\n", 
                         request.ssid.c_str(), ip.toString().c_str());
        }
        
        transitionState(WiFiState::AP_MODE);
    } else {
        MutexLock lock(serialMutex, "WiFiTask::handleStartAPOperation fail");
        Serial.println("✗ Failed to start AP");
        transitionState(WiFiState::ERROR);
    }
}

void WiFiTask::handleStopAPOperation(const WiFiOperationRequest& request) {
    {
        MutexLock lock(serialMutex, "WiFiTask::handleStopAPOperation");
        Serial.println("⌛ Stopping AP...");
    }
    
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    
    // Send AP_STOPPED event
    WiFiEvent event;
    event.type = WiFiEvent::AP_STOPPED;
    event.timestamp = millis();
    sendWiFiEvent(event);
    
    {
        MutexLock lock(serialMutex, "WiFiTask::handleStopAPOperation done");
        Serial.println("✓ AP stopped");
    }
    
    transitionState(WiFiState::IDLE);
}

void WiFiTask::cacheScanResults() {
    MutexLock lock(scanResultsMutex, "WiFiTask::cacheScanResults");
    
    int16_t count = WiFi.scanComplete();
    if (count < 0) {
        cachedNetworkCount = 0;
        return;
    }
    
    cachedNetworkCount = min((int16_t)MAX_CACHED_NETWORKS, count);
    
    for (uint16_t i = 0; i < cachedNetworkCount; i++) {
        cachedResults[i].ssid = WiFi.SSID(i);
        memcpy(cachedResults[i].bssid, WiFi.BSSID(i), 6);
        cachedResults[i].rssi = WiFi.RSSI(i);
        cachedResults[i].channel = WiFi.channel(i);
        cachedResults[i].encryptionType = WiFi.encryptionType(i);
        cachedResults[i].hidden = (WiFi.SSID(i).length() == 0);
    }
    
    lastScanTime = millis();
}

void WiFiTask::sendWiFiEvent(const WiFiEvent& event) {
    // Send to WiFi event queue (if it exists)
    if (wifiEventQueue != nullptr) {
        if (xQueueSend(wifiEventQueue, &event, pdMS_TO_TICKS(100)) != pdTRUE) {
            MutexLock lock(serialMutex, "WiFiTask::sendWiFiEvent");
            Serial.println("⚠ WiFi event queue full!");
        }
    }
}

void WiFiTask::transitionState(WiFiState newState) {
    if (currentState != newState) {
        previousState = currentState;
        currentState = newState;
        
        // Log state transition in debug mode
        #ifdef DEBUG_WIFI_STATE
        MutexLock lock(serialMutex, "WiFiTask::transitionState");
        Serial.printf("[WiFi State] %d -> %d\n", (int)previousState, (int)currentState);
        #endif
    }
}

bool WiFiTask::checkOperationTimeout() {
    if (operationStartTime == 0) {
        return false;
    }
    
    uint32_t elapsed = millis() - operationStartTime;
    uint32_t timeout = getOperationTimeout();
    
    return (elapsed > timeout);
}

uint32_t WiFiTask::getOperationTimeout() const {
    switch (currentState) {
        case WiFiState::SCANNING:
            return WIFI_SCAN_TIMEOUT_MS;
        case WiFiState::CONNECTING:
            return WIFI_CONNECT_TIMEOUT_MS;
        case WiFiState::DISCONNECTING:
            return WIFI_DISCONNECT_TIMEOUT_MS;
        default:
            return 30000;  // Default 30 seconds
    }
}

bool WiFiTask::getCachedNetwork(uint16_t index, WiFiScanResult& result) {
    MutexLock lock(scanResultsMutex, "WiFiTask::getCachedNetwork");
    
    if (index >= cachedNetworkCount) {
        return false;
    }
    
    result = cachedResults[index];
    return true;
}

bool WiFiTask::isConnected() const {
    return (currentState == WiFiState::CONNECTED && WiFi.isConnected());
}

bool WiFiTask::queueOperation(const WiFiOperationRequest& request) {
    if (operationQueue == nullptr) {
        return false;
    }
    
    return (xQueueSend(operationQueue, &request, pdMS_TO_TICKS(100)) == pdTRUE);
}

// ==========================================
// PUBLIC API IMPLEMENTATION
// ==========================================

bool initializeWiFiTask() {
    if (wifiTask != nullptr) {
        return true;  // Already initialized
    }
    
    wifiTask = new WiFiTask();
    if (wifiTask == nullptr) {
        Serial.println("✗ Failed to allocate WiFiTask");
        return false;
    }
    
    if (!wifiTask->start()) {
        Serial.println("✗ Failed to start WiFiTask");
        delete wifiTask;
        wifiTask = nullptr;
        return false;
    }
    
    return true;
}

void shutdownWiFiTask() {
    if (wifiTask != nullptr) {
        wifiTask->stop(5000);
        delete wifiTask;
        wifiTask = nullptr;
    }
}

bool isWiFiTaskRunning() {
    if (wifiTask == nullptr) {
        return false;
    }
    
    TaskStatistics stats = wifiTask->getStatistics();
    return (stats.state == TaskState::RUNNING);
}

bool queueScanRequest(bool async) {
    if (wifiTask == nullptr) {
        return false;
    }
    
    WiFiOperationRequest request;
    request.type = WiFiOperationType::SCAN;
    request.requestId = millis();
    request.timestamp = millis();
    request.async = async;
    
    return wifiTask->queueOperation(request);
}

bool queueConnectRequest(const String& ssid, const String& password, bool async) {
    if (wifiTask == nullptr) {
        return false;
    }
    
    WiFiOperationRequest request;
    request.type = WiFiOperationType::CONNECT;
    request.ssid = ssid;
    request.password = password;
    request.requestId = millis();
    request.timestamp = millis();
    request.async = async;
    
    return wifiTask->queueOperation(request);
}

bool queueDisconnectRequest(bool async) {
    if (wifiTask == nullptr) {
        return false;
    }
    
    WiFiOperationRequest request;
    request.type = WiFiOperationType::DISCONNECT;
    request.requestId = millis();
    request.timestamp = millis();
    request.async = async;
    
    return wifiTask->queueOperation(request);
}

bool queueStartAPRequest(const String& ssid, const String& password, bool async) {
    if (wifiTask == nullptr) {
        return false;
    }
    
    WiFiOperationRequest request;
    request.type = WiFiOperationType::START_AP;
    request.ssid = ssid;
    request.password = password;
    request.requestId = millis();
    request.timestamp = millis();
    request.async = async;
    
    return wifiTask->queueOperation(request);
}

bool queueStopAPRequest(bool async) {
    if (wifiTask == nullptr) {
        return false;
    }
    
    WiFiOperationRequest request;
    request.type = WiFiOperationType::STOP_AP;
    request.requestId = millis();
    request.timestamp = millis();
    request.async = async;
    
    return wifiTask->queueOperation(request);
}

WiFiState getWiFiTaskState() {
    if (wifiTask == nullptr) {
        return WiFiState::UNINITIALIZED;
    }
    
    return wifiTask->getState();
}

uint16_t getWiFiCachedNetworkCount() {
    if (wifiTask == nullptr) {
        return 0;
    }
    
    return wifiTask->getCachedNetworkCount();
}

bool getWiFiCachedNetwork(uint16_t index, WiFiScanResult& result) {
    if (wifiTask == nullptr) {
        return false;
    }
    
    return wifiTask->getCachedNetwork(index, result);
}

bool isWiFiTaskConnected() {
    if (wifiTask == nullptr) {
        return false;
    }
    
    return wifiTask->isConnected();
}

#endif // USE_RTOS
