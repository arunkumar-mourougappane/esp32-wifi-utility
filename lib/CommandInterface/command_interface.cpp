#include "command_interface.h"
#include "wifi_manager.h"
#include "wifi_task.h"
#include "ap_manager.h"
#include "ap_config.h"
#include "station_config.h"
#include "iperf_manager.h"
#include "led_controller.h"
#include "latency_analyzer.h"
#include "channel_analyzer.h"
#include "signal_monitor.h"
#include "config.h"
#include <esp_system.h>
#ifdef USE_WEBSERVER
#include "web_server.h"
#endif

// ==========================================
// COMMAND INTERFACE VARIABLES
// ==========================================
String currentInput = "";
bool promptShown = false;
unsigned long lastActivity = 0;

// ==========================================
// SERIAL INITIALIZATION
// ==========================================
void initializeSerial() {
  Serial.begin(115200);
  delay(1000); // Wait for serial to initialize
  
  // Ensure proper line ending handling
  Serial.setTimeout(100);
  
  Serial.println("\n==========================================");
  Serial.println("       ESP32 WiFi Scanner & AP");
  Serial.println("==========================================");
  Serial.println("🟡 Device initialization starting...");
  Serial.println("==========================================\n");
  Serial.flush(); // Ensure all data is sent
}

// Show initial prompt after all initialization is complete
void showInitialPrompt() {
  Serial.println("\n==========================================");
  Serial.println("🟡 Device in IDLE mode - Ready for commands");
  Serial.println("Type 'help' for available commands");
  Serial.println("==========================================\n");
  
  // Show initial prompt
  showPrompt();
}

// ==========================================
// SERIAL COMMAND HANDLING (LEGACY MODE)
// ==========================================
void handleSerialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    processCharacter(c);
  }
  
  // Show prompt if needed and no recent activity
  if (!promptShown && (millis() - lastActivity > 100)) {
    showPrompt();
  }
}

void processCharacter(char c) {
  lastActivity = millis();
  
  if (c == '\n' || c == '\r') {
    // Command completed
    if (currentInput.length() > 0) {
      Serial.println(); // Move to new line
      executeCommand(currentInput);
      currentInput = "";
      showPrompt();
    }
  } else if (c == 127 || c == 8) { // Backspace or DEL
    if (currentInput.length() > 0) {
      currentInput = currentInput.substring(0, currentInput.length() - 1);
      Serial.print("\b \b"); // Erase character on terminal
    }
  } else if (c >= 32 && c <= 126) { // Printable characters
    currentInput += c;
    Serial.print(c); // Echo character
  }
}

// ==========================================
// COMMAND EXECUTION
// ==========================================
void executeCommand(String command) {
  command.trim();
  String originalCommand = command; // Preserve original case for SSID/password extraction
  command.toLowerCase();
  
  promptShown = false; // Reset prompt flag

  // Add command execution logging for debugging
  Serial.printf("Executing command: %s\n", originalCommand.c_str());
  Serial.flush(); // Ensure command echo is sent immediately
  
  if (command == "scan on") {
    if (currentMode == MODE_STATION) {
      scanningEnabled = true;
      Serial.println("✓ WiFi scanning ENABLED");
      lastScan = 0; // Force immediate scan
#ifdef USE_NEOPIXEL
      // Brief cyan flash to indicate scan enabled
      setNeoPixelColor(0, 255, 255);
      delay(300);
#endif
    } else {
      Serial.println("✗ Error: Must be in station mode to scan. Use 'mode station' first.");
#ifdef USE_NEOPIXEL
      // Brief red flash for error
      setNeoPixelColor(255, 0, 0);
      delay(500);
#endif
    }
  }
  else if (command == "scan off") {
    scanningEnabled = false;
    Serial.println("✓ WiFi scanning DISABLED");
#ifdef USE_NEOPIXEL
    // Brief white flash to indicate scan disabled
    setNeoPixelColor(255, 255, 255);
    delay(300);
#endif
  }
  else if (command == "scan now" && currentMode == MODE_STATION) {
    performWiFiScan(); // Immediate detailed scan
  }
  else if (command.startsWith("scan info ") && currentMode == MODE_STATION) {
    String networkId = command.substring(10);
    networkId.trim();
    showNetworkDetails(networkId.toInt());
  }
  else if (command == "mode station") {
    Serial.println("[CMD] Switching to station mode...");
    Serial.flush();
    startStationMode();
    Serial.println("[CMD] Station mode switch completed");
    Serial.flush();
  }
  else if (command == "mode ap") {
    Serial.println("[CMD] Switching to AP mode...");
    Serial.flush();
    startAccessPoint();
    Serial.println("[CMD] AP mode switch completed");
    Serial.flush();
  }
  else if (command.startsWith("mode ap ")) {
    String params = originalCommand.substring(8); // Use originalCommand to preserve case
    params.trim();
    
    int spaceIndex = params.indexOf(' ');
    if (spaceIndex > 0) {
      String ssid = params.substring(0, spaceIndex);
      String remaining = params.substring(spaceIndex + 1);
      remaining.trim();
      
      // Extract password and optional security
      String password;
      String securityStr = "wpa2"; // Default to WPA2
      int secondSpace = remaining.indexOf(' ');
      
      if (secondSpace < 0) {
        password = remaining;
      } else {
        password = remaining.substring(0, secondSpace);
        securityStr = remaining.substring(secondSpace + 1);
        securityStr.trim();
        securityStr.toLowerCase();
      }
      
      ssid.trim();
      password.trim();
      
      // Remove quotes if present
      if ((ssid.startsWith("\"") && ssid.endsWith("\"")) || 
          (ssid.startsWith("'") && ssid.endsWith("'"))) {
        ssid = ssid.substring(1, ssid.length() - 1);
      }
      if ((password.startsWith("\"") && password.endsWith("\"")) || 
          (password.startsWith("'") && password.endsWith("'"))) {
        password = password.substring(1, password.length() - 1);
      }
      
      // Parse security type
      APSecurityType security = AP_SEC_WPA2_PSK; // Default
      if (securityStr == "open") {
        security = AP_SEC_OPEN;
      } else if (securityStr == "wpa2") {
        security = AP_SEC_WPA2_PSK;
      } else if (securityStr == "wpa3") {
        security = AP_SEC_WPA3_PSK;
      } else if (securityStr == "mixed") {
        security = AP_SEC_WPA2_WPA3;
      } else {
        Serial.printf("⚠ Warning: Unknown security '%s', using WPA2\n", securityStr.c_str());
      }
      
      startAccessPoint(ssid, password, security);
      Serial.println("[CMD] Custom AP mode switch completed");
      Serial.flush();
    } else {
      Serial.println("✗ Error: Usage: mode ap <ssid> <password> [security]");
      Serial.println("  Examples:");
      Serial.println("    mode ap MyHotspot MyPassword123");
      Serial.println("    mode ap MyHotspot MyPassword123 wpa2");
      Serial.println("    mode ap OpenAP '' open");
      Serial.println("    mode ap \"My Hotspot\" \"My Password 123\" wpa3");
      Serial.println("  Security: open, wpa2, wpa3, mixed (default: wpa2)");
      Serial.println("  Note: SSID: 1-32 chars, Password: 8-63 chars for WPA2/WPA3");
    }
  }
  else if (command == "mode off") {
    requestStopWiFi();
  }
  else if (command == "mode idle") {
    setIdleMode();
  }
  else if (command == "status") {
    printStatus();
  }
  else if (command == "help") {
    printHelp();
  }
  else if (command == "clear") {
    clearConsole();
  }
  else if (command == "debug reset") {
    // Debug command to reset command task state
    promptShown = false;
    Serial.println("[DEBUG] Legacy mode prompt state reset");
  }
  else if (command == "debug tasks") {
    // Debug command to show RTOS task states
    Serial.println("[DEBUG] Not in RTOS mode");
  }
  else if (command == "ap info" && currentMode == MODE_AP) {
    printAPInfo();
  }
  else if (command == "ap clients" && currentMode == MODE_AP) {
    listAPClients();
  }
  else if (command == "qr" && currentMode == MODE_AP) {
    generateAPQRCode(currentAPSSID, currentAPPassword, "WPA");
  }
  else if (command.startsWith("ap save")) {
    // Parse optional parameters: ap save [ssid] [password] [channel] [autostart]
    String params = originalCommand.substring(7);
    params.trim();
    
    APConfig config;
    
    if (params.length() == 0) {
      // Save current AP configuration
      if (currentMode != MODE_AP) {
        Serial.println("✗ Error: Not in AP mode. Use 'ap save <ssid> <password> [channel] [autostart]'");
        return;
      }
      
      // Use current configuration
      strncpy(config.ssid, currentAPSSID.c_str(), sizeof(config.ssid) - 1);
      config.ssid[sizeof(config.ssid) - 1] = '\0';
      strncpy(config.password, currentAPPassword.c_str(), sizeof(config.password) - 1);
      config.password[sizeof(config.password) - 1] = '\0';
      config.channel = currentAPChannel;
      config.security = currentAPSecurity;
      config.autoStart = true;
      config.isValid = true;
      
      if (saveAPConfig(config)) {
        Serial.println("✓ Current AP configuration saved");
        Serial.println("  Device will auto-start in AP mode on next boot");
      } else {
        Serial.println("✗ Failed to save AP configuration");
      }
    } else {
      // Parse custom configuration
      int firstSpace = params.indexOf(' ');
      if (firstSpace < 0) {
        Serial.println("✗ Error: Usage: ap save <ssid> <password> [channel] [security] [autostart]");
        Serial.println("  Example: ap save MyNetwork MyPass123 6 wpa2 yes");
        return;
      }
      
      String ssid = params.substring(0, firstSpace);
      String remaining = params.substring(firstSpace + 1);
      remaining.trim();
      
      int secondSpace = remaining.indexOf(' ');
      String password;
      String channelStr;
      String securityStr = "wpa2"; // Default
      String autoStartStr = "yes";
      
      if (secondSpace < 0) {
        password = remaining;
      } else {
        password = remaining.substring(0, secondSpace);
        remaining = remaining.substring(secondSpace + 1);
        remaining.trim();
        
        int thirdSpace = remaining.indexOf(' ');
        if (thirdSpace < 0) {
          channelStr = remaining;
        } else {
          channelStr = remaining.substring(0, thirdSpace);
          remaining = remaining.substring(thirdSpace + 1);
          remaining.trim();
          
          int fourthSpace = remaining.indexOf(' ');
          if (fourthSpace < 0) {
            securityStr = remaining;
          } else {
            securityStr = remaining.substring(0, fourthSpace);
            autoStartStr = remaining.substring(fourthSpace + 1);
            autoStartStr.trim();
          }
        }
      }
      
      // Remove quotes if present
      if ((ssid.startsWith("\"") && ssid.endsWith("\"")) || 
          (ssid.startsWith("'") && ssid.endsWith("'"))) {
        ssid = ssid.substring(1, ssid.length() - 1);
      }
      if ((password.startsWith("\"") && password.endsWith("\"")) || 
          (password.startsWith("'") && password.endsWith("'"))) {
        password = password.substring(1, password.length() - 1);
      }
      
      // Parse security type
      securityStr.toLowerCase();
      APSecurityType security = AP_SEC_WPA2_PSK; // Default
      if (securityStr == "open") {
        security = AP_SEC_OPEN;
      } else if (securityStr == "wpa2") {
        security = AP_SEC_WPA2_PSK;
      } else if (securityStr == "wpa3") {
        security = AP_SEC_WPA3_PSK;
      } else if (securityStr == "mixed") {
        security = AP_SEC_WPA2_WPA3;
      }
      
      // Validate SSID
      if (ssid.length() == 0 || ssid.length() > 32) {
        Serial.println("✗ Error: SSID must be 1-32 characters");
        return;
      }
      
      // Validate password based on security type
      if (security == AP_SEC_OPEN) {
        // Open networks don't require passwords
        password = "";
      } else {
        // WPA2/WPA3 require 8-63 character passwords
        if (password.length() < 8 || password.length() > 63) {
          Serial.println("✗ Error: Password must be 8-63 characters for WPA2/WPA3");
          return;
        }
      }
      
      // Parse channel (default to 1 if not specified or invalid)
      uint8_t channel = 1;
      if (channelStr.length() > 0) {
        int ch = channelStr.toInt();
        if (ch >= 1 && ch <= 13) {
          channel = ch;
        } else {
          Serial.printf("⚠ Warning: Invalid channel %d, using default (1)\n", ch);
        }
      }
      
      // Parse autostart
      bool autoStart = (autoStartStr.equalsIgnoreCase("yes") || 
                       autoStartStr.equalsIgnoreCase("true") || 
                       autoStartStr == "1");
      
      // Create configuration
      strncpy(config.ssid, ssid.c_str(), sizeof(config.ssid) - 1);
      config.ssid[sizeof(config.ssid) - 1] = '\0';
      strncpy(config.password, password.c_str(), sizeof(config.password) - 1);
      config.password[sizeof(config.password) - 1] = '\0';
      config.channel = channel;
      config.security = security;
      config.autoStart = autoStart;
      config.isValid = true;
      
      if (saveAPConfig(config)) {
        Serial.println("✓ AP configuration saved");
        printAPConfig(config);
      } else {
        Serial.println("✗ Failed to save AP configuration");
      }
    }
  }
  else if (command == "ap load") {
    APConfig config;
    if (loadAPConfig(config)) {
      Serial.println("✓ Loaded saved AP configuration:");
      printAPConfig(config);
      
      // Apply configuration
      currentAPSSID = String(config.ssid);
      currentAPPassword = String(config.password);
      currentAPChannel = config.channel;
      currentAPSecurity = config.security;
      
      Serial.println("\nConfiguration loaded. Use 'mode ap' to start AP with saved settings.");
    } else {
      Serial.println("✗ No saved AP configuration found");
    }
  }
  else if (command == "ap show") {
    if (hasAPConfig()) {
      APConfig config;
      if (loadAPConfig(config)) {
        Serial.println("Saved AP configuration:");
        printAPConfig(config);
      }
    } else {
      Serial.println("No saved AP configuration found");
      Serial.println("Current default configuration:");
      APConfig defaultConfig;
      getDefaultAPConfig(defaultConfig);
      printAPConfig(defaultConfig);
    }
  }
  else if (command == "ap clear") {
    if (clearAPConfig()) {
      Serial.println("✓ AP configuration cleared");
      Serial.println("  Device will not auto-start AP on next boot");
      
      // Reset to defaults
      currentAPSSID = AP_SSID;
      currentAPPassword = AP_PASSWORD;
      currentAPChannel = 1;
    } else {
      Serial.println("✗ Failed to clear AP configuration");
    }
  }
  else if (command.startsWith("deauth ") && currentMode == MODE_AP) {
    String macAddress = command.substring(7);
    macAddress.trim();
    if (macAddress == "all") {
      deauthAllClients();
    } else {
      deauthClient(macAddress);
    }
  }
  else if (command.startsWith("connect ") && currentMode == MODE_STATION) {
    // Parse SSID and password from command (preserve case sensitivity)
    String params = originalCommand.substring(8); // Use originalCommand to preserve case
    params.trim();
    
    // Check heap before processing
    uint32_t heapBefore = ESP.getFreeHeap();
    Serial.printf("[HEAP] Free heap before connect: %u bytes\n", heapBefore);
    
    int spaceIndex = params.indexOf(' ');
    if (spaceIndex > 0) {
      String ssid = params.substring(0, spaceIndex);
      String remaining = params.substring(spaceIndex + 1);
      remaining.trim();
      
      // Extract password and optional security preference
      String password;
      String securityStr = "auto"; // Default
      int secondSpace = remaining.indexOf(' ');
      
      if (secondSpace < 0) {
        password = remaining;
      } else {
        password = remaining.substring(0, secondSpace);
        securityStr = remaining.substring(secondSpace + 1);
        securityStr.trim();
        securityStr.toLowerCase();
      }
      
      ssid.trim();
      password.trim();
      
      // Validate string lengths to prevent buffer overflows
      if (ssid.length() > 32) {
        Serial.printf("✗ Error: SSID too long (%d chars, max 32)\n", ssid.length());
        return;
      }
      if (password.length() > 63) {
        Serial.printf("✗ Error: Password too long (%d chars, max 63)\n", password.length());
        return;
      }
      
      // Parse security preference
      StationSecurityPreference secPref = STA_SEC_AUTO;
      if (securityStr == "auto") {
        secPref = STA_SEC_AUTO;
      } else if (securityStr == "wpa3prefer" || securityStr == "wpa3-prefer") {
        secPref = STA_SEC_WPA3_PREFER;
      } else if (securityStr == "wpa3only" || securityStr == "wpa3-only") {
        secPref = STA_SEC_WPA3_ONLY;
      } else if (securityStr == "wpa2min" || securityStr == "wpa2-min") {
        secPref = STA_SEC_WPA2_MIN;
      } else if (securityStr == "wpa2only" || securityStr == "wpa2-only") {
        secPref = STA_SEC_WPA2_ONLY;
      } else if (secondSpace >= 0) {
        Serial.printf("⚠ Warning: Unknown security preference '%s', using AUTO\n", securityStr.c_str());
      }
      
      connectToNetwork(ssid, password, secPref);
      
      // Check heap after processing
      uint32_t heapAfter = ESP.getFreeHeap();
      Serial.printf("[HEAP] Free heap after connect: %u bytes (diff: %d)\n", heapAfter, (int32_t)heapAfter - (int32_t)heapBefore);
    } else {
      Serial.println("✗ Error: Usage: connect <ssid> <password> [security]");
      Serial.println("  Security: auto, wpa3prefer, wpa3only, wpa2min, wpa2only");
    }
  }
  else if (command == "disconnect" && currentMode == MODE_STATION) {
    disconnectFromNetwork();
  }
  else if (command.startsWith("station save")) {
    // Parse optional parameters: station save [ssid] [password] [autoconnect]
    String params = originalCommand.substring(12);
    params.trim();
    
    StationConfig config;
    
    if (params.length() == 0) {
      // Save current station configuration
      if (currentMode != MODE_STATION || WiFi.status() != WL_CONNECTED) {
        Serial.println("✗ Error: Not connected to a network. Use 'station save <ssid> <password> [autoconnect]'");
        return;
      }
      
      // Use current connection
      strncpy(config.ssid, WiFi.SSID().c_str(), sizeof(config.ssid) - 1);
      config.ssid[sizeof(config.ssid) - 1] = '\0';
      strncpy(config.password, WiFi.psk().c_str(), sizeof(config.password) - 1);
      config.password[sizeof(config.password) - 1] = '\0';
      config.autoConnect = true;
      config.securityPreference = STA_SEC_AUTO; // Default for saved connections
      config.isValid = true;
      
      if (saveStationConfig(config)) {
        Serial.println("✓ Current station configuration saved");
        Serial.println("  Device will auto-connect to this network on next boot");
      } else {
        Serial.println("✗ Failed to save station configuration");
      }
    } else {
      // Parse custom configuration
      int firstSpace = params.indexOf(' ');
      if (firstSpace < 0) {
        Serial.println("✗ Error: Usage: station save <ssid> <password> [security] [autoconnect]");
        Serial.println("  Example: station save MyNetwork MyPass123 wpa2min yes");
        return;
      }
      
      String ssid = params.substring(0, firstSpace);
      String remaining = params.substring(firstSpace + 1);
      remaining.trim();
      
      int secondSpace = remaining.indexOf(' ');
      String password;
      String securityStr = "auto"; // Default
      String autoConnectStr = "yes";
      
      if (secondSpace < 0) {
        password = remaining;
      } else {
        password = remaining.substring(0, secondSpace);
        remaining = remaining.substring(secondSpace + 1);
        remaining.trim();
        
        int thirdSpace = remaining.indexOf(' ');
        if (thirdSpace < 0) {
          securityStr = remaining;
        } else {
          securityStr = remaining.substring(0, thirdSpace);
          autoConnectStr = remaining.substring(thirdSpace + 1);
          autoConnectStr.trim();
        }
      }
      
      // Remove quotes if present
      if ((ssid.startsWith("\"") && ssid.endsWith("\"")) || 
          (ssid.startsWith("'") && ssid.endsWith("'"))) {
        ssid = ssid.substring(1, ssid.length() - 1);
      }
      if ((password.startsWith("\"") && password.endsWith("\"")) || 
          (password.startsWith("'") && password.endsWith("'"))) {
        password = password.substring(1, password.length() - 1);
      }
      
      // Parse security preference
      securityStr.toLowerCase();
      StationSecurityPreference secPref = STA_SEC_AUTO;
      if (securityStr == "auto") {
        secPref = STA_SEC_AUTO;
      } else if (securityStr == "wpa3prefer" || securityStr == "wpa3-prefer") {
        secPref = STA_SEC_WPA3_PREFER;
      } else if (securityStr == "wpa3only" || securityStr == "wpa3-only") {
        secPref = STA_SEC_WPA3_ONLY;
      } else if (securityStr == "wpa2min" || securityStr == "wpa2-min") {
        secPref = STA_SEC_WPA2_MIN;
      } else if (securityStr == "wpa2only" || securityStr == "wpa2-only") {
        secPref = STA_SEC_WPA2_ONLY;
      }
      
      // Validate SSID
      if (ssid.length() == 0 || ssid.length() > 32) {
        Serial.println("✗ Error: SSID must be 1-32 characters");
        return;
      }
      
      // Password can be empty for open networks, but max 63 chars
      if (password.length() > 63) {
        Serial.println("✗ Error: Password must be 0-63 characters");
        return;
      }
      
      // Parse autoconnect
      bool autoConnect = (autoConnectStr.equalsIgnoreCase("yes") || 
                         autoConnectStr.equalsIgnoreCase("true") || 
                         autoConnectStr == "1");
      
      // Create configuration
      strncpy(config.ssid, ssid.c_str(), sizeof(config.ssid) - 1);
      config.ssid[sizeof(config.ssid) - 1] = '\0';
      strncpy(config.password, password.c_str(), sizeof(config.password) - 1);
      config.password[sizeof(config.password) - 1] = '\0';
      config.autoConnect = autoConnect;
      config.securityPreference = secPref;
      config.isValid = true;
      
      if (saveStationConfig(config)) {
        Serial.println("✓ Station configuration saved");
        printStationConfig(config);
      } else {
        Serial.println("✗ Failed to save station configuration");
      }
    }
  }
  else if (command == "station load") {
    StationConfig config;
    if (loadStationConfig(config)) {
      Serial.println("✓ Loaded saved station configuration:");
      printStationConfig(config);
      Serial.println("\nUse 'mode station' then 'connect <ssid> <password>' to connect,");
      Serial.println("or restart device to auto-connect.");
    } else {
      Serial.println("✗ No saved station configuration found");
    }
  }
  else if (command == "station show") {
    if (hasStationConfig()) {
      StationConfig config;
      if (loadStationConfig(config)) {
        Serial.println("Saved station configuration:");
        printStationConfig(config);
      }
    } else {
      Serial.println("No saved station configuration found");
    }
  }
  else if (command == "station clear") {
    if (clearStationConfig()) {
      Serial.println("✓ Station configuration cleared");
      Serial.println("  Device will not auto-connect on next boot");
    } else {
      Serial.println("✗ Failed to clear station configuration");
    }
  }
  else if (command == "station" || command == "station help") {
    printStationHelp();
  }
  else if (command.startsWith("iperf ")) {
    executeIperfCommand(command);
  }
  else if (command == "iperf") {
    printIperfHelp();
  }
  else if (command.startsWith("latency ")) {
    executeLatencyCommand(command);
  }
  else if (command == "latency") {
    printLatencyHelp();
  }
  else if (command == "jitter") {
    executeJitterAnalysis();
  }
  else if (command == "network analysis") {
    executeNetworkAnalysis("");
  }
  else if (command.startsWith("channel ")) {
    executeChannelCommand(command);
  }
  else if (command == "channel") {
    printChannelHelp();
  }
  else if (command == "congestion") {
    executeChannelCongestionScan();
  }
  else if (command == "spectrum") {
    executeSpectrumAnalysis();
  }
  else if (command.startsWith("signal ")) {
    executeSignalCommand(command);
  }
  else if (command == "signal") {
    printSignalHelp();
  }
  else if (command == "reset" || command == "restart") {
    executeResetCommand();
  }
#ifdef USE_WEBSERVER
  else if (command == "webserver start") {
    if (startWebServer()) {
      Serial.println("🌐 Web server started successfully");
      Serial.println("📡 Access at: " + getWebServerURL());
    }
  }
  else if (command == "webserver stop") {
    stopWebServer();
  }
  else if (command == "webserver status") {
    if (isWebServerRunning()) {
      Serial.println("✅ Web server is running");
      Serial.println("📡 URL: " + getWebServerURL());
    } else {
      Serial.println("❌ Web server is not running");
      Serial.println("💡 Use 'webserver start' to start it");
    }
  }
  else if (command == "webserver") {
    printWebServerHelp();
  }
#endif
  else if (command.length() > 0) {
    Serial.println("✗ Unknown command. Type 'help' for available commands.");
  }
  
  // Ensure command execution completion is logged and flushed
  Serial.flush(); // Force all output to be sent
}

// ==========================================
// SYSTEM CONTROL FUNCTIONS  
// ==========================================
void executeResetCommand() {
  Serial.println("\n⚠️  SYSTEM RESET INITIATED");
  Serial.println("==========================================");
  Serial.println("🔄 Preparing for board reset...");
  
  // Stop all active services gracefully
  Serial.println("🛑 Stopping active services...");
  
  // Stop WiFi operations
  if (currentMode == MODE_AP) {
    Serial.println("   - Stopping Access Point");
    WiFi.softAPdisconnect(true);
  }
  if (currentMode == MODE_STATION) {
    Serial.println("   - Disconnecting from WiFi");
    WiFi.disconnect(true);
  }
  
  // Stop iPerf if running
  Serial.println("   - Stopping iPerf services");
  shutdownIperf();
  
  // Stop latency analysis if running
  Serial.println("   - Stopping latency analysis");
  shutdownLatencyAnalysis();
  
  // Stop channel monitoring
  Serial.println("   - Stopping channel monitoring");
  channelMonitoringActive = false;
  
  // Turn off LEDs
#ifdef USE_NEOPIXEL
  Serial.println("   - Turning off NeoPixel");
  setNeoPixelColor(0, 0, 0); // Turn off
#else
  Serial.println("   - Turning off LED");
  digitalWrite(LED_PIN, LOW);
#endif

  Serial.println("✅ All services stopped");
  Serial.println("🔄 Restarting ESP32 in 2 seconds...");
  Serial.println("==========================================\n");
  
  // Flush serial output before reset
  Serial.flush();
  
  // Wait a moment for the message to be sent
  delay(2000);
  
  // Perform system reset
  esp_restart();
}

// ==========================================
// DISPLAY FUNCTIONS
// ==========================================
void showPrompt() {
  String modeIcon;
  switch (currentMode) {
    case MODE_IDLE:
      modeIcon = "🟡";
      break;
    case MODE_STATION:
      modeIcon = scanningEnabled ? "🔍" : "📡";
      break;
    case MODE_AP:
      modeIcon = "📡";
      break;
    case MODE_OFF:
      modeIcon = "🔴";
      break;
  }
  
  Serial.print(modeIcon + " ESP32> ");
  promptShown = true;
}

void clearConsole() {
  // Send ANSI escape codes to clear screen and move cursor to top-left
  Serial.print("\033[2J\033[H");
  
  // Print header after clearing
  Serial.println("==========================================");
  Serial.println("       ESP32 WiFi Scanner & AP");
  Serial.println("==========================================");
  
  // Show current mode status
  String modeText;
  switch (currentMode) {
    case MODE_IDLE:
      modeText = "🟡 IDLE mode - Ready for commands";
      break;
    case MODE_STATION:
      modeText = scanningEnabled ? "🔍 STATION mode - Scanning active" : "🔍 STATION mode - Ready to scan";
      break;
    case MODE_AP:
      modeText = "📡 ACCESS POINT mode - Hotspot active";
      break;
    case MODE_OFF:
      modeText = "🔴 WiFi DISABLED";
      break;
  }
  
  Serial.println(modeText);
  Serial.println("==========================================\n");
  
  promptShown = false; // Will trigger new prompt display (legacy mode)
}

void printHelp() {
  Serial.println("📋 AVAILABLE COMMANDS:");
  Serial.println("┌─────────────────┬──────────────────────────────────────┐");
  Serial.println("│ Command         │ Description                          │");
  Serial.println("├─────────────────┼──────────────────────────────────────┤");
  Serial.println("│ mode idle       │ Set device to idle/standby mode      │");
  Serial.println("│ mode station    │ Enable station mode for scanning     │");
  Serial.println("│ mode ap         │ Start as Access Point (default)      │");
  Serial.println("│ mode ap <s> <p> │ Start AP with custom SSID/password   │");
  Serial.println("│   [security]    │   Security: open, wpa2, wpa3, mixed  │");
  Serial.println("│ mode off        │ Disable WiFi completely              │");
  Serial.println("│ scan on         │ Start WiFi scanning (station mode)   │");
  Serial.println("│ scan off        │ Stop WiFi scanning                   │");
  Serial.println("│ scan now        │ Perform detailed scan immediately    │");
  Serial.println("│ scan info <id>  │ Show detailed info for network ID    │");
  Serial.println("│ connect <s> <p> │ Connect to network (station mode)    │");
  Serial.println("│   [security]    │   Security: auto, wpa3prefer, etc.   │");
  Serial.println("│ disconnect      │ Disconnect from network (station)    │");
  Serial.println("│ station save    │ Save current WiFi connection         │");
  Serial.println("│ station save .. │ Save custom network [sec] [auto]     │");
  Serial.println("│   <s> <p> [sec] │   Security: auto, wpa3prefer, etc.   │");
  Serial.println("│ station load    │ Load saved WiFi credentials          │");
  Serial.println("│ station show    │ Show saved station config            │");
  Serial.println("│ station clear   │ Clear saved WiFi credentials         │");
  Serial.println("│ station         │ Show station & security help         │");
  Serial.println("│ status          │ Show current status                  │");
  Serial.println("│ ap info         │ Show AP details (when in AP mode)    │");
  Serial.println("│ ap clients      │ List connected clients (AP mode)     │");
  Serial.println("│ ap save         │ Save current AP config (auto-start)  │");
  Serial.println("│ ap save <s> <p> │ Save AP config [ch] [sec] [auto]     │");
  Serial.println("│   [ch] [sec] .. │   Security: open, wpa2, wpa3, mixed  │");
  Serial.println("│ ap load         │ Load & apply saved AP config         │");
  Serial.println("│ ap show         │ Show saved or default AP config      │");
  Serial.println("│ ap clear        │ Clear saved AP config (no auto)      │");
  Serial.println("│ qr              │ Show AP connection QR code (AP mode) │");
  Serial.println("│ deauth <id/mac> │ Disconnect by ID or MAC (AP mode)    │");
  Serial.println("│ deauth all      │ Disconnect all clients (AP mode)     │");
  Serial.println("│ iperf           │ Show iPerf performance test help     │");
  Serial.println("│ iperf status    │ Show current iPerf test status       │");
  Serial.println("│ latency         │ Show latency & jitter test help      │");
  Serial.println("│ latency test    │ Start basic latency test             │");
  Serial.println("│ latency status  │ Show current latency test status     │");
  Serial.println("│ jitter          │ Quick jitter analysis                │");
  Serial.println("│ network analysis│ Comprehensive network analysis       │");
  Serial.println("│ channel         │ Show channel congestion help         │");
  Serial.println("│ channel scan    │ Analyze channel congestion           │");
  Serial.println("│ congestion      │ Quick channel congestion scan        │");
  Serial.println("│ spectrum        │ Full spectrum analysis               │");
  Serial.println("│ signal          │ Show signal strength help            │");
  Serial.println("│ signal show     │ Display current signal strength      │");
  Serial.println("│ signal scan     │ Scan all nearby networks             │");
  Serial.println("│ signal monitor  │ Start continuous signal monitoring   │");
#ifdef USE_WEBSERVER
  Serial.println("│ webserver       │ Show web server help                 │");
  Serial.println("│ webserver start │ Start web server on port 80          │");
  Serial.println("│ webserver stop  │ Stop web server                      │");
  Serial.println("│ webserver status│ Check web server status              │");
#endif
  Serial.println("│ clear           │ Clear console screen                 │");
  Serial.println("│ reset           │ Restart the ESP32 device             │");
  Serial.println("│ help            │ Show this help                       │");
  Serial.println("└─────────────────┴──────────────────────────────────────┘");
  Serial.println();
  Serial.println("💡 TIP: Use 'station', 'iperf', 'latency', 'channel', 'signal' for detailed help");
  Serial.println();
}

void printStationHelp() {
  Serial.println("📡 STATION MODE & SECURITY COMMANDS:");
  Serial.println("┌──────────────────────────────────────────────────────────────┐");
  Serial.println("│ CONNECTION COMMANDS                                          │");
  Serial.println("├──────────────────────────────────────────────────────────────┤");
  Serial.println("│ connect <ssid> <password>              Connect to network    │");
  Serial.println("│ connect <ssid> <password> [security]   Connect with security │");
  Serial.println("│ disconnect                             Disconnect from WiFi  │");
  Serial.println("├──────────────────────────────────────────────────────────────┤");
  Serial.println("│ CONFIGURATION MANAGEMENT                                     │");
  Serial.println("├──────────────────────────────────────────────────────────────┤");
  Serial.println("│ station save                           Save current network  │");
  Serial.println("│ station save <ssid> <password>         Save custom network   │");
  Serial.println("│   [security] [auto]                    with options          │");
  Serial.println("│ station load                           Load saved config     │");
  Serial.println("│ station show                           Show saved config     │");
  Serial.println("│ station clear                          Clear saved config    │");
  Serial.println("└──────────────────────────────────────────────────────────────┘");
  Serial.println();
  Serial.println("🔒 SECURITY OPTIONS:");
  Serial.println("┌──────────────┬───────────────────────────────────────────────┐");
  Serial.println("│ Option       │ Behavior                                      │");
  Serial.println("├──────────────┼───────────────────────────────────────────────┤");
  Serial.println("│ auto         │ Accept any security (default)                 │");
  Serial.println("│ wpa3prefer   │ Prefer WPA3, fallback to WPA2                 │");
  Serial.println("│ wpa3only     │ Require WPA3 (reject WPA2 and lower)          │");
  Serial.println("│ wpa2min      │ Minimum WPA2 (reject WEP/Open)                │");
  Serial.println("│ wpa2only     │ Require exactly WPA2                          │");
  Serial.println("└──────────────┴───────────────────────────────────────────────┘");
  Serial.println();
  Serial.println("📋 EXAMPLES:");
  Serial.println("  connect MyNetwork MyPass123");
  Serial.println("  connect MyNetwork MyPass123 auto");
  Serial.println("  connect SecureNet Pass456 wpa3prefer");
  Serial.println("  connect HighSec Strong789 wpa3only");
  Serial.println();
  Serial.println("  station save MyNetwork MyPass123 wpa2min yes");
  Serial.println("  station save SecureNet Pass456 wpa3prefer yes");
  Serial.println();
  Serial.println("💡 TIPS:");
  Serial.println("• WPA3 offers stronger security than WPA2");
  Serial.println("• Use wpa3prefer for best available security");
  Serial.println("• Use wpa3only when strict security is required");
  Serial.println("• Security preference persists across reboots");
  Serial.println("• Device will auto-connect with saved security preference");
  Serial.println();
}

void printStatus() {
  Serial.println("\n📊 CURRENT STATUS:");
  Serial.println("──────────────────");
  
  Serial.print("WiFi Mode: ");
  switch (currentMode) {
    case MODE_IDLE:
      Serial.println("🟡 Idle (Ready)");
      break;
    case MODE_STATION:
      Serial.println("🔍 Station (Scanner)");
      break;
    case MODE_AP:
      Serial.println("📡 Access Point");
      break;
    case MODE_OFF:
      Serial.println("🔴 Disabled");
      break;
  }
  
  Serial.print("Scanning: ");
  Serial.println(scanningEnabled ? "Enabled" : "Disabled");
  
  Serial.print("LED Status: ");
  if (currentMode == MODE_AP || scanningEnabled) {
    Serial.println("Fast Blinking");
  } else if (currentMode == MODE_IDLE) {
    Serial.println("Slow Pulse (every 2s)");
  } else {
    Serial.println("Off");
  }
  
  // Station Mode Information
  if (currentMode == MODE_STATION) {
    Serial.println("──────────────────");
    Serial.println("📶 Station Mode Status:");
    
    if (WiFi.status() == WL_CONNECTED) {
      // Connected to network
      Serial.println("Status: ✅ Connected");
      
      // Network name
      Serial.print("  SSID: ");
      Serial.println(WiFi.SSID());
      
      // IP Address Information
      Serial.print("  IP Address: ");
      Serial.println(WiFi.localIP());
      
      Serial.print("  Subnet Mask: ");
      Serial.println(WiFi.subnetMask());
      
      Serial.print("  Gateway: ");
      Serial.println(WiFi.gatewayIP());
      
      Serial.print("  DNS Server: ");
      Serial.println(WiFi.dnsIP());
      
      // MAC Address
      Serial.print("  MAC Address: ");
      Serial.println(WiFi.macAddress());
      
      // Signal Strength
      int32_t rssi = WiFi.RSSI();
      Serial.print("  Signal Strength: ");
      Serial.print(rssi);
      Serial.print(" dBm (");
      
      // Signal quality description
      if (rssi >= -50) {
        Serial.print("Excellent 🟢🟢🟢🟢");
      } else if (rssi >= -60) {
        Serial.print("Very Good 🟢🟢🟢");
      } else if (rssi >= -70) {
        Serial.print("Good 🟢🟢");
      } else if (rssi >= -80) {
        Serial.print("Fair 🟡");
      } else {
        Serial.print("Weak 🔴");
      }
      Serial.println(")");
      
      // Channel Information
      Serial.print("  Channel: ");
      Serial.println(WiFi.channel());
      
      // Connection uptime
      Serial.print("  Connection Time: ");
      unsigned long uptimeSeconds = millis() / 1000;
      unsigned long hours = uptimeSeconds / 3600;
      unsigned long minutes = (uptimeSeconds % 3600) / 60;
      unsigned long seconds = uptimeSeconds % 60;
      Serial.printf("%02lu:%02lu:%02lu\n", hours, minutes, seconds);
      
    } else {
      // Not connected
      Serial.println("Status: ❌ Not Connected");
      
      // Show WiFi status code
      wl_status_t status = WiFi.status();
      Serial.print("  Status Code: ");
      switch (status) {
        case WL_IDLE_STATUS:
          Serial.println("Idle");
          break;
        case WL_NO_SSID_AVAIL:
          Serial.println("SSID Not Available");
          break;
        case WL_SCAN_COMPLETED:
          Serial.println("Scan Completed");
          break;
        case WL_CONNECT_FAILED:
          Serial.println("Connection Failed");
          break;
        case WL_CONNECTION_LOST:
          Serial.println("Connection Lost");
          break;
        case WL_DISCONNECTED:
          Serial.println("Disconnected");
          break;
        default:
          Serial.println("Unknown");
          break;
      }
      
      Serial.println("  Use 'scan now' to find networks");
      Serial.println("  Use 'connect <SSID> <password>' to connect");
    }
  }
  
  // AP Mode Information
  if (currentMode == MODE_AP) {
    Serial.println("──────────────────");
    Serial.println("📡 Access Point Status:");
    Serial.print("  AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("  Connected Clients: ");
    Serial.println(WiFi.softAPgetStationNum());
  }
  
  Serial.println("──────────────────");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  
  // Add latency test status if active
  if (getLatencyTestState() != LATENCY_IDLE) {
    Serial.println("──────────────────");
    Serial.println(getLatencyStatus());
  }
  
  Serial.println();
}

// ==========================================
// LATENCY TEST COMMAND HANDLERS
// ==========================================
void executeLatencyCommand(String command) {
  String subCommand = command.substring(8);  // Remove "latency "
  subCommand.trim();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Not connected to WiFi. Connect to network first.");
    return;
  }
  
  if (subCommand == "test") {
    // Start basic UDP echo test
    LatencyConfig config = getDefaultLatencyConfig(LATENCY_UDP_ECHO);
    if (startLatencyTest(config)) {
      Serial.println("✅ Latency test started. Use 'latency status' to monitor progress.");
    }
  }
  else if (subCommand == "test tcp") {
    // Start TCP connect test
    LatencyConfig config = getDefaultLatencyConfig(LATENCY_TCP_CONNECT);
    if (startLatencyTest(config)) {
      Serial.println("✅ TCP latency test started. Use 'latency status' to monitor progress.");
    }
  }
  else if (subCommand == "test http") {
    // Start HTTP request test
    LatencyConfig config = getDefaultLatencyConfig(LATENCY_HTTP_REQUEST);
    if (startLatencyTest(config)) {
      Serial.println("✅ HTTP latency test started. Use 'latency status' to monitor progress.");
    }
  }
  else if (subCommand.startsWith("test ")) {
    // Custom test with host
    String host = subCommand.substring(5);
    host.trim();
    LatencyConfig config = getDefaultLatencyConfig(LATENCY_UDP_ECHO);
    config.target_host = host;
    if (startLatencyTest(config)) {
      Serial.println("✅ Custom latency test started for " + host);
    }
  }
  else if (subCommand == "stop") {
    stopLatencyTest();
  }
  else if (subCommand == "reset") {
    // Force reset latency state to IDLE
    shutdownLatencyAnalysis();
    initializeLatencyAnalysis();
    Serial.println("✅ Latency analyzer reset to idle state");
  }
  else if (subCommand == "status") {
    Serial.println(getLatencyStatus());
    if (getLatencyTestState() == LATENCY_COMPLETED) {
      printLatencyResults(getLastLatencyResults());
    }
  }
  else if (subCommand == "results") {
    if (getLatencyTestState() == LATENCY_COMPLETED) {
      printLatencyResults(getLastLatencyResults());
    } else {
      Serial.println("❌ No completed latency test results available.");
    }
  }
  else {
    printLatencyHelp();
  }
}

void executeJitterAnalysis() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Not connected to WiFi. Connect to network first.");
    return;
  }
  
  Serial.println("🔍 Starting quick jitter analysis...");
  
  // Start a short UDP echo test for jitter analysis
  LatencyConfig config = getDefaultLatencyConfig(LATENCY_UDP_ECHO);
  config.packet_count = 20;  // Quick test
  config.interval_ms = 100;  // 100ms interval
  
  if (startLatencyTest(config)) {
    Serial.println("✅ Jitter analysis started. Results will be displayed when complete.");
  }
}

void printLatencyHelp() {
  Serial.println("🎯 LATENCY & JITTER ANALYSIS COMMANDS:");
  Serial.println("┌──────────────────┬──────────────────────────────────────┐");
  Serial.println("│ Command          │ Description                          │");
  Serial.println("├──────────────────┼──────────────────────────────────────┤");
  Serial.println("│ latency test     │ Start basic UDP echo latency test    │");
  Serial.println("│ latency test tcp │ Start TCP connection latency test    │");
  Serial.println("│ latency test http│ Start HTTP request latency test      │");
  Serial.println("│ latency test <ip>│ Test latency to specific host/IP     │");
  Serial.println("│ latency stop     │ Stop current latency test            │");
  Serial.println("│ latency reset    │ Reset latency analyzer to idle       │");
  Serial.println("│ latency status   │ Show current test status             │");
  Serial.println("│ latency results  │ Show last test results               │");
  Serial.println("│ jitter           │ Quick jitter analysis (20 packets)   │");
  Serial.println("│ network analysis │ Comprehensive network quality test   │");
  Serial.println("└──────────────────┴──────────────────────────────────────┘");
  Serial.println();
  Serial.println("📊 Test Types:");
  Serial.println("• UDP Echo: Tests round-trip time via UDP packets");
  Serial.println("• TCP Connect: Measures TCP connection establishment time");
  Serial.println("• HTTP Request: Tests HTTP response time");
  Serial.println();
  Serial.println("📈 Metrics Measured:");
  Serial.println("• Latency: Round-trip time (min/max/average)");
  Serial.println("• Jitter: Variation in latency between packets");
  Serial.println("• Packet Loss: Percentage of lost packets");
  Serial.println("• Network Quality: Overall quality score (0-100)");
  Serial.println();
}

// ==========================================
// CHANNEL ANALYSIS COMMAND HANDLERS
// ==========================================
void executeChannelCommand(String command) {
  String subCommand = command.substring(8);  // Remove "channel "
  subCommand.trim();
  
  if (currentMode != MODE_STATION) {
    Serial.println("❌ Channel analysis requires station mode. Use 'mode station' first.");
    return;
  }
  
  if (subCommand == "scan") {
    Serial.println("🔍 Starting comprehensive channel congestion scan...");
    ChannelAnalysisResults results = performChannelCongestionScan(getDefaultChannelScanConfig());
    printChannelAnalysisResults(results);
    printChannelRecommendations(results);
  }
  else if (subCommand == "quick") {
    Serial.println("🔍 Performing quick channel scan...");
    ChannelAnalysisResults results = quickChannelScan();
    printChannelCongestionSummary(results);
  }
  else if (subCommand == "monitor start") {
    startChannelMonitoring(30); // 30 second interval
  }
  else if (subCommand == "monitor stop") {
    stopChannelMonitoring();
  }
  else if (subCommand == "monitor status") {
    Serial.println(getChannelMonitoringStatus());
  }
  else if (subCommand == "recommendations") {
    if (getLastChannelAnalysis().total_networks > 0) {
      printChannelRecommendations(getLastChannelAnalysis());
    } else {
      Serial.println("❌ No channel data available. Run 'channel scan' first.");
    }
  }
  else if (subCommand == "export") {
    if (getLastChannelAnalysis().total_networks > 0) {
      Serial.println("📊 Channel Analysis Data (JSON):");
      Serial.println(exportChannelAnalysisToJSON(getLastChannelAnalysis()));
    } else {
      Serial.println("❌ No channel data available. Run 'channel scan' first.");
    }
  }
  else if (subCommand == "report") {
    if (getLastChannelAnalysis().total_networks > 0) {
      Serial.println(generateChannelOptimizationReport(getLastChannelAnalysis()));
    } else {
      Serial.println("❌ No channel data available. Run 'channel scan' first.");
    }
  }
  else {
    printChannelHelp();
  }
}

void executeChannelCongestionScan() {
  if (currentMode != MODE_STATION) {
    Serial.println("❌ Channel analysis requires station mode. Use 'mode station' first.");
    return;
  }
  
  Serial.println("🔍 Quick channel congestion analysis...");
  ChannelAnalysisResults results = quickChannelScan();
  printChannelCongestionSummary(results);
}

void executeSpectrumAnalysis() {
  if (currentMode != MODE_STATION) {
    Serial.println("❌ Spectrum analysis requires station mode. Use 'mode station' first.");
    return;
  }
  
  Serial.println("🌐 Full spectrum analysis starting...");
  
  ChannelScanConfig config = getDefaultChannelScanConfig();
  config.detailed_analysis = true;
  config.scan_duration_ms = 5000; // Longer scan for detailed analysis
  
  ChannelAnalysisResults results = performChannelCongestionScan(config);
  printChannelAnalysisResults(results);
  printChannelRecommendations(results);
  
  Serial.println("\n" + generateChannelOptimizationReport(results));
}

void printChannelHelp() {
  Serial.println("📡 CHANNEL CONGESTION ANALYSIS COMMANDS:");
  Serial.println("┌─────────────────────┬──────────────────────────────────────┐");
  Serial.println("│ Command             │ Description                          │");
  Serial.println("├─────────────────────┼──────────────────────────────────────┤");
  Serial.println("│ channel scan        │ Comprehensive channel analysis       │");
  Serial.println("│ channel quick       │ Quick channel congestion check       │");
  Serial.println("│ channel monitor start│ Start continuous channel monitoring │");
  Serial.println("│ channel monitor stop│ Stop channel monitoring              │");
  Serial.println("│ channel recommendations│ Show channel recommendations      │");
  Serial.println("│ channel report      │ Generate optimization report         │");
  Serial.println("│ channel export      │ Export data in JSON format           │");
  Serial.println("│ congestion          │ Quick congestion summary             │");
  Serial.println("│ spectrum            │ Full spectrum analysis               │");
  Serial.println("└─────────────────────┴──────────────────────────────────────┘");
  Serial.println();
  Serial.println("📊 Analysis Features:");
  Serial.println("• Channel congestion scoring (0-100%)");
  Serial.println("• Network overlap detection");
  Serial.println("• Signal strength analysis");
  Serial.println("• Interference detection");
  Serial.println("• Optimal channel recommendations");
  Serial.println("• Continuous monitoring capability");
  Serial.println();
  Serial.println("💡 Usage Tips:");
  Serial.println("• Use 'congestion' for quick status check");
  Serial.println("• Use 'spectrum' for detailed analysis");
  Serial.println("• Monitor changes with 'channel monitor start'");
  Serial.println("• Consider recommendations when setting AP channel");
  Serial.println();
}

#ifdef USE_WEBSERVER
void printWebServerHelp() {
  Serial.println("\n🌐 WEB SERVER COMMANDS:");
  Serial.println("┌─────────────────────┬──────────────────────────────────────┐");
  Serial.println("│ Command             │ Description                          │");
  Serial.println("├─────────────────────┼──────────────────────────────────────┤");
  Serial.println("│ webserver start     │ Start the web server                 │");
  Serial.println("│ webserver stop      │ Stop the web server                  │");
  Serial.println("│ webserver status    │ Check server status & get URL        │");
  Serial.println("└─────────────────────┴──────────────────────────────────────┘");
  Serial.println();
  Serial.println("🌐 Web Interface Features:");
  Serial.println("• Real-time system status & statistics");
  Serial.println("• WiFi network scanning & display");
  Serial.println("• Channel analysis information");
  Serial.println("• Client connection monitoring (AP mode)");
  Serial.println("• Responsive mobile-friendly design");
  Serial.println();
  Serial.println("📋 Requirements:");
  Serial.println("• Must be in AP mode OR connected to WiFi network");
  Serial.println("• Web server runs on port 80 (default HTTP port)");
  Serial.println("• Access via browser at displayed IP address");
  Serial.println();
  Serial.println("💡 Usage Tips:");
  Serial.println("• In AP mode: Connect to AP, then access web interface");
  Serial.println("• In Station mode: Connect to WiFi first, then start server");
  Serial.println("• Use 'webserver status' to get the current access URL");
  Serial.println("• Web interface updates show real-time device status");
  Serial.println();
}
#endif

// ==========================================
// SIGNAL MONITORING COMMANDS
// ==========================================
void executeSignalCommand(String command) {
  if (command == "signal show" || command == "signal status") {
    SignalInfo info = getCurrentSignalStrength();
    printSignalInfo(info);
  }
  else if (command == "signal scan") {
    Serial.println("Scanning nearby networks for signal strength...");
    std::vector<SignalInfo> networks = getNearbySignalStrengths(20);
    
    if (networks.empty()) {
      Serial.println("No networks found.");
      return;
    }
    
    Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println("Nearby Networks Signal Strength");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println();
    
    for (size_t i = 0; i < networks.size(); i++) {
      Serial.printf("%2d. %-32s %4d dBm  %3d%%  %s%s\n",
                    i + 1,
                    networks[i].ssid.c_str(),
                    networks[i].rssi,
                    networks[i].quality,
                    networks[i].qualityText.c_str(),
                    networks[i].isConnected ? " [CONNECTED]" : "");
      
      // Show mini signal meter
      Serial.print("    ");
      displaySignalMeter(networks[i].rssi);
    }
    
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  }
  else if (command.startsWith("signal monitor ")) {
    String arg = command.substring(15);
    arg.trim();
    
    if (arg == "start") {
      uint8_t interval = 5; // default 5 seconds
      startSignalMonitoring(interval);
    } else if (arg == "stop") {
      stopSignalMonitoring();
    } else {
      Serial.println("Invalid argument. Use: signal monitor start|stop");
    }
  }
  else if (command == "signal monitor") {
    startSignalMonitoring(5); // Default 5 second interval
  }
  else {
    printSignalHelp();
  }
}

void printSignalHelp() {
  Serial.println("\n📶 SIGNAL STRENGTH COMMANDS:");
  Serial.println("┌─────────────────────┬──────────────────────────────────────┐");
  Serial.println("│ Command             │ Description                          │");
  Serial.println("├─────────────────────┼──────────────────────────────────────┤");
  Serial.println("│ signal show         │ Display current signal strength      │");
  Serial.println("│ signal scan         │ Scan & show all nearby networks      │");
  Serial.println("│ signal monitor      │ Start continuous monitoring          │");
  Serial.println("│ signal monitor start│ Start signal strength monitoring     │");
  Serial.println("│ signal monitor stop │ Stop monitoring                      │");
  Serial.println("└─────────────────────┴──────────────────────────────────────┘");
  Serial.println();
  Serial.println("📊 Signal Strength Metrics:");
  Serial.println("• RSSI (Received Signal Strength Indicator) in dBm");
  Serial.println("• Quality percentage (0-100%)");
  Serial.println("• Quality rating (Excellent, Good, Fair, Weak, Very Weak)");
  Serial.println("• Visual signal strength meter");
  Serial.println();
  Serial.println("📈 RSSI Reference:");
  Serial.println("• -30 to -50 dBm : Excellent (100%)");
  Serial.println("• -50 to -60 dBm : Good (80-100%)");
  Serial.println("• -60 to -70 dBm : Fair (60-80%)");
  Serial.println("• -70 to -80 dBm : Weak (40-60%)");
  Serial.println("• -80 to -90 dBm : Very Weak (20-40%)");
  Serial.println("• Below -90 dBm  : Unusable (<20%)");
  Serial.println();
  Serial.println("💡 Usage Tips:");
  Serial.println("• Use 'signal show' for current connection status");
  Serial.println("• Use 'signal scan' to compare nearby network strengths");
  Serial.println("• Use 'signal monitor' for real-time updates");
  Serial.println("• Press any key to stop continuous monitoring");
  Serial.println("• Access web interface at /signal for graphical view");
  Serial.println();
}

