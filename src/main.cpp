#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "config.h"
#include "wifi_manager.h"
#include "wifi_task.h"
#include "ap_manager.h"
#include "led_controller.h"
#include "command_interface.h"
#include "iperf_manager.h"
#include "latency_analyzer.h"
#include "channel_analyzer.h"
#include "signal_monitor.h"
#include "port_scanner.h"
#ifdef USE_WEBSERVER
#include "web_server.h"
#endif
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
#include "tft_display.h"
#endif

// Global variables are defined in their respective modules

void setup() {
  // Initialize serial interface first
  initializeSerial();

  // Initialize hardware
  initializeLED();
  
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
  // Initialize TFT display
  initializeTFT();
  displayWelcomeScreen();
  delay(2000);
#endif
  
  // Initialize WiFi (will be configured by user commands)
  initializeWiFi();
  
  // Initialize WiFi command task (FreeRTOS)
  if (!initWiFiTask()) {
    Serial.println("❌ Failed to initialize WiFi command task");
  }
  
  // Initialize iPerf manager
  initializeIperf();
  
  // Initialize latency analyzer
  initializeLatencyAnalysis();
  
  // Initialize channel analyzer
  initializeChannelAnalysis();
  
  // Initialize port scanner
  initializePortScanner();
  
#ifdef USE_WEBSERVER
  // Initialize web server
  initializeWebServer();
#endif
  
  // Show initial prompt after all initialization is complete
  showInitialPrompt();
}

void loop() {
  // Handle serial commands directly in loop
  handleSerialCommands();
  
  // Handle WiFi connection monitoring (non-blocking)
  handleWiFiConnection();
  
  // Handle iPerf background tasks
  handleIperfTasks();
  
  // Handle latency test background tasks
  handleLatencyTasks();
  
  // Handle channel monitoring background tasks
  handleChannelMonitoringTasks();
  
  // Handle signal monitoring background tasks
  updateSignalMonitoring();
  
  // Handle port scanner background tasks
  handlePortScanTasks();
  
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
