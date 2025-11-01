#include "wifi_task.h"
#include "wifi_manager.h"
#include "ap_config.h"
#include "station_config.h"

// ==========================================
// TASK VARIABLES
// ==========================================
static QueueHandle_t wifiCommandQueue = nullptr;
static TaskHandle_t wifiTaskHandle = nullptr;

// ==========================================
// WIFI COMMAND TASK
// ==========================================
static void wifiCommandTask(void* parameter) {
    WiFiCommand cmd;
    
    Serial.println("🎯 WiFi Command Task started on Core 1");
    
    while (true) {
        // Wait for commands (block indefinitely)
        if (xQueueReceive(wifiCommandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            Serial.printf("[WiFi Task] Processing command: %d\n", cmd.type);
            
            switch (cmd.type) {
                case WIFI_CMD_SWITCH_TO_AP:
                    {
                        Serial.println("[WiFi Task] Switching to Access Point mode");
                        
                        if (strlen(cmd.param1) > 0 && strlen(cmd.param2) > 0) {
                            // Use provided credentials
                            startAccessPoint(String(cmd.param1), String(cmd.param2));
                            Serial.printf("[WiFi Task] Started AP with SSID: %s\n", cmd.param1);
                        } else {
                            // Try to load saved config
                            APConfig config;
                            if (loadAPConfig(config)) {
                                startAccessPoint(String(config.ssid), String(config.password));
                                Serial.printf("[WiFi Task] Started AP with saved config: %s\n", config.ssid);
                            } else {
                                // Use defaults
                                startAccessPoint();
                                Serial.println("[WiFi Task] Started AP with default config");
                            }
                        }
                    }
                    break;
                    
                case WIFI_CMD_SWITCH_TO_STATION:
                    {
                        Serial.println("[WiFi Task] Switching to Station mode");
                        
                        // First set to Station mode
                        startStationMode();
                        
                        if (strlen(cmd.param1) > 0 && strlen(cmd.param2) > 0) {
                            // Use provided credentials
                            connectToNetwork(String(cmd.param1), String(cmd.param2));
                            Serial.printf("[WiFi Task] Connecting to: %s\n", cmd.param1);
                        } else {
                            // Try to load saved config
                            StationConfig config;
                            if (loadStationConfig(config)) {
                                connectToNetwork(String(config.ssid), String(config.password));
                                Serial.printf("[WiFi Task] Connecting to saved network: %s\n", config.ssid);
                            } else {
                                Serial.println("[WiFi Task] ❌ No saved Station config available");
                            }
                        }
                    }
                    break;
                    
                case WIFI_CMD_STOP:
                    Serial.println("[WiFi Task] Stopping WiFi");
                    stopWiFi();
                    break;
                    
                case WIFI_CMD_IDLE:
                    Serial.println("[WiFi Task] Setting WiFi to idle mode");
                    setIdleMode();
                    break;
                    
                default:
                    Serial.printf("[WiFi Task] ❌ Unknown command type: %d\n", cmd.type);
                    break;
            }
            
            Serial.println("[WiFi Task] ✅ Command processed");
        }
    }
}

// ==========================================
// INITIALIZATION
// ==========================================
bool initWiFiTask() {
    // Create command queue (10 commands max)
    wifiCommandQueue = xQueueCreate(10, sizeof(WiFiCommand));
    if (wifiCommandQueue == nullptr) {
        Serial.println("❌ Failed to create WiFi command queue");
        return false;
    }
    
    // Create WiFi command task pinned to Core 1
    // Stack: 8192 bytes (WiFi operations need more stack), Priority: 2 (higher than TFT)
    BaseType_t result = xTaskCreatePinnedToCore(
        wifiCommandTask,     // Task function
        "WiFi_Command",      // Task name
        8192,                // Stack size (bytes) - WiFi needs more
        nullptr,             // Task parameters
        2,                   // Priority (2 = higher than TFT display)
        &wifiTaskHandle,     // Task handle
        1                    // Core ID (1 = app core)
    );
    
    if (result != pdPASS) {
        Serial.println("❌ Failed to create WiFi command task");
        vQueueDelete(wifiCommandQueue);
        wifiCommandQueue = nullptr;
        return false;
    }
    
    Serial.println("✅ WiFi Command Task initialized on Core 1");
    return true;
}

// ==========================================
// COMMAND API FUNCTIONS
// ==========================================
bool requestSwitchToAP() {
    if (wifiCommandQueue == nullptr) return false;
    
    WiFiCommand cmd;
    cmd.type = WIFI_CMD_SWITCH_TO_AP;
    cmd.param1[0] = '\0';
    cmd.param2[0] = '\0';
    
    // Send to queue (don't block if full)
    return xQueueSend(wifiCommandQueue, &cmd, 0) == pdTRUE;
}

bool requestSwitchToAP(const char* ssid, const char* password) {
    if (wifiCommandQueue == nullptr) return false;
    
    WiFiCommand cmd;
    cmd.type = WIFI_CMD_SWITCH_TO_AP;
    
    // Copy strings safely
    strncpy(cmd.param1, ssid, sizeof(cmd.param1) - 1);
    cmd.param1[sizeof(cmd.param1) - 1] = '\0';
    
    strncpy(cmd.param2, password, sizeof(cmd.param2) - 1);
    cmd.param2[sizeof(cmd.param2) - 1] = '\0';
    
    // Send to queue (don't block if full)
    return xQueueSend(wifiCommandQueue, &cmd, 0) == pdTRUE;
}

bool requestSwitchToStation() {
    if (wifiCommandQueue == nullptr) return false;
    
    WiFiCommand cmd;
    cmd.type = WIFI_CMD_SWITCH_TO_STATION;
    cmd.param1[0] = '\0';
    cmd.param2[0] = '\0';
    
    // Send to queue (don't block if full)
    return xQueueSend(wifiCommandQueue, &cmd, 0) == pdTRUE;
}

bool requestSwitchToStation(const char* ssid, const char* password) {
    if (wifiCommandQueue == nullptr) return false;
    
    WiFiCommand cmd;
    cmd.type = WIFI_CMD_SWITCH_TO_STATION;
    
    // Copy strings safely
    strncpy(cmd.param1, ssid, sizeof(cmd.param1) - 1);
    cmd.param1[sizeof(cmd.param1) - 1] = '\0';
    
    strncpy(cmd.param2, password, sizeof(cmd.param2) - 1);
    cmd.param2[sizeof(cmd.param2) - 1] = '\0';
    
    // Send to queue (don't block if full)
    return xQueueSend(wifiCommandQueue, &cmd, 0) == pdTRUE;
}

bool requestStopWiFi() {
    if (wifiCommandQueue == nullptr) return false;
    
    WiFiCommand cmd;
    cmd.type = WIFI_CMD_STOP;
    cmd.param1[0] = '\0';
    cmd.param2[0] = '\0';
    
    // Send to queue (don't block if full)
    return xQueueSend(wifiCommandQueue, &cmd, 0) == pdTRUE;
}

bool requestIdleMode() {
    if (wifiCommandQueue == nullptr) return false;
    
    WiFiCommand cmd;
    cmd.type = WIFI_CMD_IDLE;
    cmd.param1[0] = '\0';
    cmd.param2[0] = '\0';
    
    // Send to queue (don't block if full)
    return xQueueSend(wifiCommandQueue, &cmd, 0) == pdTRUE;
}
