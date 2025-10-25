/**
 * @file station_config.cpp
 * @brief Station mode configuration persistence implementation
 * 
 * Implements saving and loading of Station configuration using ESP32 NVS.
 * 
 * @version 1.0
 * @date 2025-10-25
 */

#include "station_config.h"
#include "base64_utils.h"
#include <Preferences.h>

// ==========================================
// CONSTANTS
// ==========================================
static const char* NVS_NAMESPACE = "sta_config";
static const char* KEY_SSID = "ssid";
static const char* KEY_PASSWORD = "password";
static const char* KEY_AUTO_CONNECT = "auto_connect";
static const char* KEY_VALID = "valid";

// ==========================================
// GLOBAL VARIABLES
// ==========================================
static Preferences preferences;

// ==========================================
// INITIALIZATION
// ==========================================

bool initStationConfig() {
    // NVS is automatically initialized by Arduino framework
    Serial.println("[Station Config] Configuration system initialized");
    return true;
}

// ==========================================
// SAVE/LOAD FUNCTIONS
// ==========================================

bool saveStationConfig(const StationConfig& config) {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        Serial.println("[Station Config] ERROR: Failed to open NVS for writing");
        return false;
    }
    
    bool success = true;
    
    // Save SSID
    if (preferences.putString(KEY_SSID, config.ssid) == 0) {
        Serial.println("[Station Config] ERROR: Failed to save SSID");
        success = false;
    }
    
    // Encode password to base64 before saving
    String encodedPassword = base64Encode(String(config.password));
    if (preferences.putString(KEY_PASSWORD, encodedPassword) == 0) {
        Serial.println("[Station Config] ERROR: Failed to save password");
        success = false;
    }
    
    if (preferences.putBool(KEY_AUTO_CONNECT, config.autoConnect) == 0) {
        Serial.println("[Station Config] ERROR: Failed to save auto-connect flag");
        success = false;
    }
    
    if (preferences.putBool(KEY_VALID, true) == 0) {
        Serial.println("[Station Config] ERROR: Failed to save validity flag");
        success = false;
    }
    
    preferences.end();
    
    if (success) {
        Serial.println("[Station Config] ✓ Configuration saved successfully");
    }
    
    return success;
}

bool loadStationConfig(StationConfig& config) {
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        Serial.println("[Station Config] ERROR: Failed to open NVS for reading");
        return false;
    }
    
    // Check if valid configuration exists
    bool isValid = preferences.getBool(KEY_VALID, false);
    if (!isValid) {
        preferences.end();
        Serial.println("[Station Config] No saved configuration found");
        return false;
    }
    
    // Load configuration
    String ssid = preferences.getString(KEY_SSID, "");
    String encodedPassword = preferences.getString(KEY_PASSWORD, "");
    config.autoConnect = preferences.getBool(KEY_AUTO_CONNECT, false);
    
    preferences.end();
    
    // Decode password from base64
    String password = base64Decode(encodedPassword);
    
    // Validate loaded data
    if (ssid.length() == 0 || ssid.length() > 32) {
        Serial.println("[Station Config] ERROR: Invalid SSID in saved configuration");
        return false;
    }
    
    // Password can be empty for open networks
    if (password.length() > 63) {
        Serial.println("[Station Config] ERROR: Invalid password in saved configuration");
        return false;
    }
    
    // Copy strings to config structure
    strncpy(config.ssid, ssid.c_str(), sizeof(config.ssid) - 1);
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    
    strncpy(config.password, password.c_str(), sizeof(config.password) - 1);
    config.password[sizeof(config.password) - 1] = '\0';
    
    config.isValid = true;
    
    Serial.println("[Station Config] ✓ Configuration loaded successfully");
    return true;
}

bool clearStationConfig() {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        Serial.println("[Station Config] ERROR: Failed to open NVS for clearing");
        return false;
    }
    
    bool success = preferences.clear();
    preferences.end();
    
    if (success) {
        Serial.println("[Station Config] ✓ Configuration cleared successfully");
    } else {
        Serial.println("[Station Config] ERROR: Failed to clear configuration");
    }
    
    return success;
}

bool hasStationConfig() {
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        return false;
    }
    
    bool isValid = preferences.getBool(KEY_VALID, false);
    preferences.end();
    
    return isValid;
}

// ==========================================
// DISPLAY FUNCTIONS
// ==========================================

void printStationConfig(const StationConfig& config) {
    Serial.println("==========================================");
    Serial.println("Station Mode Configuration");
    Serial.println("==========================================");
    Serial.printf("  SSID:         %s\n", config.ssid);
    
    // Mask password for security
    if (strlen(config.password) > 0) {
        Serial.print("  Password:     ");
        for (size_t i = 0; i < strlen(config.password); i++) {
            Serial.print("*");
        }
        Serial.println();
    } else {
        Serial.println("  Password:     (none - open network)");
    }
    
    Serial.printf("  Auto-Connect: %s\n", config.autoConnect ? "Yes" : "No");
    Serial.printf("  Valid:        %s\n", config.isValid ? "Yes" : "No");
    Serial.println("==========================================");
}
