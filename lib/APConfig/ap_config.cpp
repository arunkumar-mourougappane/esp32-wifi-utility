/**
 * @file ap_config.cpp
 * @brief Access Point configuration persistence implementation
 * 
 * Implements saving and loading of AP configuration using ESP32 NVS.
 * 
 * @version 1.0
 * @date 2025-10-25
 */

#include "ap_config.h"
#include "config.h"
#include "base64_utils.h"
#include <Preferences.h>

// ==========================================
// CONSTANTS
// ==========================================
static const char* NVS_NAMESPACE = "ap_config";
static const char* KEY_SSID = "ssid";
static const char* KEY_PASSWORD = "password";
static const char* KEY_CHANNEL = "channel";
static const char* KEY_AUTO_START = "auto_start";
static const char* KEY_VALID = "valid";

// Default channel
static const uint8_t DEFAULT_CHANNEL = 1;

// ==========================================
// GLOBAL VARIABLES
// ==========================================
static Preferences preferences;

// ==========================================
// INITIALIZATION
// ==========================================

bool initAPConfig() {
    // NVS is automatically initialized by Arduino framework
    Serial.println("[AP Config] Configuration system initialized");
    return true;
}

// ==========================================
// SAVE/LOAD FUNCTIONS
// ==========================================

bool saveAPConfig(const APConfig& config) {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        Serial.println("[AP Config] ERROR: Failed to open NVS for writing");
        return false;
    }
    
    bool success = true;
    
    // Save SSID
    if (preferences.putString(KEY_SSID, config.ssid) == 0) {
        Serial.println("[AP Config] ERROR: Failed to save SSID");
        success = false;
    }
    
    // Encode password to base64 before saving
    String encodedPassword = base64Encode(String(config.password));
    if (preferences.putString(KEY_PASSWORD, encodedPassword) == 0) {
        Serial.println("[AP Config] ERROR: Failed to save password");
        success = false;
    }
    
    if (preferences.putUChar(KEY_CHANNEL, config.channel) == 0) {
        Serial.println("[AP Config] ERROR: Failed to save channel");
        success = false;
    }
    
    if (preferences.putBool(KEY_AUTO_START, config.autoStart) == 0) {
        Serial.println("[AP Config] ERROR: Failed to save auto-start flag");
        success = false;
    }
    
    if (preferences.putBool(KEY_VALID, true) == 0) {
        Serial.println("[AP Config] ERROR: Failed to save validity flag");
        success = false;
    }
    
    preferences.end();
    
    if (success) {
        Serial.println("[AP Config] ✓ Configuration saved successfully");
    }
    
    return success;
}

bool loadAPConfig(APConfig& config) {
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        Serial.println("[AP Config] ERROR: Failed to open NVS for reading");
        return false;
    }
    
    // Check if valid configuration exists
    bool isValid = preferences.getBool(KEY_VALID, false);
    if (!isValid) {
        preferences.end();
        Serial.println("[AP Config] No saved configuration found");
        return false;
    }
    
    // Load configuration
    String ssid = preferences.getString(KEY_SSID, "");
    String encodedPassword = preferences.getString(KEY_PASSWORD, "");
    config.channel = preferences.getUChar(KEY_CHANNEL, DEFAULT_CHANNEL);
    config.autoStart = preferences.getBool(KEY_AUTO_START, true);
    
    preferences.end();
    
    // Decode password from base64
    String password = base64Decode(encodedPassword);
    
    // Validate loaded data
    if (ssid.length() == 0 || ssid.length() > 32) {
        Serial.println("[AP Config] ERROR: Invalid SSID in saved configuration");
        return false;
    }
    
    if (password.length() < 8 || password.length() > 63) {
        Serial.println("[AP Config] ERROR: Invalid password in saved configuration");
        return false;
    }
    
    if (config.channel < 1 || config.channel > 13) {
        Serial.println("[AP Config] ERROR: Invalid channel in saved configuration");
        config.channel = DEFAULT_CHANNEL;
    }
    
    // Copy strings to config structure
    strncpy(config.ssid, ssid.c_str(), sizeof(config.ssid) - 1);
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    
    strncpy(config.password, password.c_str(), sizeof(config.password) - 1);
    config.password[sizeof(config.password) - 1] = '\0';
    
    config.isValid = true;
    
    Serial.println("[AP Config] ✓ Configuration loaded successfully");
    return true;
}

bool clearAPConfig() {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        Serial.println("[AP Config] ERROR: Failed to open NVS for clearing");
        return false;
    }
    
    bool success = preferences.clear();
    preferences.end();
    
    if (success) {
        Serial.println("[AP Config] ✓ Configuration cleared successfully");
    } else {
        Serial.println("[AP Config] ERROR: Failed to clear configuration");
    }
    
    return success;
}

bool hasAPConfig() {
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        return false;
    }
    
    bool isValid = preferences.getBool(KEY_VALID, false);
    preferences.end();
    
    return isValid;
}

// ==========================================
// DEFAULT CONFIGURATION
// ==========================================

void getDefaultAPConfig(APConfig& config) {
    strncpy(config.ssid, AP_SSID, sizeof(config.ssid) - 1);
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    
    strncpy(config.password, AP_PASSWORD, sizeof(config.password) - 1);
    config.password[sizeof(config.password) - 1] = '\0';
    
    config.channel = DEFAULT_CHANNEL;
    config.autoStart = false;  // Don't auto-start with default config
    config.isValid = true;
}

// ==========================================
// DISPLAY FUNCTIONS
// ==========================================

void printAPConfig(const APConfig& config) {
    Serial.println("==========================================");
    Serial.println("Access Point Configuration");
    Serial.println("==========================================");
    Serial.printf("  SSID:       %s\n", config.ssid);
    Serial.printf("  Password:   %s\n", config.password);
    Serial.printf("  Channel:    %d\n", config.channel);
    Serial.printf("  Auto-Start: %s\n", config.autoStart ? "Yes" : "No");
    Serial.printf("  Valid:      %s\n", config.isValid ? "Yes" : "No");
    Serial.println("==========================================");
}
