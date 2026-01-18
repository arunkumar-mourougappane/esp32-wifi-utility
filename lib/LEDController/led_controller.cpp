/**
 * @file led_controller.cpp
 * @brief LED and NeoPixel control implementation
 * 
 * This file implements LED control functionality for both standard GPIO LEDs
 * and Adafruit NeoPixel RGB LEDs. Features include:
 * - Status indication with color-coded states
 * - Blinking and pulsing effects
 * - WiFi mode-specific visual feedback
 * - Smooth brightness transitions
 * 
 * @author Arunkumar Mourougappane
 * @version 1.0.0
 * @date 2026-01-17
 */

#include "led_controller.h"
#include "wifi_manager.h"

#ifdef USE_NEOPIXEL
// NeoPixel instance
Adafruit_NeoPixel neoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

// ==========================================
// LED INITIALIZATION
// ==========================================
void initializeLED() {
#ifdef USE_NEOPIXEL
  Serial.println("🌈 Initializing NeoPixel LED...");
  neoPixel.begin();
  neoPixel.setBrightness(NEOPIXEL_BRIGHTNESS);
  neoPixel.show(); // Initialize all pixels to 'off'
  
  // Startup animation
  setNeoPixelColor(255, 0, 255); // Purple startup
  delay(500);
  setNeoPixelColor(0, 255, 0); // Green ready
  delay(500);
  setNeoPixelColor(0, 0, 0); // Off
  Serial.println("✅ NeoPixel LED initialized");
#else
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("✅ Standard LED initialized");
#endif
}

// ==========================================
// LED CONTROL FUNCTIONS
// ==========================================
void blinkLED() {
#ifdef USE_NEOPIXEL
  // NeoPixel blink handled in updateLEDStatus
  return;
#else
  if (millis() - lastBlink >= BLINK_INTERVAL) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }
#endif
}

void updateLEDStatus() {
#ifdef USE_NEOPIXEL
  setNeoPixelStatus(currentMode, scanningEnabled);
#else
  // Standard LED status indication based on current mode
  if (currentMode == MODE_AP || scanningEnabled) {
    blinkLED(); // Blink when AP is active or scanning
  } else if (currentMode == MODE_IDLE) {
    // Slow pulse in idle mode
    static unsigned long idlePulseTime = 0;
    if (millis() - idlePulseTime >= 2000) { // Pulse every 2 seconds
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      idlePulseTime = millis();
    }
  } else {
    digitalWrite(LED_PIN, LOW); // LED off when WiFi disabled
  }
#endif
}

#ifdef USE_NEOPIXEL
// ==========================================
// NEOPIXEL FUNCTIONS
// ==========================================
void setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b) {
  neoPixel.setPixelColor(0, neoPixel.Color(r, g, b));
  neoPixel.show();
}

void setNeoPixelStatus(WiFiMode mode, bool scanning) {
  static unsigned long lastUpdate = 0;
  static bool blinkState = false;
  static unsigned long lastPulse = 0;
  static uint8_t pulseDirection = 0; // 0 = fade up, 1 = fade down
  static uint8_t pulseBrightness = 0;
  
  unsigned long now = millis();
  
  switch (mode) {
    case MODE_IDLE:
      // Blue pulse for idle mode
      if (now - lastPulse >= 20) { // Smooth pulse animation
        if (pulseDirection == 0) {
          pulseBrightness += 2;
          if (pulseBrightness >= 100) pulseDirection = 1;
        } else {
          pulseBrightness -= 2;
          if (pulseBrightness <= 10) pulseDirection = 0;
        }
        setNeoPixelColor(0, 0, pulseBrightness);
        lastPulse = now;
      }
      break;
      
    case MODE_STATION:
      if (scanning) {
        // Cyan blink for scanning
        if (now - lastUpdate >= 200) {
          blinkState = !blinkState;
          if (blinkState) {
            setNeoPixelColor(0, 255, 255); // Cyan
          } else {
            setNeoPixelColor(0, 50, 50);   // Dim cyan
          }
          lastUpdate = now;
        }
      } else {
        // Solid green for station mode (ready)
        setNeoPixelColor(0, 255, 0); // Green
      }
      break;
      
    case MODE_AP:
      // Orange blink for AP mode
      if (now - lastUpdate >= 300) {
        blinkState = !blinkState;
        if (blinkState) {
          setNeoPixelColor(255, 165, 0); // Orange
        } else {
          setNeoPixelColor(50, 25, 0);   // Dim orange
        }
        lastUpdate = now;
      }
      break;
      
    case MODE_OFF:
    default:
      // Red for off/error
      setNeoPixelColor(255, 0, 0); // Red
      break;
  }
}

void neoPixelBlink(uint8_t r, uint8_t g, uint8_t b) {
  static unsigned long lastBlink = 0;
  static bool blinkState = false;
  
  if (millis() - lastBlink >= BLINK_INTERVAL) {
    blinkState = !blinkState;
    if (blinkState) {
      setNeoPixelColor(r, g, b);
    } else {
      setNeoPixelColor(0, 0, 0); // Off
    }
    lastBlink = millis();
  }
}

void neoPixelPulse(uint8_t r, uint8_t g, uint8_t b) {
  static unsigned long lastPulse = 0;
  static uint8_t brightness = 0;
  static bool increasing = true;
  
  if (millis() - lastPulse >= 10) {
    if (increasing) {
      brightness += 2;
      if (brightness >= 255) increasing = false;
    } else {
      brightness -= 2;
      if (brightness <= 10) increasing = true;
    }
    
    // Apply brightness scaling
    uint8_t scaledR = (r * brightness) / 255;
    uint8_t scaledG = (g * brightness) / 255;
    uint8_t scaledB = (b * brightness) / 255;
    
    setNeoPixelColor(scaledR, scaledG, scaledB);
    lastPulse = millis();
  }
}
#endif
