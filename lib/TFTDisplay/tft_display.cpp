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

// Forward declarations
void displayStationInfo();
void drawQRCode(const String& qrData, int offsetX, int offsetY);
void displayAPInfo(const String& ssid, const String& password);
void displayStationDetails(const String& ssid, int rssi, const IPAddress& ip);

// Background update variables
static unsigned long lastAPInfoUpdate = 0;
static unsigned long lastStationInfoUpdate = 0;
static const unsigned long AP_INFO_UPDATE_INTERVAL = 5000; // Update every 5 seconds
static const unsigned long STATION_INFO_UPDATE_INTERVAL = 10000; // Update every 10 seconds
static bool isInAPMode = false;
static bool isInStationMode = false;
static bool qrCodeDisplayed = false;

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
    
    Serial.println("✅ TFT Display initialized");
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
void drawQRCode(const String& qrData, int offsetX, int offsetY) {
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

void displayAPInfo(const String& ssid, const String& password) {
    if (!tft) return;
    // Get connected stations
    wifi_sta_list_t stationList;
    (void) esp_wifi_ap_get_sta_list(&stationList);
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
    String displaySSID = ssid;
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
    String displayPass = password;
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
    tft->print("  " + WiFi.softAPIP().toString());
    
    // Display connected clients
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 5) + (TFT_SECTION_GAP));
    tft->setTextColor(ST77XX_CYAN);
    tft->print("  Clients:");
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 6) + (TFT_SECTION_GAP));
    tft->setTextColor(ST77XX_WHITE);
    tft->print("  " + String(stationList.num));
}

void displayStationDetails(const String& ssid, int rssi, const IPAddress& ip) {
    if (!tft) return;
    
    // Display connected network name
    tft->setTextColor(ST77XX_YELLOW);
    tft->setTextSize(1);
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y);
    tft->print("  Connected:");
    
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + TFT_LINE_HEIGHT);
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    
    // Truncate SSID if too long
    String displaySSID = ssid;
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
    tft->print("  " + ip.toString());
    
    // Display signal strength
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 3) + (TFT_SECTION_GAP));
    tft->setTextColor(ST77XX_YELLOW);
    tft->print("  Signal:");
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 4) + (TFT_SECTION_GAP));
    
    // Color based on signal strength
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
// QR CODE DISPLAY
// ==========================================
void displayAPQRCode(const String& ssid, const String& password, const String& encryption) {
    if (!tft) return;
    
    clearTFT();

    // Create WiFi connection string for QR code
    String qrData = "WIFI:T:" + encryption + ";S:" + ssid + ";P:" + password + ";;";
    
    // Calculate QR code position (centered horizontally)
    int moduleSize = 4;
    int qrSize = 25 * moduleSize; // QR code version 3 is typically 25x25 modules
    int offsetX = (TFT_WIDTH - qrSize) / 2;
    int offsetY = 5; // Top margin
    
    // Draw QR code
    drawQRCode(qrData, offsetX, offsetY);
    
    // Display AP information
    displayAPInfo(ssid, password);

    Serial.println("✅ AP Mode QR Code displayed on TFT");
}

void displayStationQRCode(const String& ssid, const String& password, const String& encryption) {
    if (!tft) return;
    
    clearTFT();
    
    // Display mode indicator
    tft->setTextColor(ST77XX_BLUE);
    tft->setTextSize(1);
    tft->setCursor(TFT_INFO_X_OFFSET, TFT_MODE_Y_OFFSET);
    tft->print("  Station Mode");
    
    // Create WiFi connection string for QR code
    String qrData = "WIFI:T:" + encryption + ";S:" + ssid + ";P:" + password + ";;";
    
    // Calculate QR code position (centered horizontally)
    int moduleSize = 4;
    int qrSize = 25 * moduleSize; // QR code version 3 is typically 25x25 modules
    int offsetX = (TFT_WIDTH - qrSize) / 2;
    int offsetY = 0; // Top margin
    
    // Draw QR code
    drawQRCode(qrData, offsetX, offsetY);
    
    // Display station information
    int rssi = WiFi.RSSI();
    IPAddress ip = WiFi.localIP();
    displayStationDetails(ssid, rssi, ip);
    
    Serial.println("✅ Station Mode QR Code displayed on TFT");
}

// ==========================================
// BACKGROUND UPDATE FUNCTIONS
// ==========================================
void updateAPInfoPeriodically() {
    if (!tft || !isInAPMode) return;
    
    unsigned long currentTime = millis();
    
    // Update AP info periodically (but keep QR code)
    if (currentTime - lastAPInfoUpdate >= AP_INFO_UPDATE_INTERVAL) {
        extern String currentAPSSID;
        extern String currentAPPassword;
        
        // Only update the info section, not the entire display
        displayAPInfo(currentAPSSID, currentAPPassword);
        
        lastAPInfoUpdate = currentTime;
        Serial.println("🔄 AP Info updated periodically");
    }
}

void updateStationInfoPeriodically() {
    if (!tft || !isInStationMode) return;
    
    unsigned long currentTime = millis();
    
    // Update Station info periodically (signal strength, IP, etc.)
    if (currentTime - lastStationInfoUpdate >= STATION_INFO_UPDATE_INTERVAL) {
        if (WiFi.status() == WL_CONNECTED) {
            String ssid = WiFi.SSID();
            int rssi = WiFi.RSSI();
            IPAddress ip = WiFi.localIP();
            
            // Only update the info section, not the entire display
            displayStationDetails(ssid, rssi, ip);
            
            lastStationInfoUpdate = currentTime;
            Serial.println("🔄 Station Info updated periodically");
        }
    }
}

void processTFTBackgroundUpdates() {
    updateAPInfoPeriodically();
    updateStationInfoPeriodically();
}

// ==========================================
// DYNAMIC MODE UPDATE
// ==========================================
void updateTFTDisplay() {
    if (!tft) return;
    
    // Get current WiFi mode
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    
    if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA) {
        // AP Mode - show AP QR code (full display)
        extern String currentAPSSID;
        extern String currentAPPassword;
        
        // Only do full display if not already in AP mode or QR code not displayed
        if (!isInAPMode || !qrCodeDisplayed) {
            displayAPQRCode(currentAPSSID, currentAPPassword, "WPA");
            qrCodeDisplayed = true;
            lastAPInfoUpdate = millis(); // Reset timer after full display
        }
        
        isInAPMode = true;
        isInStationMode = false;
        
    } else if (mode == WIFI_MODE_STA && WiFi.status() == WL_CONNECTED) {
        // Station Mode - show connected AP info
        String ssid = WiFi.SSID();
        
        // Only do full display if not already in Station mode
        if (!isInStationMode) {
            displayStationInfo();
            lastStationInfoUpdate = millis(); // Reset timer after full display
        }
        
        isInAPMode = false;
        isInStationMode = true;
        qrCodeDisplayed = false;
        
    } else {
        // Not connected or off
        displayStatus("WiFi\nDisconnected", true);
        
        // Reset all mode flags
        isInAPMode = false;
        isInStationMode = false;
        qrCodeDisplayed = false;
    }
}

void displayStationInfo() {
    if (!tft) return;
    
    clearTFT();
    
    // Display mode indicator
    tft->setTextColor(ST77XX_BLUE);
    tft->setTextSize(1);
    tft->setCursor(0, TFT_MODE_Y_OFFSET);
    tft->print("  Station Mode");
    
    // Display connection status
    if (WiFi.status() == WL_CONNECTED) {
        String ssid = WiFi.SSID();
        int rssi = WiFi.RSSI();
        IPAddress ip = WiFi.localIP();
        
        tft->setTextColor(ST77XX_GREEN);
        tft->setTextSize(1);
        tft->setCursor(0, TFT_INFO_START_Y + TFT_SECTION_GAP);
        tft->print("  Connected!");
        
        // Display SSID
        tft->setCursor(0, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 3) + TFT_SECTION_GAP);
        tft->setTextColor(ST77XX_YELLOW);
        tft->print("  Network:");
        
        tft->setCursor(0, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 4) + TFT_SECTION_GAP);
        tft->setTextColor(ST77XX_WHITE);
        String displaySSID = ssid;
        if (displaySSID.length() > 20) {
            displaySSID = displaySSID.substring(0, 17) + "...";
        }
        tft->print("  " + displaySSID);
        
        // Display IP address
        tft->setCursor(0, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 5) + (TFT_SECTION_GAP * 2));
        tft->setTextColor(ST77XX_CYAN);
        tft->print("  IP Address:");
        
        tft->setCursor(0, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 6) + (TFT_SECTION_GAP * 2));
        tft->setTextColor(ST77XX_WHITE);
        tft->print("  " + ip.toString());
        
        // Display signal strength
        tft->setCursor(0, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 7) + (TFT_SECTION_GAP * 3));
        tft->setTextColor(ST77XX_YELLOW);
        tft->print("  Signal:");
        
        tft->setCursor(0, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 8) + (TFT_SECTION_GAP * 3));
        // Color based on signal strength
        if (rssi >= -60) {
            tft->setTextColor(ST77XX_GREEN);
        } else if (rssi >= -70) {
            tft->setTextColor(ST77XX_YELLOW);
        } else {
            tft->setTextColor(ST77XX_RED);
        }
        tft->print("  " + String(rssi) + " dBm");
        
        // Signal quality bar
        int quality = constrain(2 * (rssi + 100), 0, 100);
        int barWidth = (quality * 100) / 100;  // Max 100 pixels
        int barY = TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 12) + (TFT_SECTION_GAP * 4);
        tft->fillRect(10, barY, barWidth, 10, ST77XX_GREEN);
        tft->drawRect(10, barY, 100, 10, ST77XX_WHITE);
        
        Serial.println("✅ Station Mode info displayed on TFT");
    } else {
        tft->setTextColor(ST77XX_RED);
        tft->setTextSize(2);
        tft->setCursor(10, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 5));
        tft->print("Not");
        tft->setCursor(10, TFT_INFO_START_Y + (TFT_LINE_HEIGHT * 6) + TFT_SECTION_GAP);
        tft->print("Connected");
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
