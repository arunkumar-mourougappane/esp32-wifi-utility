#include "tft_display.h"

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)

#include "wifi_manager.h"
#include <WiFi.h>

// TFT display instance
Adafruit_ST7789* tft = nullptr;

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
    int offsetY = 10;  // Top margin
    
    // Draw title
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    tft->setCursor(0, 2);
    tft->print("  Scan to Connect");
    
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
    int textY = offsetY + qrSize + borderSize + 10;
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
    
    Serial.println("✅ QR Code displayed on TFT");
}

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
