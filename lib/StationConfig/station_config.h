/**
 * @file station_config.h
 * @brief Station mode configuration persistence using NVS
 * 
 * Provides functionality to save and load Station mode configuration
 * (WiFi network credentials) to/from ESP32 Non-Volatile Storage.
 * 
 * @version 1.0
 * @date 2025-10-25
 */

#pragma once
#include <Arduino.h>

// ==========================================
// STATION SECURITY PREFERENCES
// ==========================================

/**
 * @brief Security preferences for Station mode connections
 */
enum StationSecurityPreference {
    STA_SEC_AUTO = 0,         // Auto-negotiate (default, current behavior)
    STA_SEC_WPA3_PREFER = 1,  // Prefer WPA3, fallback to WPA2 if unavailable
    STA_SEC_WPA3_ONLY = 2,    // Require WPA3 (fail if network is WPA2 or lower)
    STA_SEC_WPA2_MIN = 3,     // Minimum WPA2 (reject WEP/Open networks)
    STA_SEC_WPA2_ONLY = 4     // Require exactly WPA2 (reject WPA3, WEP, Open)
};

// ==========================================
// STATION CONFIGURATION STRUCTURE
// ==========================================

/**
 * @brief Station mode configuration structure
 */
struct StationConfig {
    char ssid[33];        // WiFi SSID (max 32 chars + null terminator)
    char password[64];    // WiFi password (max 63 chars + null terminator)
    bool autoConnect;     // Auto-connect on boot
    StationSecurityPreference securityPreference;  // Security preference for connections
    bool isValid;         // Configuration validity flag
};

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================

/**
 * @brief Initialize Station configuration system
 * @return true if initialization successful, false otherwise
 */
bool initStationConfig();

/**
 * @brief Save Station configuration to NVS
 * @param config Configuration to save
 * @return true if save successful, false otherwise
 */
bool saveStationConfig(const StationConfig& config);

/**
 * @brief Load Station configuration from NVS
 * @param config Configuration structure to populate
 * @return true if load successful, false if no saved config exists
 */
bool loadStationConfig(StationConfig& config);

/**
 * @brief Clear saved Station configuration from NVS
 * @return true if clear successful, false otherwise
 */
bool clearStationConfig();

/**
 * @brief Check if Station configuration exists in NVS
 * @return true if configuration exists, false otherwise
 */
bool hasStationConfig();

/**
 * @brief Print current Station configuration
 * @param config Configuration to display
 */
void printStationConfig(const StationConfig& config);
