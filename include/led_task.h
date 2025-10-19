#pragma once

#include <Arduino.h>
#include "config.h"

#ifdef USE_RTOS
#include "task_base.h"
#include "queue_manager.h"

#ifdef USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif

// ==========================================
// LED TASK
// ==========================================

/**
 * @file led_task.h
 * @brief LED Controller Task for smooth animations
 * 
 * Dedicated task for managing LED animations and visual feedback
 * without interfering with other operations. Runs on Core 1 (Application Core)
 * with low priority to ensure it doesn't impact critical tasks.
 * 
 * Features:
 * - Smooth 60 FPS animations
 * - Queue-based state changes
 * - Multiple animation patterns (PULSE, BLINK, FADE, SOLID)
 * - Fade transitions between states
 * - Priority-based pattern changes
 * - Support for both standard LED and NeoPixel
 * 
 * @version 4.1.0
 * @date 2025-10-18
 */

// ==========================================
// LED STATE AND PATTERN DEFINITIONS
// ==========================================

/**
 * @brief LED state representing system status
 */
enum class LEDState {
    IDLE,               ///< System idle - blue pulse
    SCANNING,           ///< WiFi scanning - cyan blink
    CONNECTING,         ///< WiFi connecting - yellow pulse
    CONNECTED,          ///< WiFi connected - green solid
    AP_MODE,            ///< AP mode active - orange blink
    ANALYZING,          ///< Analysis running - purple pulse
    ERROR,              ///< Error state - red blink
    WARNING,            ///< Warning state - amber pulse
    SUCCESS,            ///< Success indication - green flash
    OFF                 ///< LED off
};

/**
 * @brief LED animation pattern
 */
enum class LEDPattern {
    SOLID,              ///< Solid color, no animation
    BLINK,              ///< Blink on/off
    PULSE,              ///< Smooth pulse (fade in/out)
    FADE,               ///< Fade to new color
    FLASH               ///< Quick flash sequence
};

/**
 * @brief LED color structure
 */
struct LEDColor {
    uint8_t r;          ///< Red component (0-255)
    uint8_t g;          ///< Green component (0-255)
    uint8_t b;          ///< Blue component (0-255)
    
    LEDColor() : r(0), g(0), b(0) {}
    LEDColor(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    
    // Predefined colors
    static LEDColor Black()   { return LEDColor(0, 0, 0); }
    static LEDColor Red()     { return LEDColor(255, 0, 0); }
    static LEDColor Green()   { return LEDColor(0, 255, 0); }
    static LEDColor Blue()    { return LEDColor(0, 0, 255); }
    static LEDColor Yellow()  { return LEDColor(255, 255, 0); }
    static LEDColor Cyan()    { return LEDColor(0, 255, 255); }
    static LEDColor Magenta() { return LEDColor(255, 0, 255); }
    static LEDColor Orange()  { return LEDColor(255, 165, 0); }
    static LEDColor Purple()  { return LEDColor(128, 0, 128); }
    static LEDColor Amber()   { return LEDColor(255, 191, 0); }
    static LEDColor White()   { return LEDColor(255, 255, 255); }
};

/**
 * @brief LED state change request
 */
struct LEDStateRequest {
    LEDState state;             ///< Target LED state
    LEDPattern pattern;         ///< Animation pattern to use
    LEDColor color;             ///< Custom color (optional)
    uint8_t brightness;         ///< Brightness level (0-100)
    uint16_t speed;             ///< Animation speed in ms (pattern-dependent)
    uint8_t priority;           ///< Priority level (higher = more important)
    bool useCustomColor;        ///< Use custom color instead of default for state
    uint32_t timestamp;         ///< Request timestamp
};

// ==========================================
// LED TASK CLASS
// ==========================================

/**
 * @brief LED Controller Task
 * 
 * Manages LED animations and visual feedback in a dedicated task
 * to ensure smooth animations without blocking other operations.
 */
class LEDTask : public TaskBase {
public:
    /**
     * @brief Constructor
     * 
     * Creates the LED task with appropriate configuration for
     * Core 1 (Application Core) and low priority.
     */
    LEDTask();
    
    /**
     * @brief Destructor
     */
    ~LEDTask();
    
    /**
     * @brief Set LED state
     * 
     * @param state Target LED state
     * @param priority Request priority (0-255, higher = more important)
     * @return true if request was queued successfully
     */
    bool setState(LEDState state, uint8_t priority = 0);
    
    /**
     * @brief Set LED pattern
     * 
     * @param pattern Target animation pattern
     * @param speed Animation speed in ms
     * @param priority Request priority (0-255)
     * @return true if request was queued successfully
     */
    bool setPattern(LEDPattern pattern, uint16_t speed = 0, uint8_t priority = 0);
    
    /**
     * @brief Set custom LED color
     * 
     * @param color LED color
     * @param pattern Animation pattern
     * @param priority Request priority (0-255)
     * @return true if request was queued successfully
     */
    bool setColor(const LEDColor& color, LEDPattern pattern = LEDPattern::SOLID, uint8_t priority = 0);
    
    /**
     * @brief Set LED brightness
     * 
     * @param brightness Brightness level (0-100)
     */
    void setBrightness(uint8_t brightness);
    
    /**
     * @brief Get current LED state
     * 
     * @return Current LED state
     */
    LEDState getCurrentState() const { return currentState; }
    
    /**
     * @brief Get current animation pattern
     * 
     * @return Current pattern
     */
    LEDPattern getCurrentPattern() const { return currentPattern; }

protected:
    /**
     * @brief Task setup (called once on task start)
     */
    void setup() override;
    
    /**
     * @brief Task main loop (called repeatedly)
     */
    void loop() override;
    
    /**
     * @brief Task cleanup (called on task termination)
     */
    void cleanup() override;

private:
    // State management
    LEDState currentState;              ///< Current LED state
    LEDState targetState;               ///< Target LED state (for transitions)
    LEDPattern currentPattern;          ///< Current animation pattern
    LEDColor currentColor;              ///< Current LED color
    LEDColor targetColor;               ///< Target color (for fade transitions)
    uint8_t currentBrightness;          ///< Current brightness (0-100)
    uint8_t targetBrightness;           ///< Target brightness (for transitions)
    uint8_t currentPriority;            ///< Priority of current state
    uint16_t animationSpeed;            ///< Animation speed in ms
    
    // Animation state
    unsigned long lastUpdate;           ///< Last animation update timestamp
    unsigned long animationStartTime;   ///< Animation start timestamp
    uint8_t pulsePhase;                 ///< Pulse animation phase (0-255)
    bool blinkState;                    ///< Blink state (on/off)
    bool inTransition;                  ///< Currently transitioning between states
    uint8_t transitionProgress;         ///< Transition progress (0-100)
    
    // Queue
    QueueHandle_t stateQueue;           ///< LED state change queue
    
#ifdef USE_NEOPIXEL
    Adafruit_NeoPixel* neoPixel;        ///< NeoPixel instance
#endif
    
    // Private methods
    void processStateQueue();           ///< Process queued state changes
    void updateAnimation();             ///< Update current animation
    void updatePulse();                 ///< Update pulse animation
    void updateBlink();                 ///< Update blink animation
    void updateFade();                  ///< Update fade transition
    void updateSolid();                 ///< Update solid color
    void updateFlash();                 ///< Update flash sequence
    
    void applyColor(const LEDColor& color, uint8_t brightness);  ///< Apply color to LED
    void transitionToState(const LEDStateRequest& request);      ///< Transition to new state
    LEDColor getDefaultColorForState(LEDState state);            ///< Get default color for state
    LEDPattern getDefaultPatternForState(LEDState state);        ///< Get default pattern for state
    uint16_t getDefaultSpeedForPattern(LEDPattern pattern);      ///< Get default speed for pattern
    LEDColor interpolateColor(const LEDColor& from, const LEDColor& to, uint8_t progress);  ///< Interpolate between colors
};

// ==========================================
// GLOBAL ACCESS
// ==========================================

/**
 * @brief Initialize LED Task
 * 
 * Creates and starts the LED controller task.
 * Must be called after RTOS initialization.
 * 
 * @return true if initialization successful
 */
bool initializeLEDTask();

/**
 * @brief Set LED state (global function)
 * 
 * @param state Target LED state
 * @param priority Request priority (0-255)
 * @return true if request was queued successfully
 */
bool setLEDState(LEDState state, uint8_t priority = 0);

/**
 * @brief Set LED pattern (global function)
 * 
 * @param pattern Target animation pattern
 * @param speed Animation speed in ms (0 = default)
 * @param priority Request priority (0-255)
 * @return true if request was queued successfully
 */
bool setLEDPattern(LEDPattern pattern, uint16_t speed = 0, uint8_t priority = 0);

/**
 * @brief Set LED color (global function)
 * 
 * @param color LED color
 * @param pattern Animation pattern
 * @param priority Request priority (0-255)
 * @return true if request was queued successfully
 */
bool setLEDColor(const LEDColor& color, LEDPattern pattern = LEDPattern::SOLID, uint8_t priority = 0);

/**
 * @brief Set LED brightness (global function)
 * 
 * @param brightness Brightness level (0-100)
 */
void setLEDBrightness(uint8_t brightness);

/**
 * @brief Get current LED state (global function)
 * 
 * @return Current LED state
 */
LEDState getLEDState();

#endif // USE_RTOS
