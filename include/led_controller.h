#pragma once
#include <Arduino.h>
#include "config.h"

#ifdef USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif

// ==========================================
// LED CONTROL
// ==========================================
void initializeLED();
void blinkLED();
void updateLEDStatus();

#ifdef USE_NEOPIXEL
// NeoPixel-specific functions
void setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b);
void setNeoPixelStatus(WiFiMode mode, bool scanning = false);
void neoPixelBlink(uint8_t r, uint8_t g, uint8_t b);
void neoPixelPulse(uint8_t r, uint8_t g, uint8_t b);
#endif
