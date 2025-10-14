#pragma once

// ==========================================
// HARDWARE CONFIGURATION
// ==========================================
#define LED_PIN 2

// ==========================================
// TIMING CONFIGURATION
// ==========================================
#define BLINK_INTERVAL 250      // LED blink rate (ms)
#define SCAN_INTERVAL 5000      // WiFi scan interval (ms)

// ==========================================
// ACCESS POINT CONFIGURATION
// ==========================================
#define AP_SSID "ESP32-WiFiScanner"
#define AP_PASSWORD "123456789"  // Minimum 8 characters for WPA2

// ==========================================
// WIFI MODES
// ==========================================
enum WiFiMode {
  MODE_IDLE,       // Initial idle state - ready but inactive
  MODE_STATION,    // Station mode for scanning
  MODE_AP,         // Access Point mode
  MODE_OFF         // WiFi disabled
};
