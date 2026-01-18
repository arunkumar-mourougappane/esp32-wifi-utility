/**
 * @file wifi_manager.h
 * @brief WiFi state management and network operations declarations
 * 
 * This header defines the interface for WiFi management on ESP32, including:
 * - WiFi mode control (Station, AP, Idle)
 * - Network scanning and connection with security validation
 * - Access Point configuration and management
 * - QR code generation for AP credentials
 * - Security preference enforcement
 * 
 * @author Arunkumar Mourougappane
 * @version 1.0.0
 * @date 2026-01-17
 */

#pragma once
#include <Arduino.h>
#include "config.h"
#include "ap_config.h"  // For APSecurityType enum
#include "station_config.h"  // For StationSecurityPreference enum

// ==========================================
// GLOBAL STATE VARIABLES
// ==========================================

/**
 * @brief Current WiFi operational mode
 */
extern WiFiMode currentMode;

/**
 * @brief Flag indicating if WiFi scanning is enabled
 */
extern bool scanningEnabled;

/**
 * @brief Timestamp of last WiFi scan (milliseconds)
 */
extern unsigned long lastScan;

/**
 * @brief Timestamp of last LED blink (milliseconds)
 */
extern unsigned long lastBlink;

/**
 * @brief Current LED state (on/off)
 */
extern bool ledState;

// ==========================================
// ACCESS POINT CONFIGURATION VARIABLES
// ==========================================

/**
 * @brief Current Access Point SSID
 */
extern String currentAPSSID;

/**
 * @brief Current Access Point password
 */
extern String currentAPPassword;

/**
 * @brief Current Access Point WiFi channel (1-13)
 */
extern uint8_t currentAPChannel;

/**
 * @brief Current Access Point security type
 */
extern APSecurityType currentAPSecurity;

// ==========================================
// WIFI STATE MANAGEMENT
// ==========================================

/**
 * @brief Initialize WiFi subsystem
 * @details Sets up WiFi hardware and prepares for mode transitions
 */
void initializeWiFi();

/**
 * @brief Start Station mode for WiFi scanning and connection
 */
void startStationMode();

/**
 * @brief Start Access Point with default configuration
 * @details Uses AP_SSID and AP_PASSWORD from config.h
 */
void startAccessPoint();

/**
 * @brief Start Access Point with custom configuration
 * @param ssid Access Point SSID (network name)
 * @param password Access Point password (8-63 characters for WPA2/WPA3)
 * @param security Security type (default: WPA2-PSK)
 */
void startAccessPoint(const String& ssid, const String& password, APSecurityType security = AP_SEC_WPA2_PSK);

/**
 * @brief Stop all WiFi operations and disable WiFi radio
 */
void stopWiFi();

/**
 * @brief Set WiFi to idle mode (ready but not active)
 */
void setIdleMode();

// ==========================================
// HELPER FUNCTIONS
// ==========================================

/**
 * @brief Convert APSecurityType enum to human-readable string
 * @param security Security type enumeration value
 * @return String representation of security type
 */
const char* securityTypeToString(APSecurityType security);

// ==========================================
// QR CODE FUNCTIONALITY
// ==========================================

/**
 * @brief Generates and displays a QR code for AP connection information
 * @param ssid Access Point SSID
 * @param password Access Point password
 * @param security Security type (WPA, WPA2, WPA3, or nopass for open network)
 * @details QR code follows WIFI:T:WPA;S:ssid;P:password;; format
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void generateAPQRCode(const String& ssid, const String& password, const String& security = "WPA");

// ==========================================
// WIFI SCANNING
// ==========================================

/**
 * @brief Perform a WiFi network scan
 * @details Scans for available networks and displays results with signal strength
 */
void performWiFiScan();

/**
 * @brief Shows detailed information about a specific network from the last scan
 * @param networkId Network ID (1-based index) from the scan results
 * @details Displays SSID, BSSID, channel, RSSI, encryption type
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
 * @details This is non-blocking - call handleWiFiConnection() in main loop
 * @note Validates network security against preference before attempting connection
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void connectToNetwork(String ssid, String password, StationSecurityPreference securityPreference = STA_SEC_AUTO);

/**
 * @brief Monitors WiFi connection progress (call from main loop)
 * @details Only active when a connection attempt is in progress
 * @note Call this function repeatedly in the main loop during connection attempts
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void handleWiFiConnection();

/**
 * @brief Disconnects from current WiFi network
 * @details Cleanly disconnects and returns to Station mode
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void disconnectFromNetwork();


