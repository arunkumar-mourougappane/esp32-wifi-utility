/**
 * @file tft_display.h
 * @brief TFT display interface for Adafruit Feather ESP32-S3 TFT boards
 * 
 * This header defines the interface for controlling the ST7789 TFT display
 * on Adafruit Feather ESP32-S3 TFT and Reverse TFT boards. Supports display
 * initialization, status rendering, QR code generation, and FreeRTOS task
 * management for efficient display updates.
 * 
 * @author Arunkumar Mourougappane
 * @version 4.3.0
 * @date 2026-01-17
 */

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
  // Reverse TFT board pins (official Adafruit pinout)
  #define TFT_I2C_POWER  7
  #define TFT_CS        42
  #define TFT_RST       41
  #define TFT_DC        40
  #define TFT_BACKLIGHT 45
  // SPI pins for Reverse TFT
  #define TFT_MOSI      35
  #define TFT_MISO      37
  #define TFT_SCK       36
#else
  // Normal TFT board pins (ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT)
  #define TFT_I2C_POWER 21
  #define TFT_CS         7
  #define TFT_RST       40
  #define TFT_DC        39
  #define TFT_BACKLIGHT 45
  // SPI pins for Normal TFT
  #define TFT_MOSI      35
  #define TFT_MISO      37
  #define TFT_SCK       36
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
    TFT_MODE_CONNECTING, // Connecting animation screen
    TFT_MODE_IDLE,       // Station mode idle screen (blue with text)
    TFT_MODE_IDLE_YELLOW,// Idle mode screen (yellow WiFi symbol, no text)
    TFT_MODE_DISABLED,   // WiFi disabled screen
    TFT_MODE_CONNECTION_FAILED, // Connection failed screen with red icon
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
    uint8_t encryptionType; // WiFi encryption type (wifi_auth_mode_t)
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
 * @param encryptionType WiFi encryption type (wifi_auth_mode_t)
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTStationUpdate(const char* ssid, const char* password, const char* ip, int8_t rssi, uint8_t encryptionType);

/**
 * @brief Show connecting animation screen on TFT
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTConnecting();

/**
 * @brief Show Station mode idle screen on TFT
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTIdle();

/**
 * @brief Show idle mode screen with yellow WiFi symbol (no text)
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTIdleYellow();

/**
 * @brief Show WiFi disabled screen on TFT
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTDisabled();

/**
 * @brief Show WiFi connection failed screen with red icon
 * @return true if message was sent successfully, false if queue is full
 */
bool sendTFTConnectionFailed();

/**
 * @brief Show Station mode idle screen on TFT
 */
void displayStationIdleScreen();

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

/**
 * @brief Initialize NTP time synchronization and save to RTC
 * @note Called automatically when WiFi connects, but can be called manually
 * @note Time persists in RTC until power loss, works even without WiFi after sync
 */
void initializeTFTTime();

/**
 * @brief Check if RTC has valid time (not default/epoch time)
 * @return true if RTC has been initialized with valid time, false otherwise
 */
bool tftHasValidTime();

/**
 * @brief Display welcome screen with ESP32 logo and WiFi Utility text
 */
void displayWelcomeScreen();

// Global TFT instance
extern Adafruit_ST7789* tft;

#endif // TFT boards
