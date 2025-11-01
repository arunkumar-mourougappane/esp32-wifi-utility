#include "tft_display.h"

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)

#define TFT_INFO_X_OFFSET 140

// Standardized Y-offset definitions
#define TFT_MODE_Y_OFFSET 2          // Mode indicator (AP Mode, Station Mode)
#define TFT_INFO_START_Y 20          // Start of information section
#define TFT_LINE_HEIGHT 12           // Height between text lines
#define TFT_SECTION_GAP 16           // Gap between sections

#include "wifi_manager.h"
#include <WiFi.h>
#include "esp_wifi.h"

// TFT display instance
Adafruit_ST7789* tft = nullptr;

// FreeRTOS Queue for TFT messages
static QueueHandle_t tftQueue = nullptr;

// Task handle
static TaskHandle_t tftTaskHandle = nullptr;

// Forward declarations for internal functions
static void drawQRCode(const String& qrData, int offsetX, int offsetY);
static void displayAPInfoInternal(const TFTAPInfo& apInfo);
static void displayStationDetailsInternal(const TFTStationInfo& stationInfo);
static void tftDisplayTask(void* parameter);

// ==========================================
// TFT DISPLAY INITIALIZATION
// ==========================================
void initializeTFT() {
    // Initialize TFT display
    tft = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
    tft->init(TFT_WIDTH, TFT_HEIGHT);
    tft->setRotation(TFT_ROTATION);
    
    // Initialize backlight
    pinMode(TFT_BACKLIGHT, OUTPUT);
    tftBacklightOn();
    
    // Clear screen with black background
    tft->fillScreen(ST77XX_BLACK);
    
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

bool sendTFTStationUpdate(const char* ssid, const char* ip, int8_t rssi) {
    if (tftQueue == nullptr) return false;
    
    TFTMessage msg;
    msg.mode = TFT_MODE_STATION;
    
    // Copy strings safely
    strncpy(msg.data.station.ssid, ssid, sizeof(msg.data.station.ssid) - 1);
    msg.data.station.ssid[sizeof(msg.data.station.ssid) - 1] = '\0';
    
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

// ==========================================
// QR CODE UTILITIES
// ==========================================
static void drawQRCode(const String& qrData, int offsetX, int offsetY) {
    if (!tft) return;
    
    // Create QR code
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrData.c_str());
    
    // Calculate QR code display size
    int moduleSize = 4;  // Pixel size of each QR module
    int qrSize = qrcode.size * moduleSize;
    
    // Draw white background for QR code (with small border)
    int borderSize = 8;
    tft->fillRect(offsetX - borderSize, offsetY - borderSize, 
                  qrSize + (borderSize * 2), qrSize + (borderSize * 2), 
                  ST77XX_WHITE);
    
    // Draw QR code modules
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                tft->fillRect(offsetX + (x * moduleSize), 
                            offsetY + (y * moduleSize), 
                            moduleSize, moduleSize, 
                            ST77XX_BLACK);
            }
        }
    }
}

static void displayAPInfoInternal(const TFTAPInfo& apInfo) {
    if (!tft) return;
    
    // Clear the info section (preserve QR code on left side)
    // Clear from x=TFT_INFO_X_OFFSET to right edge, full height
    tft->fillRect(TFT_INFO_X_OFFSET, 0, TFT_WIDTH - TFT_INFO_X_OFFSET, TFT_HEIGHT, ST77XX_BLACK);
    
    // Display mode indicator
    tft->setTextColor(ST77XX_GREEN);
    tft->setTextSize(1);
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_MODE_Y_OFFSET);
    tft->print("  AP Mode");
     
    // Display SSID
    tft->setTextColor(ST77XX_YELLOW);
    tft->setTextSize(1);
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y);
    tft->print("  SSID:");
    
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + TFT_LINE_HEIGHT);
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    
    // Truncate SSID if too long
    String displaySSID = String(apInfo.ssid);
    if (displaySSID.length() > 20) {
        displaySSID = displaySSID.substring(0, 17) + "...";
    }
    tft->print("  " + displaySSID);
    
    // Display password
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 1) + TFT_SECTION_GAP);
    tft->setTextColor(ST77XX_YELLOW);
    tft->print("  Pass:");

    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 2) + TFT_SECTION_GAP);
    tft->setTextColor(ST77XX_WHITE);
    
    // Show password (truncated if needed)
    String displayPass = String(apInfo.password);
    if (displayPass.length() > 20) {
        displayPass = displayPass.substring(0, 17) + "...";
    }
    tft->print("  " + displayPass);
    
    // Display IP address
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 3) + (TFT_SECTION_GAP));
    tft->setTextColor(ST77XX_CYAN);
    tft->print("  IP:");
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 4) + (TFT_SECTION_GAP ));
    tft->setTextColor(ST77XX_WHITE);
    tft->print("  " + String(apInfo.ip));
    
    // Display connected clients
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 5) + (TFT_SECTION_GAP));
    tft->setTextColor(ST77XX_CYAN);
    tft->print("  Clients:");
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 6) + (TFT_SECTION_GAP));
    tft->setTextColor(ST77XX_WHITE);
    tft->print("  " + String(apInfo.clients));
}

static void displayStationDetailsInternal(const TFTStationInfo& stationInfo) {
    if (!tft) return;
    
    // Clear the info section (preserve any QR code or mode indicator)
    // Clear from x=TFT_INFO_X_OFFSET to right edge, from INFO_START_Y down
    tft->fillRect(TFT_INFO_X_OFFSET, TFT_INFO_START_Y, TFT_WIDTH - TFT_INFO_X_OFFSET, 
                  TFT_HEIGHT - TFT_INFO_START_Y, ST77XX_BLACK);
    
    // Display connected network name
    tft->setTextColor(ST77XX_YELLOW);
    tft->setTextSize(1);
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y);
    tft->print("  Connected:");
    
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + TFT_LINE_HEIGHT);
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    
    // Truncate SSID if too long
    String displaySSID = String(stationInfo.ssid);
    if (displaySSID.length() > 20) {
        displaySSID = displaySSID.substring(0, 17) + "...";
    }
    tft->print("  " + displaySSID);
    
    // Display IP address
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 1) + TFT_SECTION_GAP);
    tft->setTextColor(ST77XX_CYAN);
    tft->print("  IP:");
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 2) + TFT_SECTION_GAP);
    tft->setTextColor(ST77XX_WHITE);
    tft->print("  " + String(stationInfo.ip));
    
    // Display signal strength
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 3) + (TFT_SECTION_GAP));
    tft->setTextColor(ST77XX_YELLOW);
    tft->print("  Signal:");
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 4) + (TFT_SECTION_GAP));
    
    // Color based on signal strength
    int8_t rssi = stationInfo.rssi;
    if (rssi >= -60) {
        tft->setTextColor(ST77XX_GREEN);
    } else if (rssi >= -70) {
        tft->setTextColor(ST77XX_YELLOW);
    } else {
        tft->setTextColor(ST77XX_RED);
    }
    tft->print("  " + String(rssi) + " dBm");
}

// ==========================================
// FREERTOS DISPLAY TASK
// ==========================================
static void tftDisplayTask(void* parameter) {
    TFTMessage msg;
    TickType_t lastAPUpdate = 0;
    TickType_t lastStationUpdate = 0;
    const TickType_t AP_UPDATE_INTERVAL = pdMS_TO_TICKS(5000);      // 5 seconds
    const TickType_t STATION_UPDATE_INTERVAL = pdMS_TO_TICKS(10000); // 10 seconds
    
    TFTDisplayMode currentMode = TFT_MODE_OFF;
    TFTAPInfo lastAPInfo = {};
    TFTStationInfo lastStationInfo = {};
    
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
                    clearTFT();
                    
                    // Create WiFi connection string for QR code
                    {
                        String qrData = "WIFI:T:WPA;S:" + String(msg.data.ap.ssid) + 
                                       ";P:" + String(msg.data.ap.password) + ";;";
                        
                        // Calculate QR code position (centered horizontally)
                        int moduleSize = 4;
                        int qrSize = 25 * moduleSize; // QR code version 3
                        int offsetX = (TFT_WIDTH - qrSize) / 2;
                        int offsetY = 5; // Top margin
                        
                        // Draw QR code
                        drawQRCode(qrData, offsetX, offsetY);
                    }
                    
                    // Display AP information
                    displayAPInfoInternal(msg.data.ap);
                    
                    // Save for periodic updates
                    lastAPInfo = msg.data.ap;
                    currentMode = TFT_MODE_AP;
                    lastAPUpdate = xTaskGetTickCount();
                    
                    Serial.println("✅ AP Mode displayed via task");
                    break;
                    
                case TFT_MODE_STATION:
                    // Full display update
                    clearTFT();
                    
                    // Display mode indicator
                    tft->setTextColor(ST77XX_BLUE);
                    tft->setTextSize(1);
                    tft->setCursor(TFT_INFO_X_OFFSET, TFT_MODE_Y_OFFSET);
                    tft->print("  Station Mode");
                    
                    // Create WiFi connection string for QR code (if we had password)
                    // For now, just show station details
                    displayStationDetailsInternal(msg.data.station);
                    
                    // Save for periodic updates
                    lastStationInfo = msg.data.station;
                    currentMode = TFT_MODE_STATION;
                    lastStationUpdate = xTaskGetTickCount();
                    
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
            // Periodic AP info update (keep QR code, update info section only)
            if ((currentTick - lastAPUpdate) >= AP_UPDATE_INTERVAL) {
                // Get real-time client count for periodic updates
                wifi_sta_list_t stationList;
                if (esp_wifi_ap_get_sta_list(&stationList) == ESP_OK) {
                    lastAPInfo.clients = stationList.num;
                }
                
                displayAPInfoInternal(lastAPInfo);
                lastAPUpdate = currentTick;
                Serial.println("🔄 AP Info updated periodically by task");
            }
        } else if (currentMode == TFT_MODE_STATION) {
            // Periodic Station info update
            if ((currentTick - lastStationUpdate) >= STATION_UPDATE_INTERVAL) {
                displayStationDetailsInternal(lastStationInfo);
                lastStationUpdate = currentTick;
                Serial.println("🔄 Station Info updated periodically by task");
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
