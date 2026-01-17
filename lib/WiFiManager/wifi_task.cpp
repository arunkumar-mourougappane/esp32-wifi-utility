#include "wifi_task.h"
#include "wifi_manager.h"
#include "ap_config.h"
#include "station_config.h"
#include "logging.h"
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
#include "tft_display.h"
#endif

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
    
    LOG_INFO(TAG_TASK, "WiFi Command Task started on Core 1");
    
    while (true) {
        // Wait for commands (block indefinitely)
        if (xQueueReceive(wifiCommandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            LOG_DEBUG(TAG_TASK, "Processing WiFi command: %d", cmd.type);
            
            switch (cmd.type) {
                case WIFI_CMD_SWITCH_TO_AP:
                    {
                        LOG_INFO(TAG_TASK, "Switching to Access Point mode");
                        
                        if (strlen(cmd.param1) > 0 && strlen(cmd.param2) > 0) {
                            // Use provided credentials
                            startAccessPoint(String(cmd.param1), String(cmd.param2));
                            LOG_DEBUG(TAG_TASK, "Started AP with SSID: %s", cmd.param1);
                        } else {
                            // Try to load saved config
                            APConfig config;
                            if (loadAPConfig(config)) {
                                startAccessPoint(String(config.ssid), String(config.password));
                                LOG_DEBUG(TAG_TASK, "Started AP with saved config: %s", config.ssid);
                            } else {
                                // Use defaults
                                startAccessPoint();
                                LOG_DEBUG(TAG_TASK, "Started AP with default config");
                            }
                        }
                    }
                    break;
                    
                case WIFI_CMD_SWITCH_TO_STATION:
                    {
                        LOG_INFO(TAG_TASK, "Switching to Station mode");
                        
                        // First set to Station mode
                        startStationMode();
                        
                        if (strlen(cmd.param1) > 0 && strlen(cmd.param2) > 0) {
                            // Use provided credentials
                            connectToNetwork(String(cmd.param1), String(cmd.param2));
                            LOG_DEBUG(TAG_TASK, "Connecting to: %s", cmd.param1);
                        } else {
                            // Try to load saved config
                            StationConfig config;
                            if (loadStationConfig(config)) {
                                connectToNetwork(String(config.ssid), String(config.password));
                                LOG_DEBUG(TAG_TASK, "Connecting to saved network: %s", config.ssid);
                            } else {
                                LOG_ERROR(TAG_TASK, "No saved Station config available");
                            }
                        }
                    }
                    break;
                    
                case WIFI_CMD_STOP:
                    LOG_INFO(TAG_TASK, "Stopping WiFi");
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
                    // Show WiFi disabled screen when stopping via command
                    sendTFTDisabled();
#endif
                    stopWiFi();
                    break;
                    
                case WIFI_CMD_IDLE:
                    LOG_INFO(TAG_TASK, "Setting WiFi to idle mode");
                    setIdleMode();
                    break;
                    
                default:
                    LOG_ERROR(TAG_TASK, "Unknown command type: %d", cmd.type);
                    break;
            }
            
            LOG_DEBUG(TAG_TASK, "Command processed");
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
        LOG_ERROR(TAG_TASK, "Failed to create WiFi command queue");
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
        LOG_ERROR(TAG_TASK, "Failed to create WiFi command task");
        vQueueDelete(wifiCommandQueue);
        wifiCommandQueue = nullptr;
        return false;
    }
    
    LOG_INFO(TAG_TASK, "WiFi Command Task initialized on Core 1");
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
