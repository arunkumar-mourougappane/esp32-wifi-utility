/**
 * @file led_controller.h
 * @brief LED and NeoPixel control interface declarations
 * 
 * This header defines functions for controlling status LEDs and NeoPixel
 * RGB LEDs on ESP32 boards. Supports basic LED blinking and advanced
 * NeoPixel effects including color control, pulsing, and status indication.
 * 
 * @author Arunkumar Mourougappane
 * @version 1.0.0
 * @date 2026-01-17
 */

#pragma once
#include <Arduino.h>
#include "config.h"

#ifdef USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif

// ==========================================
// LED CONTROL
// ==========================================

/**
 * @brief Initialize LED hardware
 * @details Configures LED pins and initializes NeoPixel if enabled
 */
void initializeLED();

/**
 * @brief Blink the LED once
 * @details Toggles LED state for visual feedback
 */
void blinkLED();

/**
 * @brief Update LED status based on current WiFi mode
 * @details Sets appropriate LED color/pattern for current system state
 */
void updateLEDStatus();

#ifdef USE_NEOPIXEL
// NeoPixel-specific functions

/**
 * @brief Set NeoPixel to specific RGB color
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set NeoPixel color based on WiFi mode
 * @param mode Current WiFi mode (Station, AP, etc.)
 * @param scanning Whether WiFi scan is in progress
 */
void setNeoPixelStatus(WiFiMode mode, bool scanning = false);

/**
 * @brief Blink NeoPixel with specified color
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void neoPixelBlink(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Pulse NeoPixel with smooth breathing effect
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void neoPixelPulse(uint8_t r, uint8_t g, uint8_t b);
#endif
