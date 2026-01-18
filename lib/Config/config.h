/**
 * @file config.h
 * @brief System-wide configuration constants and hardware definitions
 * 
 * This file contains all system-wide configuration parameters including
 * hardware pin assignments, timing constants, WiFi settings, and compile-time
 * validations. It serves as the central configuration hub for the ESP32
 * WiFi Utility project.
 * 
 * @author Arunkumar Mourougappane
 * @version 1.0.0
 * @date 2026-01-17
 */

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
// SYSTEM CONSTANTS (v4.2.0)
// ==========================================

namespace SystemConstants {
    // LED animation constants
    constexpr uint32_t LED_STARTUP_ANIMATION_DELAY_MS = 300;
    constexpr uint32_t LED_UPDATE_INTERVAL_MS = 16;  // ~60 FPS
    constexpr uint32_t LED_PULSE_PERIOD_MS = 2000;
    
    // Command interface constants
    constexpr size_t COMMAND_BUFFER_SIZE = 128;
    constexpr size_t MAX_COMMAND_LENGTH = 256;
    constexpr uint32_t COMMAND_TIMEOUT_MS = 100;
    
    // WiFi constants
    constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 10000;
    constexpr uint32_t WIFI_SCAN_TIMEOUT_MS = 5000;
    constexpr uint8_t WIFI_MAX_RETRY_ATTEMPTS = 3;
    
    // Network constants
    constexpr uint16_t NETWORK_TIMEOUT_MS = 5000;
    constexpr uint16_t DNS_TIMEOUT_MS = 3000;
    constexpr size_t MAX_SSID_LENGTH = 32;
    constexpr size_t MAX_PASSWORD_LENGTH = 63;
}

// Compile-time validation
static_assert(SystemConstants::COMMAND_BUFFER_SIZE > 0, "Command buffer size must be positive");
static_assert(SystemConstants::MAX_COMMAND_LENGTH >= SystemConstants::COMMAND_BUFFER_SIZE, 
              "Max command length must be >= buffer size");
static_assert(SystemConstants::MAX_SSID_LENGTH > 0 && SystemConstants::MAX_SSID_LENGTH <= 32,
              "SSID length must be between 1 and 32");
