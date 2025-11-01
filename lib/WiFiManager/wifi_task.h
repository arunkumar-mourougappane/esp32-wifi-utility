#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// ==========================================
// WIFI COMMAND TYPES
// ==========================================
enum WiFiCommandType {
    WIFI_CMD_SWITCH_TO_AP,      // Switch to Access Point mode
    WIFI_CMD_SWITCH_TO_STATION, // Switch to Station mode
    WIFI_CMD_STOP,              // Stop WiFi
    WIFI_CMD_IDLE               // Set to idle mode
};

// ==========================================
// WIFI COMMAND MESSAGE STRUCTURE
// ==========================================
struct WiFiCommand {
    WiFiCommandType type;
    char param1[64];  // For SSID or other parameters
    char param2[64];  // For password or other parameters
};

// ==========================================
// WIFI TASK API
// ==========================================

/**
 * @brief Initialize WiFi command task and queue
 * @return true if initialization successful
 */
bool initWiFiTask();

/**
 * @brief Send command to switch to Access Point mode (uses saved config)
 * @return true if command was queued successfully
 */
bool requestSwitchToAP();

/**
 * @brief Send command to switch to Access Point mode with specific credentials
 * @param ssid Access Point SSID
 * @param password Access Point password
 * @return true if command was queued successfully
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
