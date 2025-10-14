#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "config.h"
#include "wifi_manager.h"
#include "ap_manager.h"
#include "led_controller.h"
#include "command_interface.h"

// Global variables are defined in their respective modules

void setup() {
  // Initialize hardware
  initializeLED();
  
  // Initialize serial interface
  initializeSerial();
  
  // Initialize WiFi (will be configured by user commands)
  initializeWiFi();
}

void loop() {
  // Handle serial commands
  handleSerialCommands();
  
  // WiFi scanning logic (only in station mode)
  if (scanningEnabled && currentMode == MODE_STATION && (millis() - lastScan >= SCAN_INTERVAL)) {
    performWiFiScan();
    lastScan = millis();
  }
  
  // Update AP client list periodically (only in AP mode)
  static unsigned long lastClientUpdate = 0;
  if (currentMode == MODE_AP && (millis() - lastClientUpdate >= 5000)) { // Update every 5 seconds
    updateClientList();
    lastClientUpdate = millis();
  }
  
  // Update LED status based on current mode
  updateLEDStatus();
  
  delay(100); // Small delay to prevent excessive loop iterations
}
