#include "battery_monitor.h"

// FreeRTOS task handle
static TaskHandle_t batteryTaskHandle = nullptr;

// Callback function pointer
static BatteryUpdateCallback updateCallback = nullptr;

// Last known battery state
static float lastBatteryVoltage = 0.0;
static uint8_t lastBatteryPercent = 0;

/**
 * @brief Read the current battery voltage from ADC
 * @return Battery voltage in volts
 */
float readBatteryVoltage() {
    // Read ADC value (12-bit: 0-4095)
    uint16_t adcValue = analogRead(VBAT_PIN);
    
    // Convert to voltage
    // ADC reads 0-3.3V, but battery voltage is divided by 2
    // So actual voltage = (adcValue / 4095) * 3.3 * 2
    float voltage = (adcValue / 4095.0) * 3.3 * 2.0;
    
    return voltage;
}

/**
 * @brief Read the current battery percentage
 * @return Battery percentage (0-100)
 */
uint8_t readBatteryPercent() {
    float voltage = readBatteryVoltage();
    
    // Convert voltage to percentage (0-100)
    // Clamp voltage to valid range
    if (voltage >= BATTERY_MAX_VOLTAGE) {
        return 100;
    }
    if (voltage <= BATTERY_MIN_VOLTAGE) {
        return 0;
    }
    
    // Linear interpolation between min and max voltage
    float percent = ((voltage - BATTERY_MIN_VOLTAGE) / 
                     (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0;
    
    return (uint8_t)percent;
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
    
    // Configure ADC pin
    pinMode(VBAT_PIN, INPUT);
    analogReadResolution(12);  // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db);  // Full range: 0-3.3V
    
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
