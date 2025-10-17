#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "config.h"
#include "wifi_manager.h"
#include "ap_manager.h"
#include "led_controller.h"
#include "command_interface.h"
#include "iperf_manager.h"
#include "latency_analyzer.h"
#include "channel_analyzer.h"
#ifdef USE_WEBSERVER
#include "web_server.h"
#endif

// Global variables are defined in their respective modules

void setup() {
  // Initialize serial interface first
  initializeSerial();
  // Wait for serial to initialize (Feather boards only)
#ifdef ARDUINO_FEATHER_ESP32
  sleep(3); 
#endif

  // Initialize hardware
  initializeLED();
  
  // Initialize WiFi (will be configured by user commands)
  initializeWiFi();
  
  // Initialize iPerf manager
  initializeIperf();
  
  // Initialize latency analyzer
  initializeLatencyAnalysis();
  
  // Initialize channel analyzer
  initializeChannelAnalysis();
  
#ifdef USE_WEBSERVER
  // Initialize web server
  initializeWebServer();
#endif
  
  // Show initial prompt after all initialization is complete
  showInitialPrompt();
}

void loop() {
  // Handle serial commands
  handleSerialCommands();
  
  // Handle iPerf background tasks
  handleIperfTasks();
  
  // Handle latency test background tasks
  handleLatencyTasks();
  
  // Handle channel monitoring background tasks
  handleChannelMonitoringTasks();
  
#ifdef USE_WEBSERVER
  // Handle web server requests
  handleWebServerRequests();
  
  // Monitor and auto-restart web server based on WiFi state
  monitorWebServerState();
#endif
  
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
