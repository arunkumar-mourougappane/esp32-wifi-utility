#include "queue_manager.h"

#ifdef USE_RTOS

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// ==========================================
// QUEUE HANDLES
// ==========================================

QueueHandle_t commandQueue = NULL;
QueueHandle_t wifiEventQueue = NULL;
QueueHandle_t analysisResultQueue = NULL;
QueueHandle_t webRequestQueue = NULL;
QueueHandle_t statusQueue = NULL;

// ==========================================
// QUEUE STATISTICS
// ==========================================

static QueueStatistics commandQueueStats = {0};
static QueueStatistics wifiEventQueueStats = {0};
static QueueStatistics analysisResultQueueStats = {0};
static QueueStatistics webRequestQueueStats = {0};
static QueueStatistics statusQueueStats = {0};

// ==========================================
// PRIVATE FUNCTIONS
// ==========================================

static void updateQueueStats(QueueStatistics& stats, QueueHandle_t queue) {
    if (queue != NULL) {
        stats.currentItems = uxQueueMessagesWaiting(queue);
        if (stats.currentItems > stats.peakItems) {
            stats.peakItems = stats.currentItems;
        }
    }
}

// ==========================================
// CORE FUNCTIONS
// ==========================================

bool initializeQueueManager() {
    Serial.println("[Queue] Creating queues...");
    
    // Create command queue
    commandQueue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(CommandRequest));
    if (commandQueue == NULL) {
        Serial.println("[Queue] ERROR: Failed to create command queue");
        return false;
    }
    Serial.printf("[Queue] Command queue created (length: %d)\n", COMMAND_QUEUE_LENGTH);
    
    // Create WiFi event queue
    wifiEventQueue = xQueueCreate(WIFI_EVENT_QUEUE_LENGTH, sizeof(WiFiEvent));
    if (wifiEventQueue == NULL) {
        Serial.println("[Queue] ERROR: Failed to create WiFi event queue");
        return false;
    }
    Serial.printf("[Queue] WiFi event queue created (length: %d)\n", WIFI_EVENT_QUEUE_LENGTH);
    
    // Create analysis result queue
    analysisResultQueue = xQueueCreate(ANALYSIS_RESULT_QUEUE_LENGTH, sizeof(AnalysisResult));
    if (analysisResultQueue == NULL) {
        Serial.println("[Queue] ERROR: Failed to create analysis result queue");
        return false;
    }
    Serial.printf("[Queue] Analysis result queue created (length: %d)\n", ANALYSIS_RESULT_QUEUE_LENGTH);
    
    // Create web request queue
    webRequestQueue = xQueueCreate(WEB_REQUEST_QUEUE_LENGTH, sizeof(WebRequest));
    if (webRequestQueue == NULL) {
        Serial.println("[Queue] ERROR: Failed to create web request queue");
        return false;
    }
    Serial.printf("[Queue] Web request queue created (length: %d)\n", WEB_REQUEST_QUEUE_LENGTH);
    
    // Create status queue
    statusQueue = xQueueCreate(STATUS_QUEUE_LENGTH, sizeof(StatusUpdate));
    if (statusQueue == NULL) {
        Serial.println("[Queue] ERROR: Failed to create status queue");
        return false;
    }
    Serial.printf("[Queue] Status queue created (length: %d)\n", STATUS_QUEUE_LENGTH);
    
    // Reset statistics
    resetQueueStatistics();
    
    Serial.println("[Queue] All queues created successfully");
    return true;
}

bool shutdownQueueManager() {
    Serial.println("[Queue] Deleting queues...");
    
    if (statusQueue != NULL) {
        vQueueDelete(statusQueue);
        statusQueue = NULL;
    }
    
    if (webRequestQueue != NULL) {
        vQueueDelete(webRequestQueue);
        webRequestQueue = NULL;
    }
    
    if (analysisResultQueue != NULL) {
        vQueueDelete(analysisResultQueue);
        analysisResultQueue = NULL;
    }
    
    if (wifiEventQueue != NULL) {
        vQueueDelete(wifiEventQueue);
        wifiEventQueue = NULL;
    }
    
    if (commandQueue != NULL) {
        vQueueDelete(commandQueue);
        commandQueue = NULL;
    }
    
    Serial.println("[Queue] All queues deleted");
    return true;
}

uint32_t getQueueCount() {
    uint32_t count = 0;
    if (commandQueue != NULL) count++;
    if (wifiEventQueue != NULL) count++;
    if (analysisResultQueue != NULL) count++;
    if (webRequestQueue != NULL) count++;
    if (statusQueue != NULL) count++;
    return count;
}

bool checkQueueHealth() {
    // Check if any queue is full (potential overflow)
    bool healthy = true;
    
    if (isQueueFull(commandQueue)) {
        Serial.println("[Queue] WARNING: Command queue is full!");
        healthy = false;
    }
    
    if (isQueueFull(wifiEventQueue)) {
        Serial.println("[Queue] WARNING: WiFi event queue is full!");
        healthy = false;
    }
    
    if (isQueueFull(analysisResultQueue)) {
        Serial.println("[Queue] WARNING: Analysis result queue is full!");
        healthy = false;
    }
    
    if (isQueueFull(webRequestQueue)) {
        Serial.println("[Queue] WARNING: Web request queue is full!");
        healthy = false;
    }
    
    if (isQueueFull(statusQueue)) {
        Serial.println("[Queue] WARNING: Status queue is full!");
        healthy = false;
    }
    
    return healthy;
}

// ==========================================
// COMMAND QUEUE FUNCTIONS
// ==========================================

bool sendCommand(const CommandRequest& command, uint32_t timeoutMs) {
    if (commandQueue == NULL) {
        commandQueueStats.sendFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueSend(commandQueue, &command, timeout) == pdTRUE) {
        commandQueueStats.itemsSent++;
        updateQueueStats(commandQueueStats, commandQueue);
        return true;
    }
    
    commandQueueStats.sendFailures++;
    commandQueueStats.overflows++;
    return false;
}

bool receiveCommand(CommandRequest& command, uint32_t timeoutMs) {
    if (commandQueue == NULL) {
        commandQueueStats.receiveFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueReceive(commandQueue, &command, timeout) == pdTRUE) {
        commandQueueStats.itemsReceived++;
        updateQueueStats(commandQueueStats, commandQueue);
        return true;
    }
    
    commandQueueStats.receiveFailures++;
    return false;
}

uint32_t getPendingCommandCount() {
    if (commandQueue == NULL) return 0;
    return uxQueueMessagesWaiting(commandQueue);
}

QueueStatistics getCommandQueueStatistics() {
    updateQueueStats(commandQueueStats, commandQueue);
    return commandQueueStats;
}

// ==========================================
// WIFI EVENT QUEUE FUNCTIONS
// ==========================================

bool sendWiFiEvent(const WiFiEvent& event, uint32_t timeoutMs) {
    if (wifiEventQueue == NULL) {
        wifiEventQueueStats.sendFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueSend(wifiEventQueue, &event, timeout) == pdTRUE) {
        wifiEventQueueStats.itemsSent++;
        updateQueueStats(wifiEventQueueStats, wifiEventQueue);
        return true;
    }
    
    wifiEventQueueStats.sendFailures++;
    wifiEventQueueStats.overflows++;
    return false;
}

bool receiveWiFiEvent(WiFiEvent& event, uint32_t timeoutMs) {
    if (wifiEventQueue == NULL) {
        wifiEventQueueStats.receiveFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueReceive(wifiEventQueue, &event, timeout) == pdTRUE) {
        wifiEventQueueStats.itemsReceived++;
        updateQueueStats(wifiEventQueueStats, wifiEventQueue);
        return true;
    }
    
    wifiEventQueueStats.receiveFailures++;
    return false;
}

uint32_t getPendingWiFiEventCount() {
    if (wifiEventQueue == NULL) return 0;
    return uxQueueMessagesWaiting(wifiEventQueue);
}

QueueStatistics getWiFiEventQueueStatistics() {
    updateQueueStats(wifiEventQueueStats, wifiEventQueue);
    return wifiEventQueueStats;
}

// ==========================================
// ANALYSIS RESULT QUEUE FUNCTIONS
// ==========================================

bool sendAnalysisResult(const AnalysisResult& result, uint32_t timeoutMs) {
    if (analysisResultQueue == NULL) {
        analysisResultQueueStats.sendFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueSend(analysisResultQueue, &result, timeout) == pdTRUE) {
        analysisResultQueueStats.itemsSent++;
        updateQueueStats(analysisResultQueueStats, analysisResultQueue);
        return true;
    }
    
    analysisResultQueueStats.sendFailures++;
    analysisResultQueueStats.overflows++;
    return false;
}

bool receiveAnalysisResult(AnalysisResult& result, uint32_t timeoutMs) {
    if (analysisResultQueue == NULL) {
        analysisResultQueueStats.receiveFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueReceive(analysisResultQueue, &result, timeout) == pdTRUE) {
        analysisResultQueueStats.itemsReceived++;
        updateQueueStats(analysisResultQueueStats, analysisResultQueue);
        return true;
    }
    
    analysisResultQueueStats.receiveFailures++;
    return false;
}

uint32_t getPendingAnalysisResultCount() {
    if (analysisResultQueue == NULL) return 0;
    return uxQueueMessagesWaiting(analysisResultQueue);
}

QueueStatistics getAnalysisResultQueueStatistics() {
    updateQueueStats(analysisResultQueueStats, analysisResultQueue);
    return analysisResultQueueStats;
}

// ==========================================
// WEB REQUEST QUEUE FUNCTIONS
// ==========================================

bool sendWebRequest(const WebRequest& request, uint32_t timeoutMs) {
    if (webRequestQueue == NULL) {
        webRequestQueueStats.sendFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueSend(webRequestQueue, &request, timeout) == pdTRUE) {
        webRequestQueueStats.itemsSent++;
        updateQueueStats(webRequestQueueStats, webRequestQueue);
        return true;
    }
    
    webRequestQueueStats.sendFailures++;
    webRequestQueueStats.overflows++;
    return false;
}

bool receiveWebRequest(WebRequest& request, uint32_t timeoutMs) {
    if (webRequestQueue == NULL) {
        webRequestQueueStats.receiveFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueReceive(webRequestQueue, &request, timeout) == pdTRUE) {
        webRequestQueueStats.itemsReceived++;
        updateQueueStats(webRequestQueueStats, webRequestQueue);
        return true;
    }
    
    webRequestQueueStats.receiveFailures++;
    return false;
}

uint32_t getPendingWebRequestCount() {
    if (webRequestQueue == NULL) return 0;
    return uxQueueMessagesWaiting(webRequestQueue);
}

QueueStatistics getWebRequestQueueStatistics() {
    updateQueueStats(webRequestQueueStats, webRequestQueue);
    return webRequestQueueStats;
}

// ==========================================
// STATUS QUEUE FUNCTIONS
// ==========================================

bool sendStatusUpdate(const StatusUpdate& status, uint32_t timeoutMs) {
    if (statusQueue == NULL) {
        statusQueueStats.sendFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueSend(statusQueue, &status, timeout) == pdTRUE) {
        statusQueueStats.itemsSent++;
        updateQueueStats(statusQueueStats, statusQueue);
        return true;
    }
    
    statusQueueStats.sendFailures++;
    statusQueueStats.overflows++;
    return false;
}

bool receiveStatusUpdate(StatusUpdate& status, uint32_t timeoutMs) {
    if (statusQueue == NULL) {
        statusQueueStats.receiveFailures++;
        return false;
    }
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    if (xQueueReceive(statusQueue, &status, timeout) == pdTRUE) {
        statusQueueStats.itemsReceived++;
        updateQueueStats(statusQueueStats, statusQueue);
        return true;
    }
    
    statusQueueStats.receiveFailures++;
    return false;
}

uint32_t getPendingStatusCount() {
    if (statusQueue == NULL) return 0;
    return uxQueueMessagesWaiting(statusQueue);
}

QueueStatistics getStatusQueueStatistics() {
    updateQueueStats(statusQueueStats, statusQueue);
    return statusQueueStats;
}

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

void printQueueStatistics() {
    Serial.println("\n=== Queue Statistics ===");
    
    // Command Queue
    Serial.println("\n--- Command Queue ---");
    QueueStatistics stats = getCommandQueueStatistics();
    Serial.printf("Sent: %u | Received: %u | Pending: %u/%d\n",
                  stats.itemsSent, stats.itemsReceived, stats.currentItems, COMMAND_QUEUE_LENGTH);
    Serial.printf("Peak: %u | Send Failures: %u | Receive Failures: %u | Overflows: %u\n",
                  stats.peakItems, stats.sendFailures, stats.receiveFailures, stats.overflows);
    
    // WiFi Event Queue
    Serial.println("\n--- WiFi Event Queue ---");
    stats = getWiFiEventQueueStatistics();
    Serial.printf("Sent: %u | Received: %u | Pending: %u/%d\n",
                  stats.itemsSent, stats.itemsReceived, stats.currentItems, WIFI_EVENT_QUEUE_LENGTH);
    Serial.printf("Peak: %u | Send Failures: %u | Receive Failures: %u | Overflows: %u\n",
                  stats.peakItems, stats.sendFailures, stats.receiveFailures, stats.overflows);
    
    // Analysis Result Queue
    Serial.println("\n--- Analysis Result Queue ---");
    stats = getAnalysisResultQueueStatistics();
    Serial.printf("Sent: %u | Received: %u | Pending: %u/%d\n",
                  stats.itemsSent, stats.itemsReceived, stats.currentItems, ANALYSIS_RESULT_QUEUE_LENGTH);
    Serial.printf("Peak: %u | Send Failures: %u | Receive Failures: %u | Overflows: %u\n",
                  stats.peakItems, stats.sendFailures, stats.receiveFailures, stats.overflows);
    
    // Web Request Queue
    Serial.println("\n--- Web Request Queue ---");
    stats = getWebRequestQueueStatistics();
    Serial.printf("Sent: %u | Received: %u | Pending: %u/%d\n",
                  stats.itemsSent, stats.itemsReceived, stats.currentItems, WEB_REQUEST_QUEUE_LENGTH);
    Serial.printf("Peak: %u | Send Failures: %u | Receive Failures: %u | Overflows: %u\n",
                  stats.peakItems, stats.sendFailures, stats.receiveFailures, stats.overflows);
    
    // Status Queue
    Serial.println("\n--- Status Queue ---");
    stats = getStatusQueueStatistics();
    Serial.printf("Sent: %u | Received: %u | Pending: %u/%d\n",
                  stats.itemsSent, stats.itemsReceived, stats.currentItems, STATUS_QUEUE_LENGTH);
    Serial.printf("Peak: %u | Send Failures: %u | Receive Failures: %u | Overflows: %u\n",
                  stats.peakItems, stats.sendFailures, stats.receiveFailures, stats.overflows);
    
    Serial.println("======================\n");
}

void resetQueueStatistics() {
    commandQueueStats = {0};
    wifiEventQueueStats = {0};
    analysisResultQueueStats = {0};
    webRequestQueueStats = {0};
    statusQueueStats = {0};
}

void flushAllQueues() {
    Serial.println("[Queue] Flushing all queues...");
    
    if (commandQueue != NULL) {
        xQueueReset(commandQueue);
    }
    if (wifiEventQueue != NULL) {
        xQueueReset(wifiEventQueue);
    }
    if (analysisResultQueue != NULL) {
        xQueueReset(analysisResultQueue);
    }
    if (webRequestQueue != NULL) {
        xQueueReset(webRequestQueue);
    }
    if (statusQueue != NULL) {
        xQueueReset(statusQueue);
    }
    
    Serial.println("[Queue] All queues flushed");
}

bool isQueueFull(QueueHandle_t queueHandle) {
    if (queueHandle == NULL) return false;
    return uxQueueSpacesAvailable(queueHandle) == 0;
}

bool isQueueEmpty(QueueHandle_t queueHandle) {
    if (queueHandle == NULL) return true;
    return uxQueueMessagesWaiting(queueHandle) == 0;
}

#endif // USE_RTOS
