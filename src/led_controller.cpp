#include "led_controller.h"
#include "wifi_manager.h"

// ==========================================
// LED INITIALIZATION
// ==========================================
void initializeLED() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

// ==========================================
// LED CONTROL FUNCTIONS
// ==========================================
void blinkLED() {
  if (millis() - lastBlink >= BLINK_INTERVAL) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }
}

void updateLEDStatus() {
  // LED status indication based on current mode
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
}
