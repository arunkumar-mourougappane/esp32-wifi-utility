#pragma once

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <qrcode.h>

// TFT Display pins for Adafruit Feather ESP32-S3 TFT
#define TFT_CS        7
#define TFT_DC        39
#define TFT_RST       40
#define TFT_BACKLIGHT 45

// Display dimensions
#define TFT_WIDTH  135
#define TFT_HEIGHT 240

// Display rotation (0, 1, 2, or 3)
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
#define TFT_ROTATION 3  // Reverse TFT
#else
#define TFT_ROTATION 1  // Normal TFT
#endif

// ==========================================
// TFT DISPLAY FUNCTIONS
// ==========================================

/**
 * @brief Initializes the TFT display
 */
void initializeTFT();

/**
 * @brief Clears the TFT display
 */
void clearTFT();

/**
 * @brief Displays QR code for AP mode connection
 * @param ssid WiFi network SSID
 * @param password WiFi network password
 * @param encryption Encryption type (usually "WPA")
 */
void displayAPQRCode(const String& ssid, const String& password, const String& encryption);

/**
 * @brief Displays QR code for Station mode (connected AP)
 * @param ssid WiFi network SSID
 * @param password WiFi network password
 * @param encryption Encryption type (usually "WPA")
 */
void displayStationQRCode(const String& ssid, const String& password, const String& encryption);

/**
 * @brief Update TFT display based on current WiFi mode
 */
void updateTFTDisplay();

/**
 * @brief Displays status text on TFT
 * @param text Status text to display
 * @param clear Whether to clear screen first
 */
void displayStatus(const String& text, bool clear = true);

/**
 * @brief Turns off the TFT backlight
 */
void tftBacklightOff();

/**
 * @brief Turns on the TFT backlight
 */
void tftBacklightOn();

// Global TFT instance
extern Adafruit_ST7789* tft;

#endif // TFT boards
