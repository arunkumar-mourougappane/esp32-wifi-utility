#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Battery update interval (ms)
#define BATTERY_UPDATE_INTERVAL_MS 30000  // 30 seconds

// Minimum percentage change to trigger update notification
#define BATTERY_CHANGE_THRESHOLD 5  // 5% change

// Callback function type for battery updates
typedef void (*BatteryUpdateCallback)(uint8_t percentage, float voltage);

/**
 * @brief Initialize the battery monitoring system
 * @param callback Optional callback function to be called when battery percentage changes significantly
 * @return true if initialization was successful, false otherwise
 */
bool initializeBatteryMonitor(BatteryUpdateCallback callback = nullptr);

/**
 * @brief Read the current battery voltage
 * @return Battery voltage in volts (0.0 if error)
 */
float readBatteryVoltage();

/**
 * @brief Read the current battery percentage
 * @return Battery percentage (0-100)
 */
uint8_t readBatteryPercent();

/**
 * @brief Get the last known battery voltage
 * @return Last battery voltage reading in volts
 */
float getLastBatteryVoltage();

/**
 * @brief Get the last known battery percentage
 * @return Last battery percentage (0-100)
 */
uint8_t getLastBatteryPercent();

/**
 * @brief Stop the battery monitoring task
 */
void stopBatteryMonitor();
