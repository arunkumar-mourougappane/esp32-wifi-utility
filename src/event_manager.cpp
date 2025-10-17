#include "event_manager.h"

#ifdef USE_RTOS

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

// ==========================================
// EVENT GROUP HANDLE
// ==========================================

EventGroupHandle_t systemEventGroup = NULL;

// ==========================================
// CORE FUNCTIONS
// ==========================================

bool initializeEventManager() {
    Serial.println("[Event] Creating system event group...");
    
    systemEventGroup = xEventGroupCreate();
    if (systemEventGroup == NULL) {
        Serial.println("[Event] ERROR: Failed to create system event group");
        return false;
    }
    
    // Clear all bits initially
    xEventGroupClearBits(systemEventGroup, ALL_EVENT_BITS);
    
    Serial.println("[Event] System event group created successfully");
    return true;
}

bool shutdownEventManager() {
    Serial.println("[Event] Deleting system event group...");
    
    if (systemEventGroup != NULL) {
        vEventGroupDelete(systemEventGroup);
        systemEventGroup = NULL;
    }
    
    Serial.println("[Event] System event group deleted");
    return true;
}

bool isEventManagerInitialized() {
    return systemEventGroup != NULL;
}

// ==========================================
// EVENT BIT OPERATIONS
// ==========================================

EventBits_t setEventBits(EventBits_t bits) {
    if (systemEventGroup == NULL) return 0;
    return xEventGroupSetBits(systemEventGroup, bits);
}

EventBits_t clearEventBits(EventBits_t bits) {
    if (systemEventGroup == NULL) return 0;
    return xEventGroupClearBits(systemEventGroup, bits);
}

EventBits_t getEventBits() {
    if (systemEventGroup == NULL) return 0;
    return xEventGroupGetBits(systemEventGroup);
}

bool isEventBitSet(EventBits_t bits) {
    if (systemEventGroup == NULL) return false;
    EventBits_t currentBits = xEventGroupGetBits(systemEventGroup);
    return (currentBits & bits) == bits;
}

bool isAnyEventBitSet(EventBits_t bits) {
    if (systemEventGroup == NULL) return false;
    EventBits_t currentBits = xEventGroupGetBits(systemEventGroup);
    return (currentBits & bits) != 0;
}

// ==========================================
// EVENT WAITING FUNCTIONS
// ==========================================

EventBits_t waitForEventBits(EventBits_t bits, bool clearOnExit, uint32_t timeoutMs) {
    if (systemEventGroup == NULL) return 0;
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    
    // Wait for all bits
    return xEventGroupWaitBits(
        systemEventGroup,
        bits,
        clearOnExit ? pdTRUE : pdFALSE,  // Clear on exit
        pdTRUE,                           // Wait for all bits
        timeout
    );
}

EventBits_t waitForAnyEventBit(EventBits_t bits, bool clearOnExit, uint32_t timeoutMs) {
    if (systemEventGroup == NULL) return 0;
    
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    
    // Wait for any bit
    return xEventGroupWaitBits(
        systemEventGroup,
        bits,
        clearOnExit ? pdTRUE : pdFALSE,  // Clear on exit
        pdFALSE,                          // Wait for any bit
        timeout
    );
}

// ==========================================
// WIFI EVENT FUNCTIONS
// ==========================================

void signalWiFiConnected() {
    setEventBits(WIFI_CONNECTED_BIT);
    Serial.println("[Event] WiFi connected event signaled");
}

void signalWiFiDisconnected() {
    clearEventBits(WIFI_CONNECTED_BIT);
    Serial.println("[Event] WiFi disconnected event signaled");
}

bool isWiFiConnectedEvent() {
    return isEventBitSet(WIFI_CONNECTED_BIT);
}

bool waitForWiFiConnection(uint32_t timeoutMs) {
    EventBits_t bits = waitForEventBits(WIFI_CONNECTED_BIT, false, timeoutMs);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

void signalWiFiScanStarted() {
    setEventBits(WIFI_SCANNING_BIT);
    Serial.println("[Event] WiFi scan started event signaled");
}

void signalWiFiScanCompleted() {
    clearEventBits(WIFI_SCANNING_BIT);
    Serial.println("[Event] WiFi scan completed event signaled");
}

bool isWiFiScanningEvent() {
    return isEventBitSet(WIFI_SCANNING_BIT);
}

bool waitForWiFiScanComplete(uint32_t timeoutMs) {
    // Wait for scanning bit to be cleared (scan complete)
    unsigned long startMs = millis();
    while (isWiFiScanningEvent() && (millis() - startMs < timeoutMs)) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return !isWiFiScanningEvent();
}

// ==========================================
// WEB SERVER EVENT FUNCTIONS
// ==========================================

void signalWebServerStarted() {
    setEventBits(WEB_SERVER_RUNNING_BIT);
    Serial.println("[Event] Web server started event signaled");
}

void signalWebServerStopped() {
    clearEventBits(WEB_SERVER_RUNNING_BIT);
    Serial.println("[Event] Web server stopped event signaled");
}

bool isWebServerRunningEvent() {
    return isEventBitSet(WEB_SERVER_RUNNING_BIT);
}

// ==========================================
// ANALYSIS EVENT FUNCTIONS
// ==========================================

void signalAnalysisStarted() {
    setEventBits(ANALYSIS_RUNNING_BIT);
    Serial.println("[Event] Analysis started event signaled");
}

void signalAnalysisCompleted() {
    clearEventBits(ANALYSIS_RUNNING_BIT);
    Serial.println("[Event] Analysis completed event signaled");
}

bool isAnalysisRunningEvent() {
    return isEventBitSet(ANALYSIS_RUNNING_BIT);
}

bool waitForAnalysisComplete(uint32_t timeoutMs) {
    // Wait for analysis bit to be cleared (analysis complete)
    unsigned long startMs = millis();
    while (isAnalysisRunningEvent() && (millis() - startMs < timeoutMs)) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return !isAnalysisRunningEvent();
}

// ==========================================
// AP MODE EVENT FUNCTIONS
// ==========================================

void signalAPModeStarted() {
    setEventBits(AP_MODE_ACTIVE_BIT);
    Serial.println("[Event] AP mode started event signaled");
}

void signalAPModeStopped() {
    clearEventBits(AP_MODE_ACTIVE_BIT);
    Serial.println("[Event] AP mode stopped event signaled");
}

bool isAPModeActiveEvent() {
    return isEventBitSet(AP_MODE_ACTIVE_BIT);
}

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

void printEventBits() {
    if (systemEventGroup == NULL) {
        Serial.println("[Event] Event group not initialized");
        return;
    }
    
    EventBits_t bits = xEventGroupGetBits(systemEventGroup);
    
    Serial.println("\n=== System Event Bits ===");
    Serial.printf("Raw Value: 0x%02X\n\n", bits);
    
    Serial.printf("WiFi Connected:      %s\n", 
                 (bits & WIFI_CONNECTED_BIT) ? "SET" : "CLEAR");
    Serial.printf("WiFi Scanning:       %s\n", 
                 (bits & WIFI_SCANNING_BIT) ? "SET" : "CLEAR");
    Serial.printf("Web Server Running:  %s\n", 
                 (bits & WEB_SERVER_RUNNING_BIT) ? "SET" : "CLEAR");
    Serial.printf("Analysis Running:    %s\n", 
                 (bits & ANALYSIS_RUNNING_BIT) ? "SET" : "CLEAR");
    Serial.printf("AP Mode Active:      %s\n", 
                 (bits & AP_MODE_ACTIVE_BIT) ? "SET" : "CLEAR");
    
    Serial.println("========================\n");
}

const char* getEventBitName(EventBits_t bit) {
    switch (bit) {
        case WIFI_CONNECTED_BIT:
            return "WiFi Connected";
        case WIFI_SCANNING_BIT:
            return "WiFi Scanning";
        case WEB_SERVER_RUNNING_BIT:
            return "Web Server Running";
        case ANALYSIS_RUNNING_BIT:
            return "Analysis Running";
        case AP_MODE_ACTIVE_BIT:
            return "AP Mode Active";
        default:
            return "Unknown";
    }
}

void clearAllEventBits() {
    if (systemEventGroup != NULL) {
        xEventGroupClearBits(systemEventGroup, ALL_EVENT_BITS);
        Serial.println("[Event] All event bits cleared");
    }
}

#endif // USE_RTOS
