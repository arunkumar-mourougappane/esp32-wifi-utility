#pragma once
#include <Arduino.h>
#include "config.h"
#include "ap_config.h"  // For APSecurityType enum
#include "station_config.h"  // For StationSecurityPreference enum

// ==========================================
// GLOBAL STATE VARIABLES
// ==========================================
extern WiFiMode currentMode;
extern bool scanningEnabled;
extern unsigned long lastScan;
extern unsigned long lastBlink;
extern bool ledState;

// ==========================================
// ACCESS POINT CONFIGURATION VARIABLES
// ==========================================
extern String currentAPSSID;
extern String currentAPPassword;
extern uint8_t currentAPChannel;
extern APSecurityType currentAPSecurity;

// ==========================================
// WIFI STATE MANAGEMENT
// ==========================================
void initializeWiFi();
void startStationMode();
void startAccessPoint();
void startAccessPoint(const String& ssid, const String& password, APSecurityType security = AP_SEC_WPA2_PSK);
void stopWiFi();
void setIdleMode();

// ==========================================
// HELPER FUNCTIONS
// ==========================================
const char* securityTypeToString(APSecurityType security);

// ==========================================
// QR CODE FUNCTIONALITY
// ==========================================

/**
 * @brief Generates and displays a QR code for AP connection information
 * @param ssid Access Point SSID
 * @param password Access Point password
 * @param security Security type (WPA2, WEP, or nopass for open network)
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void generateAPQRCode(const String& ssid, const String& password, const String& security = "WPA");

// ==========================================
// WIFI SCANNING
// ==========================================
void performWiFiScan();

/**
 * @brief Shows detailed information about a specific network from the last scan
 * @param networkId Network ID (1-based index) from the scan results
 */
void showNetworkDetails(int networkId);

// ==========================================
// NETWORK CONNECTION
// ==========================================

/**
 * @brief Connects to a WiFi network using SSID and password (non-blocking)
 * @param ssid Network name to connect to
 * @param password Network password for authentication
 * @param securityPreference Security level requirement (default: AUTO)
 * @note This is non-blocking - call handleWiFiConnection() in main loop
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void connectToNetwork(String ssid, String password, StationSecurityPreference securityPreference = STA_SEC_AUTO);

/**
 * @brief Monitors WiFi connection progress (call from main loop)
 * @note Only active when a connection attempt is in progress
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void handleWiFiConnection();

/**
 * @brief Disconnects from current WiFi network
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void disconnectFromNetwork();


