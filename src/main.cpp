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

// RTOS Support (v4.1.0)
#ifdef USE_RTOS
#include "rtos_manager.h"
#include "command_task.h"
#include "wifi_task.h"
#ifdef USE_WEBSERVER
#include "web_task.h"
#endif
#include "analysis_task.h"
#include "led_task.h"
#endif

// Global variables are defined in their respective modules

void setup() {
  // Initialize serial interface first
  initializeSerial();

#ifdef USE_RTOS
  // Initialize RTOS infrastructure (v4.1.0)
  Serial.println("\n===========================================");
  Serial.println("ESP32 WiFi Utility v4.1.0 - RTOS Edition");
  Serial.println("===========================================\n");
  
  if (!initializeRTOS()) {
    Serial.println("FATAL ERROR: RTOS initialization failed!");
    Serial.println("System cannot continue. Please check serial output.");
    while (1) {
      delay(1000);
    }
  }
  
  // Print RTOS system information
  printRTOSStatistics();
  
  Serial.println("[RTOS] Starting legacy modules in compatibility mode...");
  Serial.println("[RTOS] NOTE: Full task migration will occur in future phases");
  Serial.println();
  
  // Initialize Command Task (Phase 2)
  Serial.println("[RTOS] Initializing Command Interface Task...");
  if (!initializeCommandTask()) {
    Serial.println("WARNING: Command Task initialization failed!");
    Serial.println("Falling back to legacy command handling.");
  } else {
    Serial.println("[RTOS] Command Task started successfully on Core 1");
  }
  
  // Initialize WiFi Task (Phase 3)
  Serial.println("[RTOS] Initializing WiFi Manager Task...");
  if (!initializeWiFiTask()) {
    Serial.println("WARNING: WiFi Task initialization failed!");
    Serial.println("Falling back to legacy WiFi handling.");
  } else {
    Serial.println("[RTOS] WiFi Task started successfully on Core 0 (WiFi Core)");
  }
  
#ifdef USE_WEBSERVER
  // Initialize Web Server Task (Phase 4)
  Serial.println("[RTOS] Initializing Web Server Task...");
  if (!initializeWebTask()) {
    Serial.println("WARNING: Web Server Task initialization failed!");
    Serial.println("Falling back to legacy web server handling.");
  } else {
    Serial.println("[RTOS] Web Server Task started successfully on Core 0 (WiFi Core)");
  }
#endif
  
  // Initialize Analysis Task (Phase 5)
  Serial.println("[RTOS] Initializing Analysis Task...");
  if (!initializeAnalysisTask()) {
    Serial.println("WARNING: Analysis Task initialization failed!");
    Serial.println("Falling back to legacy analysis handling.");
  } else {
    Serial.println("[RTOS] Analysis Task started successfully on Core 0 (WiFi Core)");
  }
  
  // Initialize LED Controller Task (Phase 6)
  Serial.println("[RTOS] Initializing LED Controller Task...");
  if (!initializeLEDTask()) {
    Serial.println("WARNING: LED Task initialization failed!");
    Serial.println("Falling back to legacy LED handling.");
  } else {
    Serial.println("[RTOS] LED Task started successfully on Core 1 (Application Core)");
  }
#else
  // Non-RTOS mode - initialize hardware directly
  initializeLED();
#endif

  // Initialize hardware (shared between RTOS and non-RTOS modes)
  
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
  
#ifndef USE_RTOS
  // Show initial prompt after all initialization is complete (legacy mode only)
  // In RTOS mode, CommandTask handles the prompt
  showInitialPrompt();
#endif
}

void loop() {
#ifndef USE_RTOS
  // Legacy mode: Handle serial commands directly in loop
  handleSerialCommands();
#else
  // RTOS mode: CommandTask handles serial commands asynchronously
  // Only need to handle background tasks here
#endif
  
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
