/**
 * @file wifi_task.h
 * @brief FreeRTOS task for WiFi mode switching and command handling
 * 
 * This header defines the FreeRTOS-based WiFi command task interface for
 * thread-safe WiFi mode switching. Uses a command queue to safely switch
 * between AP, Station, and Idle modes from any thread context.
 * 
 * The task provides asynchronous WiFi control, preventing blocking operations
 * in the main application thread and ensuring clean mode transitions.
 * 
 * @author Arunkumar Mourougappane
 * @version 5.0.0
 * @date 2026-01-17
 */

#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// ==========================================
// WIFI COMMAND TYPES
// ==========================================

/**
 * @brief WiFi command types for mode switching
 */
enum WiFiCommandType {
    WIFI_CMD_SWITCH_TO_AP,      ///< Switch to Access Point mode
    WIFI_CMD_SWITCH_TO_STATION, ///< Switch to Station mode
    WIFI_CMD_STOP,              ///< Stop WiFi
    WIFI_CMD_IDLE               ///< Set to idle mode
};

// ==========================================
// WIFI COMMAND MESSAGE STRUCTURE
// ==========================================

/**
 * @brief WiFi command message for queue communication
 */
struct WiFiCommand {
    WiFiCommandType type;    ///< Command type
    char param1[64];         ///< For SSID or other parameters
    char param2[64];         ///< For password or other parameters
};

// ==========================================
// WIFI TASK API
// ==========================================

/**
 * @brief Initialize WiFi command task and queue
 * @return true if initialization successful
 * @details Creates FreeRTOS task and command queue for WiFi operations
 */
bool initWiFiTask();

/**
 * @brief Send command to switch to Access Point mode (uses saved config)
 * @return true if command was queued successfully
 * @details Loads AP configuration from NVS and starts AP mode
 */
bool requestSwitchToAP();

/**
 * @brief Send command to switch to Access Point mode with specific credentials
 * @param ssid Access Point SSID
 * @param password Access Point password
 * @return true if command was queued successfully
 * @details Starts AP with provided credentials, overriding saved config
 */
bool requestSwitchToAP(const char* ssid, const char* password);

/**
 * @brief Send command to switch to Station mode (uses saved config)
 * @return true if command was queued successfully
 */
bool requestSwitchToStation();

/**
 * @brief Send command to switch to Station mode with specific credentials
 * @param ssid Network SSID to connect to
 * @param password Network password
 * @return true if command was queued successfully
 */
bool requestSwitchToStation(const char* ssid, const char* password);

/**
 * @brief Send command to stop WiFi
 * @return true if command was queued successfully
 */
bool requestStopWiFi();

/**
 * @brief Send command to set WiFi to idle mode
 * @return true if command was queued successfully
 */
bool requestIdleMode();
