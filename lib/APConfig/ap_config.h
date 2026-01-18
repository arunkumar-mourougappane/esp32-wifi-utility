/**
 * @file ap_config.h
 * @brief Access Point configuration persistence using NVS
 * 
 * Provides functionality to save and load Access Point configuration
 * (SSID, password, channel) to/from ESP32 Non-Volatile Storage.
 * 
 * @version 4.2.0
 * @date 2025-10-25
 */

#pragma once
#include <Arduino.h>
#include "config.h"

// ==========================================
// AP SECURITY TYPES
// ==========================================

/**
 * @brief WiFi security types for Access Point mode
 */
enum APSecurityType {
    AP_SEC_OPEN = 0,        // No password required (open network)
    AP_SEC_WPA2_PSK = 1,    // WPA2 Personal (default, recommended)
    AP_SEC_WPA3_PSK = 2,    // WPA3 Personal (modern devices only)
    AP_SEC_WPA2_WPA3 = 3    // WPA2/WPA3 Mixed mode (compatibility + security)
};

// ==========================================
// AP CONFIGURATION STRUCTURE
// ==========================================

/**
 * @brief Access Point configuration structure
 */
struct APConfig {
    char ssid[33];        // WiFi SSID (max 32 chars + null terminator)
    char password[64];    // WiFi password (max 63 chars + null terminator)
    uint8_t channel;      // WiFi channel (1-13)
    APSecurityType security;  // Security type (Open, WPA2, WPA3, Mixed)
    bool autoStart;       // Auto-start AP on boot
    bool isValid;         // Configuration validity flag
};

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================

/**
 * @brief Initialize AP configuration system
 * @return true if initialization successful, false otherwise
 */
bool initAPConfig();

/**
 * @brief Save AP configuration to NVS
 * @param config Configuration to save
 * @return true if save successful, false otherwise
 */
bool saveAPConfig(const APConfig& config);

/**
 * @brief Load AP configuration from NVS
 * @param config Configuration structure to populate
 * @return true if load successful, false if no saved config exists
 */
bool loadAPConfig(APConfig& config);

/**
 * @brief Clear saved AP configuration from NVS
 * @return true if clear successful, false otherwise
 */
bool clearAPConfig();

/**
 * @brief Check if AP configuration exists in NVS
 * @return true if configuration exists, false otherwise
 */
bool hasAPConfig();

/**
 * @brief Get default AP configuration
 * @param config Configuration structure to populate with defaults
 */
void getDefaultAPConfig(APConfig& config);

/**
 * @brief Print current AP configuration
 * @param config Configuration to display
 */
void printAPConfig(const APConfig& config);
