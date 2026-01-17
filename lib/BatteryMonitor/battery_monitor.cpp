#include "battery_monitor.h"
#include <Wire.h>
#include "Adafruit_MAX1704X.h"
#include "Adafruit_LC709203F.h"
#include "logging.h"

// Component tag for logging
#define TAG_BATTERY "Battery"

// Battery monitor objects
static Adafruit_MAX17048 maxlipo;
static Adafruit_LC709203F lc;

// FreeRTOS task handle
static TaskHandle_t batteryTaskHandle = nullptr;

// Callback function pointer
static BatteryUpdateCallback updateCallback = nullptr;

// Last known battery state
static float lastBatteryVoltage = 0.0;
static uint8_t lastBatteryPercent = 0;

// Battery monitor type detection
static bool usingMAX17048 = false;
static bool batteryMonitorFound = false;

/**
 * @brief Read the current battery voltage from I2C battery monitor
 * @return Battery voltage in volts (0.0 if error)
 */
float readBatteryVoltage() {
    if (!batteryMonitorFound) {
        return 0.0;
    }
    
    if (usingMAX17048) {
        return maxlipo.cellVoltage();
    } else {
        return lc.cellVoltage();
    }
}

/**
 * @brief Read the current battery percentage
 * @return Battery percentage (0-100)
 */
uint8_t readBatteryPercent() {
    if (!batteryMonitorFound) {
        return 0;
    }
    
    if (usingMAX17048) {
        return (uint8_t)maxlipo.cellPercent();
    } else {
        return (uint8_t)lc.cellPercent();
    }
}

/**
 * @brief Get the last known battery voltage
 * @return Last battery voltage reading in volts
 */
float getLastBatteryVoltage() {
    return lastBatteryVoltage;
}

/**
 * @brief Get the last known battery percentage
 * @return Last battery percentage (0-100)
 */
uint8_t getLastBatteryPercent() {
    return lastBatteryPercent;
}

/**
 * @brief Battery monitoring task
 * Periodically reads battery voltage and notifies callbacks of significant changes
 */
static void batteryMonitorTask(void* parameter) {
    // Initial reading
    lastBatteryVoltage = readBatteryVoltage();
    lastBatteryPercent = readBatteryPercent();
    
    // Notify callback of initial state
    if (updateCallback != nullptr) {
        updateCallback(lastBatteryPercent, lastBatteryVoltage);
    }
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(BATTERY_UPDATE_INTERVAL_MS);
    
    while (true) {
        // Wait for the next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        // Read current battery state
        float currentVoltage = readBatteryVoltage();
        uint8_t currentPercent = readBatteryPercent();
        
        // Check if percentage has changed significantly
        int percentChange = abs((int)currentPercent - (int)lastBatteryPercent);
        
        if (percentChange >= BATTERY_CHANGE_THRESHOLD) {
            // Update stored values
            lastBatteryVoltage = currentVoltage;
            lastBatteryPercent = currentPercent;
            
            // Notify callback
            if (updateCallback != nullptr) {
                updateCallback(currentPercent, currentVoltage);
            }
        }
    }
}

/**
 * @brief Initialize the battery monitoring system
 * @param callback Optional callback function to be called when battery percentage changes significantly
 * @return true if initialization was successful, false otherwise
 */
bool initializeBatteryMonitor(BatteryUpdateCallback callback) {
    // Store callback
    updateCallback = callback;
    
    // Initialize I2C
    Wire.begin();
    
    // Try to detect MAX17048 first (address 0x36)
    if (maxlipo.begin()) {
        usingMAX17048 = true;
        batteryMonitorFound = true;
        LOG_INFO(TAG_BATTERY, "Found MAX17048 battery monitor (Chip ID: 0x%04X)", maxlipo.getChipID());
    }
    // Try LC709203F (address 0x0B)
    else if (lc.begin()) {
        usingMAX17048 = false;
        batteryMonitorFound = true;
        LOG_INFO(TAG_BATTERY, "Found LC709203F battery monitor (Version: 0x%04X)", lc.getICversion());
        
        // Configure LC709203F
        lc.setThermistorB(3950);
        lc.setPackSize(LC709203F_APA_500MAH);  // Adjust based on your battery
        lc.setAlarmVoltage(3.8);
        LOG_DEBUG(TAG_BATTERY, "LC709203F configured: Thermistor=3950, PackSize=500mAh, AlarmV=3.8V");
    }
    else {
        LOG_ERROR(TAG_BATTERY, "No battery monitor found (MAX17048 or LC709203F)");
        batteryMonitorFound = false;
        return false;
    }
    
    // Create battery monitoring task
    BaseType_t result = xTaskCreatePinnedToCore(
        batteryMonitorTask,      // Task function
        "BatteryMonitor",        // Task name
        4096,                    // Stack size
        nullptr,                 // Parameters
        1,                       // Priority (low)
        &batteryTaskHandle,      // Task handle
        1                        // Core 1 (same as TFT task)
    );
    
    if (result != pdPASS) {
        return false;
    }
    
    return true;
}

/**
 * @brief Stop the battery monitoring task
 */
void stopBatteryMonitor() {
    if (batteryTaskHandle != nullptr) {
        vTaskDelete(batteryTaskHandle);
        batteryTaskHandle = nullptr;
    }
}
