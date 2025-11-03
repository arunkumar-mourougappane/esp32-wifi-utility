#pragma once

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <qrcode.h>
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// TFT Display pins - board specific
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
  // Reverse TFT board pins
  #define TFT_CS        42
  #define TFT_DC        40
  #define TFT_RST       41
  #define TFT_BACKLIGHT 45
#else
  // Normal TFT board pins (ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT)
  #define TFT_CS        7
  #define TFT_DC        39
  #define TFT_RST       40
  #define TFT_BACKLIGHT 45
#endif

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
// TFT DISPLAY MESSAGE STRUCTURES
// ==========================================

// Display mode enumeration
enum TFTDisplayMode {
    TFT_MODE_OFF,        // Display cleared
    TFT_MODE_AP,         // Access Point mode with QR code
    TFT_MODE_STATION,    // Station mode with connection info
    TFT_MODE_STATUS      // Generic status message
};

// AP information structure (for queue)
struct TFTAPInfo {
    char ssid[33];       // Max SSID length is 32 + null terminator
    char password[64];   // Max password length is 63 + null terminator
    char ip[16];         // IPv4 address string (xxx.xxx.xxx.xxx)
    uint8_t clients;     // Number of connected clients
};

// Station information structure (for queue)
struct TFTStationInfo {
    char ssid[33];       // Connected network SSID
    char password[64];   // WiFi password (for QR code generation)
    char ip[16];         // Assigned IP address
    int8_t rssi;         // Signal strength in dBm
};

// Status message structure (for queue)
struct TFTStatusInfo {
    char message[128];   // Status message text
};

// Queue message structure
struct TFTMessage {
    TFTDisplayMode mode;
    union {
        TFTAPInfo ap;
        TFTStationInfo station;
        TFTStatusInfo status;
    } data;
};

// ==========================================
// TFT DISPLAY FUNCTIONS
// ==========================================

/**
 * @brief Initializes the TFT display and starts the display task
 */
void initializeTFT();

/**
 * @brief Clears the TFT display
 */
void clearTFT();

/**
 * @brief Send AP mode information to TFT display task
 * @param ssid WiFi network SSID
 * @param password WiFi network password
 * @param ip IP address string
 * @param clients Number of connected clients
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTAPUpdate(const char* ssid, const char* password, const char* ip, uint8_t clients);

/**
 * @brief Send Station mode information to TFT display task
 * @param ssid Connected WiFi network SSID
 * @param password WiFi network password (for QR code)
 * @param ip IP address string
 * @param rssi Signal strength in dBm
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTStationUpdate(const char* ssid, const char* password, const char* ip, int8_t rssi);

/**
 * @brief Send status message to TFT display task
 * @param message Status message to display
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTStatus(const char* message);

/**
 * @brief Clear the TFT display via task queue
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTClear();

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
