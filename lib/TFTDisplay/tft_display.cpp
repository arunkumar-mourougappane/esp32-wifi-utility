#include "tft_display.h"

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)

#define TFT_INFO_X_OFFSET 140

// Standardized Y-offset definitions
#define TFT_MODE_Y_OFFSET 2          // Mode indicator (AP Mode, Station Mode)
#define TFT_INFO_START_Y 20          // Start of information section
#define TFT_LINE_HEIGHT 12           // Height between text lines
#define TFT_SECTION_GAP 16           // Gap between sections

#include "wifi_manager.h"
#include "battery_monitor.h"
#include <WiFi.h>
#include <SPI.h>
#include "esp_wifi.h"
#include <time.h>
#include "esp_sntp.h"

// ==========================================
// BITMAP IMAGES
// ==========================================
// QR Code placeholder bitmap (100x100)
static const unsigned char PROGMEM image_download_1_bits[] = {0xff,0xff,0xff,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0xff,0xff,0xff,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x0f,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x0f,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x0f,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x0f,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x0f,0xf0,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x0f,0xf0,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x0f,0xf0,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x0f,0xf0,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0x00,0xff,0x0f,0xff,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0x00,0xff,0x0f,0xff,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0x00,0xff,0x0f,0xff,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0x00,0xff,0x0f,0xff,0xf0,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0xff,0xff,0xff,0xf0,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0xff,0xff,0xff,0xf0,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0xff,0xff,0xff,0xf0,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0xff,0xf0,0xf0,0xff,0xff,0xff,0xf0,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x00,0xf0,0x00,0xf0,0x0f,0xf0,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0x00,0xf0,0x0f,0xf0,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0x00,0xf0,0x0f,0xf0,0xf0,0xf0,0x00,0x00,0xf0,0xf0,0x00,0x00,0xf0,0x00,0xf0,0x0f,0xf0,0xf0,0xf0,0x00,0x00,0xf0,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xff,0xff,0xff,0xf0,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xff,0xff,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x0f,0xff,0xff,0xf0,0x00,0xf0,0xff,0x0f,0x00,0xf0,0xff,0xf0,0xf0,0x0f,0xff,0xff,0xf0,0x00,0xf0,0xff,0x0f,0x00,0xf0,0xff,0xf0,0xf0,0x0f,0xff,0xff,0xf0,0x00,0xf0,0xff,0x0f,0x00,0xf0,0xff,0xf0,0xf0,0x0f,0xff,0xff,0xf0,0x00,0xf0,0xff,0x0f,0x00,0xf0,0xff,0xf0,0xf0,0xff,0xff,0x00,0x0f,0xf0,0xff,0xff,0x00,0x0f,0xff,0xff,0x00,0xf0,0xff,0xff,0x00,0x0f,0xf0,0xff,0xff,0x00,0x0f,0xff,0xff,0x00,0xf0,0xff,0xff,0x00,0x0f,0xf0,0xff,0xff,0x00,0x0f,0xff,0xff,0x00,0xf0,0xff,0xff,0x00,0x0f,0xf0,0xff,0xff,0x00,0x0f,0xff,0xff,0x00,0xff,0x00,0xff,0xf0,0x0f,0x00,0x00,0x0f,0x00,0x00,0x0f,0x00,0xf0,0xff,0x00,0xff,0xf0,0x0f,0x00,0x00,0x0f,0x00,0x00,0x0f,0x00,0xf0,0xff,0x00,0xff,0xf0,0x0f,0x00,0x00,0x0f,0x00,0x00,0x0f,0x00,0xf0,0xff,0x00,0xff,0xf0,0x0f,0x00,0x00,0x0f,0x00,0x00,0x0f,0x00,0xf0,0xf0,0x00,0x00,0x00,0xf0,0xff,0x00,0xf0,0xff,0xff,0x0f,0xff,0xf0,0xf0,0x00,0x00,0x00,0xf0,0xff,0x00,0xf0,0xff,0xff,0x0f,0xff,0xf0,0xf0,0x00,0x00,0x00,0xf0,0xff,0x00,0xf0,0xff,0xff,0x0f,0xff,0xf0,0xf0,0x00,0x00,0x00,0xf0,0xff,0x00,0xf0,0xff,0xff,0x0f,0xff,0xf0,0x00,0x0f,0x00,0xff,0x0f,0x00,0xf0,0x0f,0x00,0xff,0x00,0x00,0xf0,0x00,0x0f,0x00,0xff,0x0f,0x00,0xf0,0x0f,0x00,0xff,0x00,0x00,0xf0,0x00,0x0f,0x00,0xff,0x0f,0x00,0xf0,0x0f,0x00,0xff,0x00,0x00,0xf0,0x00,0x0f,0x00,0xff,0x0f,0x00,0xf0,0x0f,0x00,0xff,0x00,0x00,0xf0,0xff,0x0f,0xff,0x00,0xf0,0x00,0xff,0xff,0xff,0x00,0xf0,0x0f,0x00,0xff,0x0f,0xff,0x00,0xf0,0x00,0xff,0xff,0xff,0x00,0xf0,0x0f,0x00,0xff,0x0f,0xff,0x00,0xf0,0x00,0xff,0xff,0xff,0x00,0xf0,0x0f,0x00,0xff,0x0f,0xff,0x00,0xf0,0x00,0xff,0xff,0xff,0x00,0xf0,0x0f,0x00,0xff,0xf0,0x00,0xff,0x0f,0x00,0xf0,0xff,0x00,0xf0,0xff,0xff,0xf0,0xff,0xf0,0x00,0xff,0x0f,0x00,0xf0,0xff,0x00,0xf0,0xff,0xff,0xf0,0xff,0xf0,0x00,0xff,0x0f,0x00,0xf0,0xff,0x00,0xf0,0xff,0xff,0xf0,0xff,0xf0,0x00,0xff,0x0f,0x00,0xf0,0xff,0x00,0xf0,0xff,0xff,0xf0,0xf0,0xff,0xff,0x0f,0xff,0x0f,0xf0,0xf0,0xf0,0xff,0x0f,0xf0,0xf0,0xf0,0xff,0xff,0x0f,0xff,0x0f,0xf0,0xf0,0xf0,0xff,0x0f,0xf0,0xf0,0xf0,0xff,0xff,0x0f,0xff,0x0f,0xf0,0xf0,0xf0,0xff,0x0f,0xf0,0xf0,0xf0,0xff,0xff,0x0f,0xff,0x0f,0xf0,0xf0,0xf0,0xff,0x0f,0xf0,0xf0,0xf0,0x00,0xf0,0xff,0xff,0xff,0x0f,0x0f,0xff,0xff,0xf0,0xff,0x00,0xf0,0x00,0xf0,0xff,0xff,0xff,0x0f,0x0f,0xff,0xff,0xf0,0xff,0x00,0xf0,0x00,0xf0,0xff,0xff,0xff,0x0f,0x0f,0xff,0xff,0xf0,0xff,0x00,0xf0,0x00,0xf0,0xff,0xff,0xff,0x0f,0x0f,0xff,0xff,0xf0,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xf0,0xff,0x00,0xf0,0x00,0xf0,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xf0,0xff,0x00,0xf0,0x00,0xf0,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xf0,0xff,0x00,0xf0,0x00,0xf0,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xf0,0xff,0x00,0xf0,0x00,0xf0,0xff,0x00,0xff,0xff,0xff,0xf0,0xf0,0xff,0x00,0xf0,0xf0,0xf0,0xf0,0x00,0xf0,0xff,0xff,0xff,0xf0,0xf0,0xff,0x00,0xf0,0xf0,0xf0,0xf0,0x00,0xf0,0xff,0xff,0xff,0xf0,0xf0,0xff,0x00,0xf0,0xf0,0xf0,0xf0,0x00,0xf0,0xff,0xff,0xff,0xf0,0xf0,0xff,0x00,0xf0,0xf0,0xf0,0xf0,0x00,0xf0,0xf0,0x00,0x00,0xf0,0xff,0xff,0xff,0x0f,0xf0,0x00,0xf0,0x00,0x00,0xf0,0x00,0x00,0xf0,0xff,0xff,0xff,0x0f,0xf0,0x00,0xf0,0x00,0x00,0xf0,0x00,0x00,0xf0,0xff,0xff,0xff,0x0f,0xf0,0x00,0xf0,0x00,0x00,0xf0,0x00,0x00,0xf0,0xff,0xff,0xff,0x0f,0xf0,0x00,0xf0,0x00,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x00,0xff,0xff,0xff,0xf0,0x00,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x00,0xff,0xff,0xff,0xf0,0x00,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x00,0xff,0xff,0xff,0xf0,0x00,0x00,0xf0,0xff,0xf0,0xf0,0xf0,0x00,0x00,0xff,0xff,0xff,0xf0,0x00,0x00,0xf0,0xff,0xf0,0xf0,0xff,0x0f,0xf0,0xf0,0x0f,0xf0,0x00,0x0f,0xf0,0xf0,0xff,0xf0,0xf0,0xff,0x0f,0xf0,0xf0,0x0f,0xf0,0x00,0x0f,0xf0,0xf0,0xff,0xf0,0xf0,0xff,0x0f,0xf0,0xf0,0x0f,0xf0,0x00,0x0f,0xf0,0xf0,0xff,0xf0,0xf0,0xff,0x0f,0xf0,0xf0,0x0f,0xf0,0x00,0x0f,0xf0,0xf0,0xff,0xf0,0xf0,0x0f,0xff,0x00,0x00,0xf0,0x00,0xff,0xff,0xf0,0xf0,0xff,0xf0,0xf0,0x0f,0xff,0x00,0x00,0xf0,0x00,0xff,0xff,0xf0,0xf0,0xff,0xf0,0xf0,0x0f,0xff,0x00,0x00,0xf0,0x00,0xff,0xff,0xf0,0xf0,0xff,0xf0,0xf0,0x0f,0xff,0x00,0x00,0xf0,0x00,0xff,0xff,0xf0,0xf0,0x00,0x00,0xf0,0x0f,0xff,0xf0,0xf0,0x0f,0xff,0xf0,0xff,0xf0,0xf0,0x00,0x00,0xf0,0x0f,0xff,0xf0,0xf0,0x0f,0xff,0xf0,0xff,0xf0,0xf0,0x00,0x00,0xf0,0x0f,0xff,0xf0,0xf0,0x0f,0xff,0xf0,0xff,0xf0,0xf0,0x00,0x00,0xf0,0x0f,0xff,0xf0,0xf0,0x0f,0xff,0xf0,0xff,0xf0,0xff,0xff,0xff,0xf0,0xff,0xf0,0x00,0x0f,0xff,0x00,0x0f,0x00,0xf0,0xff,0xff,0xff,0xf0,0xff,0xf0,0x00,0x0f,0xff,0x00,0x0f,0x00,0xf0,0xff,0xff,0xff,0xf0,0xff,0xf0,0x00,0x0f,0xff,0x00,0x0f,0x00,0xf0,0xff,0xff,0xff,0xf0,0xff,0xf0,0x00,0x0f,0xff,0x00,0x0f,0x00,0xf0};

// Download icon bitmap (60x21)
static const unsigned char PROGMEM image_download_bits[] = {0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x01,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x07,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x08,0x55,0x55,0x50,0x80,0x00,0x00,0x00,0x08,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x08,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x08,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x08,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x07,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x01,0x55,0x55,0x50,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x80,0x40,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

// Lock icon bitmap (7x8)
static const unsigned char PROGMEM image_Lock_bits[] = {0x38,0x44,0x44,0xfe,0xfe,0xee,0xfe,0x7c};

// TFT display instance
Adafruit_ST7789* tft = nullptr;

// FreeRTOS Queue for TFT messages
static QueueHandle_t tftQueue = nullptr;

// Task handle
static TaskHandle_t tftTaskHandle = nullptr;

// NTP and time tracking
static bool ntpSynced = false;
static int lastDisplayedDay = -1;

// Forward declarations for internal functions
static void drawQRCode(const String& qrData, int offsetX, int offsetY);
static void displayAPInfoInternal(const TFTAPInfo& apInfo);
static void displayStationDetailsInternal(const TFTStationInfo& stationInfo);
static void updateTimeDisplay();
static void updateDateDisplay();
static void updateClientsDisplay(uint8_t clients);
static void updateBatteryDisplay(uint8_t percent);
static void initializeNTP();
static void tftDisplayTask(void* parameter);
static void onBatteryUpdate(uint8_t percentage, float voltage);

// ==========================================
// TFT DISPLAY INITIALIZATION
// ==========================================
void initializeTFT() {
    // Enable TFT I2C power (required for both TFT and Reverse TFT boards)
    pinMode(TFT_I2C_POWER, OUTPUT);
    digitalWrite(TFT_I2C_POWER, HIGH);
    delay(10);  // Wait for power to stabilize

    // Initialize SPI with correct pins for Feather ESP32-S3 TFT boards
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);

    // Initialize TFT display
    tft = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
    tft->init(TFT_WIDTH, TFT_HEIGHT);
    tft->setRotation(TFT_ROTATION);
    
    // Initialize backlight
    pinMode(TFT_BACKLIGHT, OUTPUT);
    tftBacklightOn();
    
    // Clear screen with black background
    tft->fillScreen(ST77XX_BLACK);
    
    // Initialize battery monitor with callback
    if (!initializeBatteryMonitor(onBatteryUpdate)) {
        Serial.println("⚠️  Battery monitor initialization failed");
    }
    
    // Create message queue (5 messages max)
    tftQueue = xQueueCreate(5, sizeof(TFTMessage));
    if (tftQueue == nullptr) {
        Serial.println("❌ Failed to create TFT message queue");
        return;
    }
    
    // Create TFT display task pinned to Core 1 (app core)
    // Stack: 4096 bytes, Priority: 1 (low)
    BaseType_t result = xTaskCreatePinnedToCore(
        tftDisplayTask,      // Task function
        "TFT_Display",       // Task name
        4096,                // Stack size (bytes)
        nullptr,             // Task parameters
        1,                   // Priority (1 = low)
        &tftTaskHandle,      // Task handle
        1                    // Core ID (1 = app core)
    );
    
    if (result != pdPASS) {
        Serial.println("❌ Failed to create TFT display task");
        return;
    }
    
    Serial.println("✅ TFT Display initialized with FreeRTOS task on Core 1");
}

// ==========================================
// QUEUE MESSAGE FUNCTIONS
// ==========================================
bool sendTFTAPUpdate(const char* ssid, const char* password, const char* ip, uint8_t clients) {
    if (tftQueue == nullptr) return false;
    
    TFTMessage msg;
    msg.mode = TFT_MODE_AP;
    
    // Copy strings safely
    strncpy(msg.data.ap.ssid, ssid, sizeof(msg.data.ap.ssid) - 1);
    msg.data.ap.ssid[sizeof(msg.data.ap.ssid) - 1] = '\0';
    
    strncpy(msg.data.ap.password, password, sizeof(msg.data.ap.password) - 1);
    msg.data.ap.password[sizeof(msg.data.ap.password) - 1] = '\0';
    
    strncpy(msg.data.ap.ip, ip, sizeof(msg.data.ap.ip) - 1);
    msg.data.ap.ip[sizeof(msg.data.ap.ip) - 1] = '\0';
    
    msg.data.ap.clients = clients;
    
    // Send to queue (don't block if full)
    return xQueueSend(tftQueue, &msg, 0) == pdTRUE;
}

bool sendTFTStationUpdate(const char* ssid, const char* password, const char* ip, int8_t rssi) {
    if (tftQueue == nullptr) return false;
    
    TFTMessage msg;
    msg.mode = TFT_MODE_STATION;
    
    // Copy strings safely
    strncpy(msg.data.station.ssid, ssid, sizeof(msg.data.station.ssid) - 1);
    msg.data.station.ssid[sizeof(msg.data.station.ssid) - 1] = '\0';
    
    strncpy(msg.data.station.password, password, sizeof(msg.data.station.password) - 1);
    msg.data.station.password[sizeof(msg.data.station.password) - 1] = '\0';
    
    strncpy(msg.data.station.ip, ip, sizeof(msg.data.station.ip) - 1);
    msg.data.station.ip[sizeof(msg.data.station.ip) - 1] = '\0';
    
    msg.data.station.rssi = rssi;
    
    // Send to queue (don't block if full)
    return xQueueSend(tftQueue, &msg, 0) == pdTRUE;
}

bool sendTFTStatus(const char* message) {
    if (tftQueue == nullptr) return false;
    
    TFTMessage msg;
    msg.mode = TFT_MODE_STATUS;
    
    // Copy string safely
    strncpy(msg.data.status.message, message, sizeof(msg.data.status.message) - 1);
    msg.data.status.message[sizeof(msg.data.status.message) - 1] = '\0';
    
    // Send to queue (don't block if full)
    return xQueueSend(tftQueue, &msg, 0) == pdTRUE;
}

bool sendTFTClear() {
    if (tftQueue == nullptr) return false;
    
    TFTMessage msg;
    msg.mode = TFT_MODE_OFF;
    
    // Send to queue (don't block if full)
    return xQueueSend(tftQueue, &msg, 0) == pdTRUE;
}

void clearTFT() {
    if (tft) {
        tft->fillScreen(ST77XX_BLACK);
    }
}

void tftBacklightOn() {
    digitalWrite(TFT_BACKLIGHT, HIGH);
}

void tftBacklightOff() {
    digitalWrite(TFT_BACKLIGHT, LOW);
}

void initializeTFTTime() {
    if (!ntpSynced) {
        initializeNTP();
    }
}

// ==========================================
// BATTERY MONITORING CALLBACK
// ==========================================
static void onBatteryUpdate(uint8_t percentage, float voltage) {
    // This callback is called by the battery monitor when battery level changes significantly
    updateBatteryDisplay(percentage);
}

static void updateBatteryDisplay(uint8_t percent) {
    if (!tft) return;
    
    // Battery display configuration
    // Position varies based on rotation
    #if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
        const int batteryX = 203;       // Reverse TFT: rotation 3 (240x135)
    #else
        const int batteryX = 105;       // Normal TFT: rotation 1 (240x135)
    #endif
    const int batteryY = 2;         // Y position
    const int batteryWidth = 28;    // Battery body width
    const int batteryHeight = 12;   // Battery body height
    const int terminalWidth = 2;    // Battery terminal width
    const int terminalHeight = 6;   // Battery terminal height
    const int numBars = 5;          // Number of internal bars
    const int barSpacing = 1;       // Spacing between bars
    const int borderThickness = 1;  // Battery outline thickness
    
    // Calculate how many bars to fill based on percentage
    int filledBars = (percent + 9) / 20;  // 0-20%=1, 21-40%=2, 41-60%=3, 61-80%=4, 81-100%=5
    if (filledBars > numBars) filledBars = numBars;
    if (filledBars < 0) filledBars = 0;
    
    // Choose color based on battery level
    uint16_t fillColor;
    if (percent > 50) {
        fillColor = 0x07E0;  // Green
    } else if (percent > 20) {
        fillColor = 0xFFE0;  // Yellow
    } else {
        fillColor = 0xF800;  // Red
    }
    
    uint16_t outlineColor = 0xFFFF;  // White outline
    uint16_t emptyColor = ST77XX_BLACK;  // Black for empty areas
    
    // Clear entire battery area (including terminal)
    tft->fillRect(batteryX, batteryY, batteryWidth + terminalWidth + 2, batteryHeight, emptyColor);
    
    // Draw battery body outline
    tft->drawRect(batteryX, batteryY, batteryWidth, batteryHeight, outlineColor);
    
    // Draw battery terminal (small rectangle on the right)
    int terminalX = batteryX + batteryWidth;
    int terminalY = batteryY + (batteryHeight - terminalHeight) / 2;
    tft->fillRect(terminalX, terminalY, terminalWidth, terminalHeight, outlineColor);
    
    // Calculate bar dimensions inside the battery
    int innerX = batteryX + borderThickness + 1;
    int innerY = batteryY + borderThickness + 1;
    int innerWidth = batteryWidth - (2 * borderThickness) - 2;
    int innerHeight = batteryHeight - (2 * borderThickness) - 2;
    
    int barWidth = (innerWidth - (numBars - 1) * barSpacing) / numBars;
    
    // Draw battery bars
    for (int i = 0; i < numBars; i++) {
        int barX = innerX + i * (barWidth + barSpacing);
        
        if (i < filledBars) {
            // Filled bar - use fill color
            tft->fillRect(barX, innerY, barWidth, innerHeight, fillColor);
        }
        // Empty bars remain black (already cleared)
    }
}

// ==========================================
// QR CODE UTILITIES
// ==========================================
static void drawQRCode(const String& qrData, int offsetX, int offsetY) {
    if (!tft) return;
    
    // Create QR code (version 3 for WiFi credentials)
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrData.c_str());
    
    // Calculate module size to fit 100x100 pixel area
    // QR code version 3 is 29x29 modules
    // 100 / 29 = ~3.4, so use 3 pixels per module for 87x87, centered in 100x100
    int moduleSize = 3;
    int qrSize = qrcode.size * moduleSize; // Should be ~87x87
    
    // Center the QR code in the 100x100 area
    int centerOffsetX = (100 - qrSize) / 2;
    int centerOffsetY = (100 - qrSize) / 2;
    
    // Draw white background for entire 100x100 QR code area
    tft->fillRect(offsetX, offsetY, 100, 100, ST77XX_WHITE);
    
    // Draw QR code modules centered in the white area
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                tft->fillRect(offsetX + centerOffsetX + (x * moduleSize), 
                            offsetY + centerOffsetY + (y * moduleSize), 
                            moduleSize, moduleSize, 
                            ST77XX_BLACK);
            }
        }
    }
}

static void displayAPInfoInternal(const TFTAPInfo& apInfo) {
    if (!tft) return;
    
    // Clear entire screen
    tft->fillScreen(0x0);
    
    // Define colors (RGB565)
    uint16_t COLOR_GREEN = 0x4B1;    // Green for labels
    uint16_t COLOR_CYAN = 0x5FA;     // Cyan for labels
    uint16_t COLOR_WHITE = 0xFFFF;   // White for values
    uint16_t COLOR_LOCK = 0x5FE0;    // Green for lock icon
    
    tft->setTextWrap(false);
    tft->setTextSize(1);
    
    // MODE: label and value (top left)
    tft->setTextColor(COLOR_GREEN);
    tft->setCursor(4, 4);
    tft->print("MODE:");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(40, 4);
    tft->print("AP");
    
    // Date and time (top right)
    tft->setTextColor(COLOR_WHITE);
    
    // Display current date from RTC
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 0)) {
        // Display date
        tft->setCursor(111, 4);
        const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        char dateStr[7];
        sprintf(dateStr, "%s %02d", months[timeinfo.tm_mon], timeinfo.tm_mday);
        tft->print(dateStr);
        lastDisplayedDay = timeinfo.tm_mday;
        
        // Display time
        tft->setCursor(152, 4);
        char timeStr[9];
        sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        tft->print(timeStr);
    } else {
        // No time available, show placeholders
        tft->setCursor(111, 4);
        tft->print("--- --");
        tft->setCursor(152, 4);
        tft->print("--:--:--");
        lastDisplayedDay = -1;
    }
    
    // SSID: label and value (second line)
    tft->setTextColor(COLOR_GREEN);
    tft->setCursor(4, 17);
    tft->print("SSID:");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(38, 17);
    String displaySSID = String(apInfo.ssid);
    if (displaySSID.length() > 14) {
        displaySSID = displaySSID.substring(0, 11) + "...";
    }
    tft->print(displaySSID);
    
    // Security label and lock icon (right side of SSID line)
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(171, 17);
    tft->print("Security:");
    
    // Determine lock color based on security (green if password protected, red if open)
    bool isSecure = (apInfo.password[0] != '\0' && strlen(apInfo.password) > 0);
    uint16_t lockColor = isSecure ? 0x07E0 : 0xF800;  // Green or Red
    tft->drawBitmap(227, 16, image_Lock_bits, 7, 8, lockColor);
    
    // Download/battery icon (top right corner)
    tft->drawBitmap(175, 1, image_download_bits, 60, 21, COLOR_GREEN);
    
    // Display battery percentage
    uint8_t batteryPercent = getLastBatteryPercent();
    updateBatteryDisplay(batteryPercent);
    
    // QR Code area (left side, starting at y=29)
    tft->drawBitmap(4, 29, image_download_1_bits, 100, 100, COLOR_WHITE);
    
    // Right side information panel (starting at x=111)
    // IP: label and value
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(111, 33);
    tft->print("IP: ");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(141, 33);
    tft->print(apInfo.ip);
    
    // NM: (Netmask) label and value
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(111, 49);
    tft->print("NM: ");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(141, 49);
    tft->print("255.255.255.255");
    
    // BSSID: label
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(111, 63);
    tft->print("BSSID:");
    
    // BSSID: value (on next line for space)
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(111, 76);
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    char bssidStr[18];
    sprintf(bssidStr, "%02x:%02x:%02x:%02x:%02x:%02x", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    tft->print(bssidStr);
    
    // Clients: label and value
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(111, 91);
    tft->print("Clients: ");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(168, 91);
    tft->print(String(apInfo.clients));
}

// Initialize NTP time synchronization
static void initializeNTP() {
    if (WiFi.status() == WL_CONNECTED && !ntpSynced) {
        configTime(-8 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
        // Wait a bit for time to sync
        delay(100);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 2000)) {
            ntpSynced = true;
            Serial.println("✅ NTP time synchronized");
        }
    }
}

// Update only the date display (called when day changes)
static void updateDateDisplay() {
    if (!tft) return;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 0)) {
        return;
    }
    
    // Clear only the date area (6 characters = 36 pixels)
    tft->fillRect(111, 4, 36, 8, ST77XX_BLACK);
    
    // Redraw date
    tft->setTextColor(0xFFFF);
    tft->setTextSize(1);
    tft->setCursor(111, 4);
    
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char dateStr[7];
    sprintf(dateStr, "%s %02d", months[timeinfo.tm_mon], timeinfo.tm_mday);
    tft->print(dateStr);
    
    lastDisplayedDay = timeinfo.tm_mday;
}

// Update only the time display (called every second)
static void updateTimeDisplay() {
    if (!tft) return;
    
    // Get current time from RTC
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 0)) {
        // If time not available, try to sync NTP
        initializeNTP();
        if (!getLocalTime(&timeinfo, 0)) {
            // Still no time, show placeholder
            tft->fillRect(152, 4, 48, 8, ST77XX_BLACK);
            tft->setTextColor(0xFFFF);
            tft->setTextSize(1);
            tft->setCursor(152, 4);
            tft->print("--:--:--");
            return;
        }
    }
    
    // Check if date changed and update if needed
    if (timeinfo.tm_mday != lastDisplayedDay) {
        updateDateDisplay();
    }
    
    // Clear only the time area (8 characters wide at size 1 = 48 pixels)
    tft->fillRect(152, 4, 48, 8, ST77XX_BLACK);
    
    // Redraw time
    tft->setTextColor(0xFFFF);
    tft->setTextSize(1);
    tft->setCursor(152, 4);
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    tft->print(timeStr);
}

// Update only the clients count (called when clients change)
static void updateClientsDisplay(uint8_t clients) {
    if (!tft) return;
    
    // Clear only the clients value area (3 digits max = ~18 pixels)
    tft->fillRect(168, 91, 24, 8, ST77XX_BLACK);
    
    // Redraw clients count
    tft->setTextColor(0xFFFF);
    tft->setTextSize(1);
    tft->setCursor(168, 91);
    tft->print(String(clients));
}

static void displayStationDetailsInternal(const TFTStationInfo& stationInfo) {
    if (!tft) return;
    
    // Clear entire screen
    tft->fillScreen(0x0);
    
    // Define colors (RGB565)
    uint16_t COLOR_GREEN = 0x4B1;    // Green for labels
    uint16_t COLOR_CYAN = 0x5FA;     // Cyan for labels
    uint16_t COLOR_WHITE = 0xFFFF;   // White for values
    uint16_t COLOR_LOCK = 0x5FE0;    // Green for lock icon
    uint16_t COLOR_YELLOW = 0xFFE0;  // Yellow for signal strength
    uint16_t COLOR_RED = 0xF800;     // Red for poor signal
    
    tft->setTextWrap(false);
    tft->setTextSize(1);
    
    // MODE: label and value (top left)
    tft->setTextColor(COLOR_GREEN);
    tft->setCursor(4, 4);
    tft->print("MODE:");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(40, 4);
    tft->print("STA");
    
    // Date and time (top right)
    tft->setTextColor(COLOR_WHITE);
    
    // Display current date from RTC
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 0)) {
        // Display date
        tft->setCursor(111, 4);
        const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        char dateStr[7];
        sprintf(dateStr, "%s %02d", months[timeinfo.tm_mon], timeinfo.tm_mday);
        tft->print(dateStr);
        
        // Display time
        tft->setCursor(152, 4);
        char timeStr[9];
        sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        tft->print(timeStr);
    } else {
        // No time available, show placeholders
        tft->setCursor(111, 4);
        tft->print("--- --");
        tft->setCursor(152, 4);
        tft->print("--:--:--");
    }
    
    // SSID: label and value (second line)
    tft->setTextColor(COLOR_GREEN);
    tft->setCursor(4, 17);
    tft->print("SSID:");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(38, 17);
    String displaySSID = String(stationInfo.ssid);
    if (displaySSID.length() > 14) {
        displaySSID = displaySSID.substring(0, 11) + "...";
    }
    tft->print(displaySSID);
    
    // Security label and lock icon (right side of SSID line)
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(171, 17);
    tft->print("Security:");
    
    // Determine lock color based on security (green if password protected, red if open)
    bool isSecure = (stationInfo.password[0] != '\0' && strlen(stationInfo.password) > 0);
    uint16_t lockColor = isSecure ? 0x07E0 : 0xF800;  // Green or Red
    tft->drawBitmap(227, 16, image_Lock_bits, 7, 8, lockColor);
    
    // Download/battery icon (top right corner)
    tft->drawBitmap(175, 1, image_download_bits, 60, 21, COLOR_GREEN);
    
    // Display battery percentage
    uint8_t batteryPercent = getLastBatteryPercent();
    updateBatteryDisplay(batteryPercent);
    
    // QR Code for WiFi credentials (left side, starting at y=29)
    // Generate QR data for WiFi connection
    String qrData = "WIFI:T:WPA;S:" + String(stationInfo.ssid) + ";P:" + String(stationInfo.password) + ";;";
    drawQRCode(qrData, 4, 29);
    
    // Right side information panel (starting at x=111)
    // IP: label and value
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(111, 33);
    tft->print("IP: ");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(141, 33);
    tft->print(String(stationInfo.ip));
    
    // Signal strength with label
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(111, 49);
    tft->print("RSSI: ");
    
    // Color based on signal strength
    int8_t rssi = stationInfo.rssi;
    if (rssi >= -60) {
        tft->setTextColor(0x07E0);  // Bright green for excellent signal
    } else if (rssi >= -70) {
        tft->setTextColor(COLOR_YELLOW);  // Yellow for good signal
    } else {
        tft->setTextColor(COLOR_RED);  // Red for poor signal
    }
    tft->setCursor(153, 49);
    tft->print(String(rssi));
    tft->setTextColor(COLOR_WHITE);
    tft->print(" dBm");
    
    // Gateway (can be derived or shown as placeholder)
    tft->setTextColor(COLOR_CYAN);
    tft->setCursor(111, 63);
    tft->print("GW: ");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setCursor(141, 63);
    // Extract gateway from IP (assume same subnet, .1 as gateway)
    String ipStr = String(stationInfo.ip);
    int lastDot = ipStr.lastIndexOf('.');
    if (lastDot > 0) {
        String gateway = ipStr.substring(0, lastDot) + ".1";
        tft->print(gateway);
    } else {
        tft->print("---");
    }
}

// ==========================================
// FREERTOS DISPLAY TASK
// ==========================================
static void tftDisplayTask(void* parameter) {
    TFTMessage msg;
    TickType_t lastTimeUpdate = 0;
    TickType_t lastClientsCheck = 0;
    TickType_t lastStationUpdate = 0;
    const TickType_t TIME_UPDATE_INTERVAL = pdMS_TO_TICKS(1000);      // 1 second
    const TickType_t CLIENTS_CHECK_INTERVAL = pdMS_TO_TICKS(1000);    // 1 second
    const TickType_t STATION_UPDATE_INTERVAL = pdMS_TO_TICKS(10000);  // 10 seconds
    
    TFTDisplayMode currentMode = TFT_MODE_OFF;
    TFTAPInfo lastAPInfo = {};
    TFTStationInfo lastStationInfo = {};
    uint8_t lastClientCount = 0;
    
    Serial.println("🎯 TFT Display task started on Core 1");
    
    while (true) {
        // Wait for messages with timeout for periodic updates
        if (xQueueReceive(tftQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Process new message
            switch (msg.mode) {
                case TFT_MODE_OFF:
                    clearTFT();
                    currentMode = TFT_MODE_OFF;
                    Serial.println("🔄 TFT cleared via task");
                    break;
                    
                case TFT_MODE_AP:
                    // Full display update with QR code
                    
                    // Try to initialize NTP if WiFi is connected
                    initializeNTP();
                    
                    // Display AP information first (this clears the screen)
                    displayAPInfoInternal(msg.data.ap);
                    
                    // Now draw QR code over the placeholder bitmap
                    {
                        String qrData = "WIFI:T:WPA;S:" + String(msg.data.ap.ssid) + 
                                       ";P:" + String(msg.data.ap.password) + ";;";
                        
                        // QR code position matches the placeholder area
                        int offsetX = 4;      // Left edge
                        int offsetY = 29;     // Below SSID line
                        
                        // Draw QR code at the placeholder location
                        drawQRCode(qrData, offsetX, offsetY);
                    }
                    
                    // Save for periodic updates
                    lastAPInfo = msg.data.ap;
                    lastClientCount = msg.data.ap.clients;
                    currentMode = TFT_MODE_AP;
                    lastTimeUpdate = xTaskGetTickCount();
                    lastClientsCheck = xTaskGetTickCount();
                    
                    Serial.println("✅ AP Mode displayed via task");
                    break;
                    
                case TFT_MODE_STATION:
                    // Full display update with QR code (only if connected)
                    
                    // Try to initialize NTP if WiFi is connected
                    initializeNTP();
                    
                    // Display station information (this clears screen and shows QR code)
                    displayStationDetailsInternal(msg.data.station);
                    
                    // Save for periodic updates
                    lastStationInfo = msg.data.station;
                    currentMode = TFT_MODE_STATION;
                    lastStationUpdate = xTaskGetTickCount();
                    lastTimeUpdate = xTaskGetTickCount();
                    
                    Serial.println("✅ Station Mode displayed via task");
                    break;
                case TFT_MODE_STATUS:
                    clearTFT();
                    displayStatus(String(msg.data.status.message), false);
                    currentMode = TFT_MODE_STATUS;
                    Serial.println("✅ Status displayed via task");
                    break;
            }
        }
        
        // Handle periodic updates for current mode
        TickType_t currentTick = xTaskGetTickCount();
        
        if (currentMode == TFT_MODE_AP) {
            // Update time every second
            if ((currentTick - lastTimeUpdate) >= TIME_UPDATE_INTERVAL) {
                updateTimeDisplay();
                lastTimeUpdate = currentTick;
            }
            
            // Check client count every second and update only if changed
            if ((currentTick - lastClientsCheck) >= CLIENTS_CHECK_INTERVAL) {
                wifi_sta_list_t stationList;
                if (esp_wifi_ap_get_sta_list(&stationList) == ESP_OK) {
                    if (stationList.num != lastClientCount) {
                        lastClientCount = stationList.num;
                        updateClientsDisplay(lastClientCount);
                    }
                }
                lastClientsCheck = currentTick;
            }
        } else if (currentMode == TFT_MODE_STATION) {
            // Update time every second in Station mode too
            if ((currentTick - lastTimeUpdate) >= TIME_UPDATE_INTERVAL) {
                updateTimeDisplay();
                lastTimeUpdate = currentTick;
            }
            
            // Periodic Station info update - check if still connected
            if ((currentTick - lastStationUpdate) >= STATION_UPDATE_INTERVAL) {
                // Check if WiFi is still connected
                if (WiFi.status() != WL_CONNECTED) {
                    // Device disconnected - show status message
                    displayStatus("WiFi Disconnected", false);
                } else {
                    // Still connected - refresh display
                    displayStationDetailsInternal(lastStationInfo);
                }
                lastStationUpdate = currentTick;
            }
        }
        
        // Small yield to prevent task starvation
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ==========================================
// STATUS DISPLAY
// ==========================================
// STATUS DISPLAY
// ==========================================
void displayStatus(const String& text, bool clear) {
    if (!tft) return;
    
    if (clear) {
        clearTFT();
    }
    
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(2);
    int statusStartY = TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 3);
    tft->setCursor(10, statusStartY);
    
    // Word wrap text
    int maxCharsPerLine = 16;  // Approximate for size 2
    String remaining = text;
    int y = statusStartY;
    
    while (remaining.length() > 0) {
        if (remaining.length() <= maxCharsPerLine) {
            tft->setCursor(10, y);
            tft->print(remaining);
            break;
        }
        
        // Find last space before max chars
        int splitPos = remaining.lastIndexOf(' ', maxCharsPerLine);
        if (splitPos == -1) {
            splitPos = maxCharsPerLine;
        }
        
        String line = remaining.substring(0, splitPos);
        tft->setCursor(10, y);
        tft->print(line);
        
        remaining = remaining.substring(splitPos);
        remaining.trim();
        y += (TFT_LINE_HEIGHT * 2); // Use double line height for size 2 text
        
        if (y > TFT_HEIGHT - (TFT_LINE_HEIGHT * 2)) break;  // Prevent overflow
    }
}

#endif // TFT boards
