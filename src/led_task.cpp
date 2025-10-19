#include "led_task.h"

#ifdef USE_RTOS

// ==========================================
// GLOBAL INSTANCE
// ==========================================
static LEDTask* ledTask = nullptr;

// ==========================================
// CONSTRUCTOR / DESTRUCTOR
// ==========================================

LEDTask::LEDTask() 
    : TaskBase("LED", 4096, TaskPriority::PRIORITY_LOW, 1),  // Core 1, Low priority
      currentState(LEDState::OFF),
      targetState(LEDState::OFF),
      currentPattern(LEDPattern::SOLID),
      currentColor(LEDColor::Black()),
      targetColor(LEDColor::Black()),
      currentBrightness(50),
      targetBrightness(50),
      currentPriority(0),
      animationSpeed(1000),
      lastUpdate(0),
      animationStartTime(0),
      pulsePhase(0),
      blinkState(false),
      inTransition(false),
      transitionProgress(0),
      stateQueue(nullptr)
#ifdef USE_NEOPIXEL
      , neoPixel(nullptr)
#endif
{
    // Queue created in setup()
}

LEDTask::~LEDTask() {
    if (stateQueue != nullptr) {
        vQueueDelete(stateQueue);
        stateQueue = nullptr;
    }
    
#ifdef USE_NEOPIXEL
    if (neoPixel != nullptr) {
        delete neoPixel;
        neoPixel = nullptr;
    }
#endif
}

// ==========================================
// TASK LIFECYCLE
// ==========================================

void LEDTask::setup() {
    Serial.println("[LEDTask] Setting up LED controller...");
    
    // Create state queue
    stateQueue = xQueueCreate(10, sizeof(LEDStateRequest));
    if (stateQueue == nullptr) {
        Serial.println("[LEDTask] ERROR: Failed to create state queue");
        return;
    }
    
#ifdef USE_NEOPIXEL
    // Initialize NeoPixel
    Serial.println("[LEDTask] Initializing NeoPixel...");
    neoPixel = new Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
    neoPixel->begin();
    neoPixel->setBrightness(NEOPIXEL_BRIGHTNESS);
    neoPixel->show();
    
    // Startup animation - Purple to Green
    applyColor(LEDColor::Purple(), 50);
    vTaskDelay(pdMS_TO_TICKS(300));
    applyColor(LEDColor::Green(), 50);
    vTaskDelay(pdMS_TO_TICKS(300));
    applyColor(LEDColor::Black(), 0);
    
    Serial.println("[LEDTask] NeoPixel initialized");
#else
    // Initialize standard LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.println("[LEDTask] Standard LED initialized");
#endif
    
    // Set initial state to IDLE
    currentState = LEDState::IDLE;
    currentColor = getDefaultColorForState(LEDState::IDLE);
    currentPattern = getDefaultPatternForState(LEDState::IDLE);
    animationSpeed = getDefaultSpeedForPattern(currentPattern);
    animationStartTime = millis();
    
    Serial.println("[LEDTask] Setup complete");
}

void LEDTask::loop() {
    // Process state queue
    processStateQueue();
    
    // Update current animation
    updateAnimation();
    
    // Run at ~60 FPS (16-17ms per frame)
    vTaskDelay(pdMS_TO_TICKS(16));
}

void LEDTask::cleanup() {
    Serial.println("[LEDTask] Cleaning up...");
    
    // Turn off LED
    applyColor(LEDColor::Black(), 0);
    
    if (stateQueue != nullptr) {
        vQueueDelete(stateQueue);
        stateQueue = nullptr;
    }
    
#ifdef USE_NEOPIXEL
    if (neoPixel != nullptr) {
        neoPixel->clear();
        neoPixel->show();
        delete neoPixel;
        neoPixel = nullptr;
    }
#endif
    
    Serial.println("[LEDTask] Cleanup complete");
}

// ==========================================
// PUBLIC API
// ==========================================

bool LEDTask::setState(LEDState state, uint8_t priority) {
    LEDStateRequest request;
    request.state = state;
    request.pattern = getDefaultPatternForState(state);
    request.color = getDefaultColorForState(state);
    request.brightness = currentBrightness;
    request.speed = getDefaultSpeedForPattern(request.pattern);
    request.priority = priority;
    request.useCustomColor = false;
    request.timestamp = millis();
    
    if (stateQueue == nullptr) {
        return false;
    }
    
    return xQueueSend(stateQueue, &request, 0) == pdTRUE;
}

bool LEDTask::setPattern(LEDPattern pattern, uint16_t speed, uint8_t priority) {
    LEDStateRequest request;
    request.state = currentState;
    request.pattern = pattern;
    request.color = currentColor;
    request.brightness = currentBrightness;
    request.speed = (speed == 0) ? getDefaultSpeedForPattern(pattern) : speed;
    request.priority = priority;
    request.useCustomColor = true;  // Keep current color
    request.timestamp = millis();
    
    if (stateQueue == nullptr) {
        return false;
    }
    
    return xQueueSend(stateQueue, &request, 0) == pdTRUE;
}

bool LEDTask::setColor(const LEDColor& color, LEDPattern pattern, uint8_t priority) {
    LEDStateRequest request;
    request.state = currentState;
    request.pattern = pattern;
    request.color = color;
    request.brightness = currentBrightness;
    request.speed = getDefaultSpeedForPattern(pattern);
    request.priority = priority;
    request.useCustomColor = true;
    request.timestamp = millis();
    
    if (stateQueue == nullptr) {
        return false;
    }
    
    return xQueueSend(stateQueue, &request, 0) == pdTRUE;
}

void LEDTask::setBrightness(uint8_t brightness) {
    if (brightness > 100) {
        brightness = 100;
    }
    targetBrightness = brightness;
}

// ==========================================
// STATE MANAGEMENT
// ==========================================

void LEDTask::processStateQueue() {
    LEDStateRequest request;
    
    // Process all pending requests
    while (xQueueReceive(stateQueue, &request, 0) == pdTRUE) {
        // Check priority - only accept if priority is >= current priority
        if (request.priority >= currentPriority) {
            transitionToState(request);
        }
    }
}

void LEDTask::transitionToState(const LEDStateRequest& request) {
    // Store new state
    targetState = request.state;
    targetColor = request.color;
    targetBrightness = request.brightness;
    currentPriority = request.priority;
    
    // If pattern is changing, start transition
    if (request.pattern != currentPattern || request.state != currentState) {
        currentPattern = request.pattern;
        currentState = request.state;
        animationSpeed = request.speed;
        animationStartTime = millis();
        pulsePhase = 0;
        blinkState = false;
        
        // Start fade transition if colors are different
        if (currentColor.r != targetColor.r || 
            currentColor.g != targetColor.g || 
            currentColor.b != targetColor.b) {
            inTransition = true;
            transitionProgress = 0;
        } else {
            currentColor = targetColor;
            inTransition = false;
        }
    }
}

// ==========================================
// ANIMATION UPDATES
// ==========================================

void LEDTask::updateAnimation() {
    unsigned long now = millis();
    
    // Handle fade transition first
    if (inTransition) {
        updateFade();
        return;
    }
    
    // Handle brightness changes
    if (currentBrightness != targetBrightness) {
        if (currentBrightness < targetBrightness) {
            currentBrightness++;
        } else {
            currentBrightness--;
        }
    }
    
    // Update animation based on current pattern
    switch (currentPattern) {
        case LEDPattern::SOLID:
            updateSolid();
            break;
        case LEDPattern::PULSE:
            updatePulse();
            break;
        case LEDPattern::BLINK:
            updateBlink();
            break;
        case LEDPattern::FLASH:
            updateFlash();
            break;
        case LEDPattern::FADE:
            updateFade();
            break;
    }
}

void LEDTask::updatePulse() {
    // Smooth pulse animation using sine wave
    unsigned long now = millis();
    unsigned long elapsed = now - animationStartTime;
    
    // Calculate pulse phase (0-255) based on animation speed
    // Full cycle = animationSpeed ms
    float cycle = (elapsed % animationSpeed) / (float)animationSpeed;
    pulsePhase = (uint8_t)(127.5f + 127.5f * sin(cycle * 2.0f * PI));
    
    // Apply pulsing brightness
    uint8_t pulseBrightness = (currentBrightness * pulsePhase) / 255;
    applyColor(currentColor, pulseBrightness);
    
    lastUpdate = now;
}

void LEDTask::updateBlink() {
    unsigned long now = millis();
    
    // Toggle blink state based on animation speed
    if (now - lastUpdate >= animationSpeed / 2) {
        blinkState = !blinkState;
        
        if (blinkState) {
            applyColor(currentColor, currentBrightness);
        } else {
            applyColor(LEDColor::Black(), 0);
        }
        
        lastUpdate = now;
    }
}

void LEDTask::updateFade() {
    // Smooth fade transition between colors
    if (!inTransition) {
        applyColor(targetColor, currentBrightness);
        currentColor = targetColor;
        return;
    }
    
    // Update transition progress (0-100)
    transitionProgress += 5;  // 5% per frame = ~330ms total transition at 60 FPS
    if (transitionProgress >= 100) {
        transitionProgress = 100;
        inTransition = false;
        currentColor = targetColor;
    }
    
    // Interpolate color
    LEDColor interpolated = interpolateColor(currentColor, targetColor, transitionProgress);
    applyColor(interpolated, currentBrightness);
    
    if (!inTransition) {
        currentColor = targetColor;
    }
}

void LEDTask::updateSolid() {
    // Just maintain solid color
    applyColor(currentColor, currentBrightness);
}

void LEDTask::updateFlash() {
    unsigned long now = millis();
    unsigned long elapsed = now - animationStartTime;
    
    // Flash sequence: 100ms on, 100ms off, 100ms on, then solid
    if (elapsed < 100) {
        applyColor(currentColor, currentBrightness);
    } else if (elapsed < 200) {
        applyColor(LEDColor::Black(), 0);
    } else if (elapsed < 300) {
        applyColor(currentColor, currentBrightness);
    } else {
        // After flash sequence, switch to solid
        currentPattern = LEDPattern::SOLID;
        applyColor(currentColor, currentBrightness);
    }
}

// ==========================================
// HELPER METHODS
// ==========================================

void LEDTask::applyColor(const LEDColor& color, uint8_t brightness) {
#ifdef USE_NEOPIXEL
    if (neoPixel != nullptr) {
        // Scale color by brightness (0-100)
        uint8_t r = (color.r * brightness) / 100;
        uint8_t g = (color.g * brightness) / 100;
        uint8_t b = (color.b * brightness) / 100;
        
        neoPixel->setPixelColor(0, neoPixel->Color(r, g, b));
        neoPixel->show();
    }
#else
    // Standard LED - simple on/off based on brightness
    if (brightness > 0) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
#endif
}

LEDColor LEDTask::getDefaultColorForState(LEDState state) {
    switch (state) {
        case LEDState::IDLE:        return LEDColor::Blue();
        case LEDState::SCANNING:    return LEDColor::Cyan();
        case LEDState::CONNECTING:  return LEDColor::Yellow();
        case LEDState::CONNECTED:   return LEDColor::Green();
        case LEDState::AP_MODE:     return LEDColor::Orange();
        case LEDState::ANALYZING:   return LEDColor::Purple();
        case LEDState::ERROR:       return LEDColor::Red();
        case LEDState::WARNING:     return LEDColor::Amber();
        case LEDState::SUCCESS:     return LEDColor::Green();
        case LEDState::OFF:
        default:                    return LEDColor::Black();
    }
}

LEDPattern LEDTask::getDefaultPatternForState(LEDState state) {
    switch (state) {
        case LEDState::IDLE:        return LEDPattern::PULSE;
        case LEDState::SCANNING:    return LEDPattern::BLINK;
        case LEDState::CONNECTING:  return LEDPattern::PULSE;
        case LEDState::CONNECTED:   return LEDPattern::SOLID;
        case LEDState::AP_MODE:     return LEDPattern::BLINK;
        case LEDState::ANALYZING:   return LEDPattern::PULSE;
        case LEDState::ERROR:       return LEDPattern::BLINK;
        case LEDState::WARNING:     return LEDPattern::PULSE;
        case LEDState::SUCCESS:     return LEDPattern::FLASH;
        case LEDState::OFF:
        default:                    return LEDPattern::SOLID;
    }
}

uint16_t LEDTask::getDefaultSpeedForPattern(LEDPattern pattern) {
    switch (pattern) {
        case LEDPattern::SOLID:     return 0;
        case LEDPattern::BLINK:     return 500;   // 500ms per blink cycle
        case LEDPattern::PULSE:     return 2000;  // 2s per pulse cycle
        case LEDPattern::FADE:      return 500;   // 500ms fade duration
        case LEDPattern::FLASH:     return 300;   // 300ms flash sequence
        default:                    return 1000;
    }
}

LEDColor LEDTask::interpolateColor(const LEDColor& from, const LEDColor& to, uint8_t progress) {
    if (progress >= 100) {
        return to;
    }
    
    uint8_t r = from.r + ((to.r - from.r) * progress) / 100;
    uint8_t g = from.g + ((to.g - from.g) * progress) / 100;
    uint8_t b = from.b + ((to.b - from.b) * progress) / 100;
    
    return LEDColor(r, g, b);
}

// ==========================================
// GLOBAL FUNCTIONS
// ==========================================

bool initializeLEDTask() {
    if (ledTask != nullptr) {
        Serial.println("[LEDTask] ERROR: Already initialized");
        return false;
    }
    
    ledTask = new LEDTask();
    if (ledTask == nullptr) {
        Serial.println("[LEDTask] ERROR: Failed to create task");
        return false;
    }
    
    if (!ledTask->start()) {
        Serial.println("[LEDTask] ERROR: Failed to start task");
        delete ledTask;
        ledTask = nullptr;
        return false;
    }
    
    Serial.println("[LEDTask] ✅ Initialized successfully");
    return true;
}

bool setLEDState(LEDState state, uint8_t priority) {
    if (ledTask == nullptr) {
        return false;
    }
    return ledTask->setState(state, priority);
}

bool setLEDPattern(LEDPattern pattern, uint16_t speed, uint8_t priority) {
    if (ledTask == nullptr) {
        return false;
    }
    return ledTask->setPattern(pattern, speed, priority);
}

bool setLEDColor(const LEDColor& color, LEDPattern pattern, uint8_t priority) {
    if (ledTask == nullptr) {
        return false;
    }
    return ledTask->setColor(color, pattern, priority);
}

void setLEDBrightness(uint8_t brightness) {
    if (ledTask != nullptr) {
        ledTask->setBrightness(brightness);
    }
}

LEDState getLEDState() {
    if (ledTask == nullptr) {
        return LEDState::OFF;
    }
    return ledTask->getCurrentState();
}

#endif // USE_RTOS
