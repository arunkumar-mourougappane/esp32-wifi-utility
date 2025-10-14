#pragma once
#include <Arduino.h>
#include "config.h"

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

// ==========================================
// WIFI STATE MANAGEMENT
// ==========================================
void initializeWiFi();
void startStationMode();
void startAccessPoint();
void startAccessPoint(const String& ssid, const String& password);
void stopWiFi();
void setIdleMode();

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

// ==========================================
// NETWORK CONNECTION
// ==========================================

/**
 * @brief Connects to a WiFi network using SSID and password
 * @param ssid Network name to connect to
 * @param password Network password for authentication
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void connectToNetwork(String ssid, String password);

/**
 * @brief Disconnects from current WiFi network
 * @see wifi_manager.cpp for detailed implementation documentation
 */
void disconnectFromNetwork();


