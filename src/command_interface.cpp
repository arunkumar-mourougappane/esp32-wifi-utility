#include "command_interface.h"
#include "wifi_manager.h"
#include "ap_manager.h"
#include "iperf_manager.h"
#include "led_controller.h"
#include "latency_analyzer.h"
#include "channel_analyzer.h"
#include "config.h"
#include <esp_system.h>

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
  Serial.println("🟡 Device initialization starting...");
  Serial.println("==========================================\n");
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
  else if (command == "reset" || command == "restart") {
    executeResetCommand();
  }
  else if (command.length() > 0) {
    Serial.println("✗ Unknown command. Type 'help' for available commands.");
  }
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
  Serial.println("│ latency         │ Show latency & jitter test help      │");
  Serial.println("│ latency test    │ Start basic latency test             │");
  Serial.println("│ latency status  │ Show current latency test status     │");
  Serial.println("│ jitter          │ Quick jitter analysis                │");
  Serial.println("│ network analysis│ Comprehensive network analysis       │");
  Serial.println("│ channel         │ Show channel congestion help         │");
  Serial.println("│ channel scan    │ Analyze channel congestion           │");
  Serial.println("│ congestion      │ Quick channel congestion scan        │");
  Serial.println("│ spectrum        │ Full spectrum analysis               │");
  Serial.println("│ clear           │ Clear console screen                 │");
  Serial.println("│ reset           │ Restart the ESP32 device            │");
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
