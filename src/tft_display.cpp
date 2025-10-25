#include "tft_display.h"

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)

#include "wifi_manager.h"
#include <WiFi.h>
#include "esp_wifi.h"

// TFT display instance
Adafruit_ST7789* tft = nullptr;

// Forward declaration
void displayStationInfo();

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
// QR CODE DISPLAY
// ==========================================
void displayAPQRCode(const String& ssid, const String& password, const String& encryption) {
    if (!tft) return;
    
    clearTFT();
    
    // Display mode indicator
    tft->setTextColor(ST77XX_GREEN);
    tft->setTextSize(1);
    tft->setCursor(0, 2);
    tft->print("  AP Mode");
    
    // Create WiFi connection string for QR code
    String qrData = "WIFI:T:" + encryption + ";S:" + ssid + ";P:" + password + ";;";
    
    // Create QR code
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrData.c_str());
    
    // Calculate QR code display size to fit screen
    int moduleSize = 4;  // Pixel size of each QR module
    int qrSize = qrcode.size * moduleSize;
    
    // Calculate position to center QR code
    int offsetX = (TFT_WIDTH - qrSize) / 2;
    int offsetY = 18;  // Top margin (adjusted for mode text)
    
    // Draw white background for QR code (with small border)
    int borderSize = 8;
    tft->fillRect(offsetX - borderSize, offsetY - borderSize, 
                  qrSize + (borderSize * 2), qrSize + (borderSize * 2), 
                  ST77XX_WHITE);
    
    // Draw QR code
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
    
    // Display SSID below QR code
    int textY = offsetY + qrSize + borderSize + 5;
    tft->setTextColor(ST77XX_YELLOW);
    tft->setTextSize(1);
    tft->setCursor(0, textY);
    tft->print("  SSID:");
    
    tft->setCursor(0, textY + 12);
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    
    // Truncate SSID if too long
    String displaySSID = ssid;
    if (displaySSID.length() > 20) {
        displaySSID = displaySSID.substring(0, 17) + "...";
    }
    tft->print("  " + displaySSID);
    
    // Display password
    tft->setCursor(0, textY + 28);
    tft->setTextColor(ST77XX_YELLOW);
    tft->print("  Pass:");
    
    tft->setCursor(0, textY + 40);
    tft->setTextColor(ST77XX_WHITE);
    
    // Show password or asterisks
    String displayPass = password;
    if (displayPass.length() > 20) {
        displayPass = displayPass.substring(0, 17) + "...";
    }
    tft->print("  " + displayPass);
    
    // Display IP address
    tft->setCursor(0, textY + 56);
    tft->setTextColor(ST77XX_CYAN);
    tft->print("  IP: ");
    tft->setTextColor(ST77XX_WHITE);
    tft->print(WiFi.softAPIP().toString());
    
    Serial.println("✅ AP Mode QR Code displayed on TFT");
}

void displayStationQRCode(const String& ssid, const String& password, const String& encryption) {
    if (!tft) return;
    
    clearTFT();
    
    // Display mode indicator
    tft->setTextColor(ST77XX_BLUE);
    tft->setTextSize(1);
    tft->setCursor(0, 2);
    tft->print("  Station Mode");
    
    // Create WiFi connection string for QR code
    String qrData = "WIFI:T:" + encryption + ";S:" + ssid + ";P:" + password + ";;";
    
    // Create QR code
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrData.c_str());
    
    // Calculate QR code display size to fit screen
    int moduleSize = 4;  // Pixel size of each QR module
    int qrSize = qrcode.size * moduleSize;
    
    // Calculate position to center QR code
    int offsetX = (TFT_WIDTH - qrSize) / 2;
    int offsetY = 18;  // Top margin (adjusted for mode text)
    
    // Draw white background for QR code (with small border)
    int borderSize = 8;
    tft->fillRect(offsetX - borderSize, offsetY - borderSize, 
                  qrSize + (borderSize * 2), qrSize + (borderSize * 2), 
                  ST77XX_WHITE);
    
    // Draw QR code
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
    
    // Display SSID below QR code
    int textY = offsetY + qrSize + borderSize + 5;
    tft->setTextColor(ST77XX_YELLOW);
    tft->setTextSize(1);
    tft->setCursor(0, textY);
    tft->print("  Connected:");
    
    tft->setCursor(0, textY + 12);
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    
    // Truncate SSID if too long
    String displaySSID = ssid;
    if (displaySSID.length() > 20) {
        displaySSID = displaySSID.substring(0, 17) + "...";
    }
    tft->print("  " + displaySSID);
    
    // Display IP address
    tft->setCursor(0, textY + 28);
    tft->setTextColor(ST77XX_CYAN);
    tft->print("  IP: ");
    tft->setTextColor(ST77XX_WHITE);
    tft->print(WiFi.localIP().toString());
    
    // Display signal strength
    tft->setCursor(0, textY + 44);
    tft->setTextColor(ST77XX_YELLOW);
    tft->print("  Signal: ");
    tft->setTextColor(ST77XX_WHITE);
    int rssi = WiFi.RSSI();
    tft->print(String(rssi) + " dBm");
    
    Serial.println("✅ Station Mode QR Code displayed on TFT");
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
        // AP Mode - show AP QR code
        extern String currentAPSSID;
        extern String currentAPPassword;
        displayAPQRCode(currentAPSSID, currentAPPassword, "WPA");
    } else if (mode == WIFI_MODE_STA && WiFi.status() == WL_CONNECTED) {
        // Station Mode - show connected AP QR code
        String ssid = WiFi.SSID();
        // Note: We don't have the password in station mode, so we'll just show network info
        displayStationInfo();
    } else {
        // Not connected or off
        displayStatus("WiFi\nDisconnected", true);
    }
}

void displayStationInfo() {
    if (!tft) return;
    
    clearTFT();
    
    // Display mode indicator
    tft->setTextColor(ST77XX_BLUE);
    tft->setTextSize(1);
    tft->setCursor(0, 2);
    tft->print("  Station Mode");
    
    // Display connection status
    if (WiFi.status() == WL_CONNECTED) {
        String ssid = WiFi.SSID();
        int rssi = WiFi.RSSI();
        IPAddress ip = WiFi.localIP();
        
        tft->setTextColor(ST77XX_GREEN);
        tft->setTextSize(1);
        tft->setCursor(0, 30);
        tft->print("  Connected!");
        
        // Display SSID
        tft->setCursor(0, 55);
        tft->setTextColor(ST77XX_YELLOW);
        tft->print("  Network:");
        
        tft->setCursor(0, 70);
        tft->setTextColor(ST77XX_WHITE);
        String displaySSID = ssid;
        if (displaySSID.length() > 20) {
            displaySSID = displaySSID.substring(0, 17) + "...";
        }
        tft->print("  " + displaySSID);
        
        // Display IP address
        tft->setCursor(0, 95);
        tft->setTextColor(ST77XX_CYAN);
        tft->print("  IP Address:");
        
        tft->setCursor(0, 110);
        tft->setTextColor(ST77XX_WHITE);
        tft->print("  " + ip.toString());
        
        // Display signal strength
        tft->setCursor(0, 135);
        tft->setTextColor(ST77XX_YELLOW);
        tft->print("  Signal:");
        
        tft->setCursor(0, 150);
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
        tft->fillRect(10, 170, barWidth, 10, ST77XX_GREEN);
        tft->drawRect(10, 170, 100, 10, ST77XX_WHITE);
        
        Serial.println("✅ Station Mode info displayed on TFT");
    } else {
        tft->setTextColor(ST77XX_RED);
        tft->setTextSize(2);
        tft->setCursor(10, 80);
        tft->print("Not");
        tft->setCursor(10, 100);
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
    tft->setCursor(10, 50);
    
    // Word wrap text
    int maxCharsPerLine = 16;  // Approximate for size 2
    String remaining = text;
    int y = 50;
    
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
        y += 20;
        
        if (y > TFT_HEIGHT - 20) break;  // Prevent overflow
    }
}

#endif // TFT boards
