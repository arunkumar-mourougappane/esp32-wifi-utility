#include "command_interface.h"
#include "wifi_manager.h"
#include "ap_manager.h"
#include "iperf_manager.h"
#include "config.h"

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
  
  // Print startup message
  Serial.println("\n==========================================");
  Serial.println("       ESP32 WiFi Scanner & AP");
  Serial.println("==========================================");
  Serial.println("🟡 Device in IDLE mode - Ready for commands");
  Serial.println("Type 'help' for available commands");
  Serial.println("==========================================\n");
  
  // Show initial prompt
  showPrompt();
}

// ==========================================
// SERIAL COMMAND HANDLING
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
  command.toLowerCase();
  promptShown = false; // Reset prompt flag
  
  if (command == "scan on") {
    if (currentMode == MODE_STATION) {
      scanningEnabled = true;
      Serial.println("✓ WiFi scanning ENABLED");
      lastScan = 0; // Force immediate scan
    } else {
      Serial.println("✗ Error: Must be in station mode to scan. Use 'mode station' first.");
    }
  }
  else if (command == "scan off") {
    scanningEnabled = false;
    Serial.println("✓ WiFi scanning DISABLED");
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
    startStationMode();
  }
  else if (command == "mode ap") {
    startAccessPoint();
  }
  else if (command.startsWith("mode ap ")) {
    String params = command.substring(8);
    params.trim();
    
    int spaceIndex = params.indexOf(' ');
    if (spaceIndex > 0) {
      String ssid = params.substring(0, spaceIndex);
      String password = params.substring(spaceIndex + 1);
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
      
      startAccessPoint(ssid, password);
    } else {
      Serial.println("✗ Error: Usage: mode ap <ssid> <password>");
      Serial.println("  Examples:");
      Serial.println("    mode ap MyHotspot MyPassword123");
      Serial.println("    mode ap \"My Hotspot\" \"My Password 123\"");
      Serial.println("  Note: SSID: 1-32 chars, Password: 8-63 chars");
    }
  }
  else if (command == "mode off") {
    stopWiFi();
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
  else if (command == "ap info" && currentMode == MODE_AP) {
    printAPInfo();
  }
  else if (command == "ap clients" && currentMode == MODE_AP) {
    listAPClients();
  }
  else if (command == "qr" && currentMode == MODE_AP) {
    generateAPQRCode(currentAPSSID, currentAPPassword, "WPA");
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
    // Parse SSID and password from command
    String params = command.substring(8);
    params.trim();
    
    int spaceIndex = params.indexOf(' ');
    if (spaceIndex > 0) {
      String ssid = params.substring(0, spaceIndex);
      String password = params.substring(spaceIndex + 1);
      ssid.trim();
      password.trim();
      connectToNetwork(ssid, password);
    } else {
      Serial.println("✗ Error: Usage: connect <ssid> <password>");
    }
  }
  else if (command == "disconnect" && currentMode == MODE_STATION) {
    disconnectFromNetwork();
  }
  else if (command.startsWith("iperf ")) {
    executeIperfCommand(command);
  }
  else if (command == "iperf") {
    printIperfHelp();
  }
  else if (command.length() > 0) {
    Serial.println("✗ Unknown command. Type 'help' for available commands.");
  }
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
  
  promptShown = false; // Will trigger new prompt display
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
  Serial.println("│ mode off        │ Disable WiFi completely              │");
  Serial.println("│ scan on         │ Start WiFi scanning (station mode)   │");
  Serial.println("│ scan off        │ Stop WiFi scanning                   │");
  Serial.println("│ scan now        │ Perform detailed scan immediately    │");
  Serial.println("│ scan info <id>  │ Show detailed info for network ID    │");
  Serial.println("│ connect <s> <p> │ Connect to network (station mode)    │");
  Serial.println("│ disconnect      │ Disconnect from network (station)    │");
  Serial.println("│ status          │ Show current status                  │");
  Serial.println("│ ap info         │ Show AP details (when in AP mode)    │");
  Serial.println("│ ap clients      │ List connected clients (AP mode)     │");
  Serial.println("│ qr              │ Show AP connection QR code (AP mode) │");
  Serial.println("│ deauth <id/mac> │ Disconnect by ID or MAC (AP mode)    │");
  Serial.println("│ deauth all      │ Disconnect all clients (AP mode)     │");
  Serial.println("│ iperf           │ Show iPerf performance test help     │");
  Serial.println("│ iperf status    │ Show current iPerf test status       │");
  Serial.println("│ clear           │ Clear console screen                 │");
  Serial.println("│ help            │ Show this help                       │");
  Serial.println("└─────────────────┴──────────────────────────────────────┘");
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
  
  if (currentMode == MODE_AP) {
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("Connected Clients: ");
    Serial.println(WiFi.softAPgetStationNum());
  }
  
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.println();
}
