#pragma once

// ==========================================
// HARDWARE CONFIGURATION
// ==========================================
#define LED_PIN 2

// NeoPixel configuration for Feather ESP32-S3 TFT
#ifdef USE_NEOPIXEL
  #define NEOPIXEL_PIN 33        // NeoPixel data pin on Feather ESP32-S3 TFT
  #define NEOPIXEL_COUNT 1       // Number of NeoPixels
  #define NEOPIXEL_BRIGHTNESS 50 // Brightness level (0-255)
#endif

// ==========================================
// TIMING CONFIGURATION
// ==========================================
#define BLINK_INTERVAL 250      // LED blink rate (ms)
#define SCAN_INTERVAL 5000      // WiFi scan interval (ms)

// ==========================================
// ACCESS POINT CONFIGURATION
// ==========================================
// Note: Default SSID is dynamically generated as <chip_id>_AABBCCDDEEFF using device MAC address
// This define is kept for backward compatibility but not used by default
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
