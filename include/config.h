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

// ==========================================
// RTOS CONFIGURATION (v4.1.0)
// ==========================================
// Set USE_RTOS=1 in platformio.ini to enable RTOS features
// This enables FreeRTOS task-based architecture for:
// - Concurrent operation execution
// - Non-blocking WiFi scans
// - Async command processing
// - Background analysis tasks
// 
// When USE_RTOS is not defined or 0, the system operates
// in legacy synchronous mode for backward compatibility.
//
// See docs/technical/RTOS_REQUIREMENTS.md for details
// ==========================================

#ifdef USE_RTOS
  #if USE_RTOS == 1
    #define RTOS_ENABLED 1
  #else
    #define RTOS_ENABLED 0
  #endif
#else
  #define RTOS_ENABLED 0
#endif
