/**
 * @file battery_monitor.h
 * @brief Battery monitoring system for ESP32 devices with battery support
 * 
 * This header defines the interface for monitoring battery voltage and charge
 * percentage on ESP32 devices equipped with battery management hardware.
 * Uses FreeRTOS task for periodic monitoring and callback notifications.
 * 
 * @author Arunkumar Mourougappane
 * @version 4.3.0
 * @date 2026-01-17
 */

#pragma once

#ifdef BATTERY_MONITOR_SUPPORT

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

/**
 * @brief Check if battery monitor hardware is available
 * @return true if battery monitor found, false otherwise
 */
bool isBatteryMonitorAvailable();

#endif // BATTERY_MONITOR_SUPPORT
