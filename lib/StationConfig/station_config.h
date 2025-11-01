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
// STATION CONFIGURATION STRUCTURE
// ==========================================

/**
 * @brief Station mode configuration structure
 */
struct StationConfig {
    char ssid[33];        // WiFi SSID (max 32 chars + null terminator)
    char password[64];    // WiFi password (max 63 chars + null terminator)
    bool autoConnect;     // Auto-connect on boot
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
