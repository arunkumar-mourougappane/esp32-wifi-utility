#include "wifi_manager.h"
#include "command_interface.h"
#include "led_controller.h"
#include "ap_config.h"
#include "station_config.h"
#include "logging.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>
#include <qrcode.h>
#include "esp_wifi.h"
#ifdef USE_NEOPIXEL
#include "web_server.h"
#endif
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
#include "tft_display.h"
#endif

// Helper macro for prompt handling
#define RESET_PROMPT() promptShown = false

// ==========================================
// GLOBAL STATE VARIABLES
// ==========================================
WiFiMode currentMode = MODE_IDLE;
bool scanningEnabled = false;
unsigned long lastScan = 0;
unsigned long lastBlink = 0;
bool ledState = false;

// Connection state tracking for non-blocking operation
static bool isConnecting = false;
static unsigned long connectionStartTime = 0;
static String connectingSSID = "";
static String connectingPassword = "";
static int connectionAttempts = 0;

// ==========================================
// ACCESS POINT CONFIGURATION VARIABLES
// ==========================================
String currentAPSSID = AP_SSID;
String currentAPPassword = AP_PASSWORD;
uint8_t currentAPChannel = 1;

// ==========================================
// WIFI INITIALIZATION
// ==========================================
void initializeWiFi() {
  // Initialize AP configuration system
  initAPConfig();
  
  // Initialize Station configuration system
  initStationConfig();
  
  // Check for saved AP configuration and auto-start if configured
  APConfig savedAPConfig;
  if (loadAPConfig(savedAPConfig) && savedAPConfig.autoStart) {
    LOG_INFO(TAG_WIFI, "Saved AP configuration found - auto-starting...");
    
    // Apply saved configuration
    currentAPSSID = String(savedAPConfig.ssid);
    currentAPPassword = String(savedAPConfig.password);
    currentAPChannel = savedAPConfig.channel;
    
    // Start AP with saved configuration
    startAccessPoint();
    return;
  }
  
  // Check for saved Station configuration and auto-connect if configured
  StationConfig savedStaConfig;
  if (loadStationConfig(savedStaConfig) && savedStaConfig.autoConnect) {
    LOG_INFO(TAG_WIFI, "Saved Station configuration found - auto-connecting...");
    
    // Start station mode and connect
    WiFi.mode(WIFI_STA);
    delay(100);
    currentMode = MODE_STATION;
    
    // Attempt connection
    connectToNetwork(String(savedStaConfig.ssid), String(savedStaConfig.password));
  }
}

// ==========================================
// MODE MANAGEMENT
// ==========================================
void startStationMode() {
  stopWiFi();
  
  delay(100);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  delay(100);
  
  currentMode = MODE_STATION;
  scanningEnabled = false;
  
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
  // Show Station mode status on TFT
  sendTFTStatus("Station Mode\nReady");
#endif
  
  LOG_INFO(TAG_WIFI, "Station mode activated - Ready to scan for networks");
  LOG_DEBUG(TAG_WIFI, "Use 'scan on' to start scanning");
  LOG_DEBUG(TAG_WIFI, "Web server will auto-start upon WiFi connection");
  Serial.flush(); // Ensure output is sent immediately
  
  RESET_PROMPT();
}

void startAccessPoint() {
  stopWiFi();
  
  delay(100);
  
  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP(currentAPSSID.c_str(), currentAPPassword.c_str(), currentAPChannel);
  
  if (apStarted) {
    currentMode = MODE_AP;
    scanningEnabled = false;
    
    LOG_INFO(TAG_AP, "Access Point mode activated");
    LOG_INFO(TAG_AP, "SSID: %s", currentAPSSID.c_str());
    LOG_DEBUG(TAG_AP, "Password: %s", currentAPPassword.c_str());
    LOG_INFO(TAG_AP, "Channel: %d", currentAPChannel);
    LOG_INFO(TAG_AP, "IP Address: %s", WiFi.softAPIP().toString().c_str());
    LOG_DEBUG(TAG_AP, "Use 'ap info' for detailed information");
    
    // Generate and display QR code for easy mobile connection
    generateAPQRCode(currentAPSSID, currentAPPassword, "WPA");
    
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
    // Send AP info to TFT display task via queue
    wifi_sta_list_t stationList;
    uint8_t clientCount = 0;
    if (esp_wifi_ap_get_sta_list(&stationList) == ESP_OK) {
      clientCount = stationList.num;
    }
    sendTFTAPUpdate(currentAPSSID.c_str(), currentAPPassword.c_str(), 
                    WiFi.softAPIP().toString().c_str(), clientCount);
#endif
    
    // Web server will be auto-started by monitorWebServerState()
    LOG_DEBUG(TAG_WEB, "Web server will auto-start momentarily");
    Serial.flush(); // Ensure output is sent immediately
    
    RESET_PROMPT();
  } else {
    currentMode = MODE_OFF;
    LOG_ERROR(TAG_AP, "Failed to start Access Point");
    Serial.flush(); // Ensure error message is sent
    RESET_PROMPT();
  }
}

void startAccessPoint(const String& ssid, const String& password) {
  // Validate SSID and password
  if (ssid.length() == 0 || ssid.length() > 32) {
    LOG_ERROR(TAG_AP, "SSID must be 1-32 characters long");
    return;
  }
  
  if (password.length() < 8 || password.length() > 63) {
    LOG_ERROR(TAG_AP, "Password must be 8-63 characters long for WPA2");
    return;
  }
  
  // Update current AP configuration
  currentAPSSID = ssid;
  currentAPPassword = password;
  
  // Start AP with new configuration
  stopWiFi();
  
  delay(100);
  
  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP(currentAPSSID.c_str(), currentAPPassword.c_str(), currentAPChannel);
  
  if (apStarted) {
    currentMode = MODE_AP;
    scanningEnabled = false;
    
    LOG_INFO(TAG_AP, "Custom Access Point mode activated");
    LOG_INFO(TAG_AP, "SSID: %s", currentAPSSID.c_str());
    LOG_DEBUG(TAG_AP, "Password: %s", currentAPPassword.c_str());
    LOG_INFO(TAG_AP, "IP Address: %s", WiFi.softAPIP().toString().c_str());
    LOG_DEBUG(TAG_AP, "Use 'ap info' for detailed information");
    
    // Generate and display QR code for easy mobile connection
    generateAPQRCode(currentAPSSID, currentAPPassword, "WPA");
    
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
    // Send AP info to TFT display task via queue
    wifi_sta_list_t stationList;
    uint8_t clientCount = 0;
    if (esp_wifi_ap_get_sta_list(&stationList) == ESP_OK) {
      clientCount = stationList.num;
    }
    sendTFTAPUpdate(currentAPSSID.c_str(), currentAPPassword.c_str(), 
                    WiFi.softAPIP().toString().c_str(), clientCount);
#endif
    
    // Web server will be auto-started by monitorWebServerState()
    LOG_DEBUG(TAG_WEB, "Web server will auto-start momentarily");
    
    RESET_PROMPT();
  } else {
    currentMode = MODE_OFF;
    LOG_ERROR(TAG_AP, "Failed to start custom Access Point");
    LOG_WARN(TAG_AP, "Check SSID and password requirements");
    
    // Restore default configuration on failure
    currentAPSSID = AP_SSID;
    currentAPPassword = AP_PASSWORD;
    
    RESET_PROMPT();
  }
}

void stopWiFi() {
#ifdef USE_NEOPIXEL
  // Stop web server before disabling WiFi
  if (isWebServerRunning()) {
    LOG_INFO(TAG_WEB, "Stopping web server...");
    stopWebServer();
  }
#endif

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
  // Show WiFi disabled screen when stopping WiFi
  if (currentMode == MODE_AP || currentMode == MODE_STATION) {
    sendTFTDisabled();
  }
#endif
   
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  
  currentMode = MODE_OFF;
  scanningEnabled = false;
  digitalWrite(LED_PIN, LOW);
  
  LOG_INFO(TAG_WIFI, "WiFi disabled");
}

void setIdleMode() {
#ifdef USE_NEOPIXEL
  // Stop web server before setting idle mode
  if (isWebServerRunning()) {
    LOG_INFO(TAG_WEB, "Stopping web server...");
    stopWebServer();
  }
#endif
  
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  
  currentMode = MODE_IDLE;
  scanningEnabled = false;
  digitalWrite(LED_PIN, LOW);
  
  LOG_INFO(TAG_WIFI, "Device set to IDLE mode - Ready for commands");
  LOG_DEBUG(TAG_WIFI, "Use 'mode station' or 'mode ap' to activate WiFi");
}

// ==========================================
// WIFI SCANNING
// ==========================================
void performWiFiScan() {
  Serial.println("\n🔍 === WiFi Network Scanner === 🔍");
  Serial.println("Scanning for available networks...");
  
  // Start comprehensive scan
  int networkCount = WiFi.scanNetworks(false, true); // async=false, show_hidden=true
  
  if (networkCount == 0) {
    Serial.println("❌ No networks found");
    Serial.println("Try moving closer to WiFi access points or check antenna connection");
  } else {
    Serial.printf("✅ Discovered %d networks:\n\n", networkCount);
    
    // Print detailed header
    Serial.println("╔════╤═══════════════════════════╤══════╤════╤══════════════════╤═════════╤═══════════════════╗");
    Serial.println("║ ID │         SSID              │ RSSI │ CH │    Encryption    │ Quality │      BSSID        ║");
    Serial.println("╠════╪═══════════════════════════╪══════╪════╪══════════════════╪═════════╪═══════════════════╣");
    
    for (int i = 0; i < networkCount; ++i) {
      // Get network details
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      uint8_t channel = WiFi.channel(i);
      wifi_auth_mode_t encryptionType = WiFi.encryptionType(i);
      uint8_t* bssid = WiFi.BSSID(i);
      
      // Handle empty/hidden SSIDs
      if (ssid.length() == 0) {
        ssid = "<Hidden Network>";
      }
      
      // Truncate long SSIDs
      if (ssid.length() > 25) {
        ssid = ssid.substring(0, 22) + "...";
      }
      
      // Calculate signal quality percentage
      int quality = 0;
      if (rssi >= -50) quality = 100;
      else if (rssi >= -60) quality = 80;
      else if (rssi >= -70) quality = 60;
      else if (rssi >= -80) quality = 40;
      else if (rssi >= -90) quality = 20;
      else quality = 0;
      
      // Print network row
      Serial.printf("║%3d │ %-25s │%5d │%3d │", i + 1, ssid.c_str(), rssi, channel);
      
      // Print encryption type with icon
      String encStr = "";
      switch (encryptionType) {
        case WIFI_AUTH_OPEN:
          encStr = "🔓 Open         ";
          break;
        case WIFI_AUTH_WEP:
          encStr = "🔒 WEP          ";
          break;
        case WIFI_AUTH_WPA_PSK:
          encStr = "🔒 WPA          ";
          break;
        case WIFI_AUTH_WPA2_PSK:
          encStr = "🔒 WPA2         ";
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          encStr = "🔒 WPA/WPA2     ";
          break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          encStr = "🏢 WPA2-Ent     ";
          break;
        case WIFI_AUTH_WPA3_PSK:
          encStr = "🔐 WPA3         ";
          break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
          encStr = "🔐 WPA2/WPA3    ";
          break;
        case WIFI_AUTH_WAPI_PSK:
          encStr = "🔒 WAPI         ";
          break;
        default:
          encStr = "❓ Unknown      ";
      }
      Serial.printf(" %-18s │", encStr.c_str());
      
      // Print signal quality with visual indicator
      String qualityStr = "";
      if (quality >= 80) qualityStr = "🟢";
      else if (quality >= 60) qualityStr = "🟡";
      else if (quality >= 40) qualityStr = "🟠";
      else qualityStr = "🔴";
      
      Serial.printf(" %s %3d%% │", qualityStr.c_str(), quality);
      
      // Print BSSID (MAC address)
      if (bssid) {
        Serial.printf(" %02X:%02X:%02X:%02X:%02X:%02X ║\n", 
                     bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
      } else {
        Serial.printf(" ??:??:??:??:??:?? ║\n");
      }
    }
    
    Serial.println("╚════╧═══════════════════════════╧══════╧════╧══════════════════╧═════════╧═══════════════════╝");
    
    // Print summary statistics
    Serial.println("\n📈 Network Summary:");
    
    // Count networks by encryption type
    int openNetworks = 0, wepNetworks = 0, wpaNetworks = 0, wpa2Networks = 0, wpa3Networks = 0;
    int strongSignals = 0, weakSignals = 0;
    
    for (int i = 0; i < networkCount; ++i) {
      wifi_auth_mode_t encType = WiFi.encryptionType(i);
      int32_t rssi = WiFi.RSSI(i);
      
      switch (encType) {
        case WIFI_AUTH_OPEN: openNetworks++; break;
        case WIFI_AUTH_WEP: wepNetworks++; break;
        case WIFI_AUTH_WPA_PSK: wpaNetworks++; break;
        case WIFI_AUTH_WPA2_PSK:
        case WIFI_AUTH_WPA_WPA2_PSK:
        case WIFI_AUTH_WPA2_ENTERPRISE: wpa2Networks++; break;
        case WIFI_AUTH_WPA3_PSK:
        case WIFI_AUTH_WPA2_WPA3_PSK: wpa3Networks++; break;
      }
      
      if (rssi > -60) strongSignals++;
      else if (rssi < -80) weakSignals++;
    }
    
    Serial.printf("├─ 🔓 Open: %d  🔒 WEP: %d  🔒 WPA2: %d  🔐 WPA3: %d\n", 
                  openNetworks, wepNetworks, wpa2Networks, wpa3Networks);
    Serial.printf("├─ 📶 Strong signals (>-60dBm): %d\n", strongSignals);
    Serial.printf("├─ 📱 Weak signals (<-80dBm): %d\n", weakSignals);
    
    // Channel usage analysis
    uint8_t channelCount[14] = {0}; // Channels 1-13 (14 is special)
    for (int i = 0; i < networkCount; ++i) {
      uint8_t ch = WiFi.channel(i);
      if (ch >= 1 && ch <= 13) {
        channelCount[ch]++;
      }
    }
    
    // Find most congested channels
    uint8_t maxChannel = 1, maxCount = channelCount[1];
    for (int ch = 1; ch <= 13; ch++) {
      if (channelCount[ch] > maxCount) {
        maxChannel = ch;
        maxCount = channelCount[ch];
      }
    }
    
    Serial.printf("├─ 📡 Most congested channel: %d (%d networks)\n", maxChannel, maxCount);
    Serial.printf("└─ 💡 Recommended channels for AP: 1, 6, 11 (least interference)\n");
    
    Serial.println("\n🔗 To connect: use 'connect <SSID> <password>'");
    Serial.println("=== End of WiFi Scan ===\n");

  }
  
  // Clean up
  WiFi.scanDelete();
  RESET_PROMPT(); // Show prompt after scan results
}

void showNetworkDetails(int networkId) {
  Serial.println("\n📡 === Detailed Network Information === 📡");
  
  // Perform a fresh scan to get current data
  int networkCount = WiFi.scanNetworks(false, true);
  
  if (networkCount == 0) {
    Serial.println("❌ No networks found. Run 'scan now' first.");
    return;
  }
  
  // Convert to 0-based index
  int index = networkId - 1;
  
  if (index < 0 || index >= networkCount) {
    Serial.printf("❌ Invalid network ID. Valid range: 1-%d\n", networkCount);
    Serial.println("💡 Use 'scan now' to see available networks");
    return;
  }
  
  // Get detailed network information
  String ssid = WiFi.SSID(index);
  int32_t rssi = WiFi.RSSI(index);
  uint8_t channel = WiFi.channel(index);
  wifi_auth_mode_t encryptionType = WiFi.encryptionType(index);
  uint8_t* bssid = WiFi.BSSID(index);
  
  // Handle hidden networks
  if (ssid.length() == 0) {
    ssid = "<Hidden Network>";
  }
  
  Serial.println("┌─────────────────────────────────────────────────────────┐");
  Serial.printf("│ 🏷️  Network Name: %-38s │\n", ssid.c_str());
  Serial.println("├─────────────────────────────────────────────────────────┤");
  
  // BSSID (MAC Address)
  if (bssid) {
    char bssidStr[18]; // XX:XX:XX:XX:XX:XX format
    sprintf(bssidStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    Serial.printf("│ 🔗 BSSID (MAC):  %-38s │\n", bssidStr);
  } else {
    Serial.printf("│ 🔗 BSSID (MAC):  %-38s │\n", "Unknown");
  }
  
  // Signal Information
  Serial.printf("│ 📶 Signal (RSSI): %-37s │\n", String(rssi) + " dBm");
  
  // Signal quality calculation and description
  int quality = 0;
  String qualityDesc = "";
  String signalIcon = "";
  
  if (rssi >= -30) {
    quality = 100; qualityDesc = "Excellent (Very Close)"; signalIcon = "🟢🟢🟢🟢";
  } else if (rssi >= -50) {
    quality = 90; qualityDesc = "Excellent"; signalIcon = "🟢🟢🟢🟢";
  } else if (rssi >= -60) {
    quality = 80; qualityDesc = "Very Good"; signalIcon = "🟢🟢🟢⚪";
  } else if (rssi >= -67) {
    quality = 70; qualityDesc = "Good"; signalIcon = "🟢🟢🟡⚪";
  } else if (rssi >= -70) {
    quality = 60; qualityDesc = "Fair"; signalIcon = "🟢🟡🟡⚪";
  } else if (rssi >= -80) {
    quality = 50; qualityDesc = "Weak"; signalIcon = "🟡🟡🔴⚪";
  } else if (rssi >= -90) {
    quality = 30; qualityDesc = "Very Weak"; signalIcon = "🟡🔴🔴⚪";
  } else {
    quality = 10; qualityDesc = "Extremely Weak"; signalIcon = "🔴🔴🔴⚪";
  }
  
  // Create quality string with proper formatting
  String qualityString = String(quality) + "% (" + qualityDesc + ") " + signalIcon;
  Serial.printf("│ 📊 Signal Quality: %-42s │\n", qualityString.c_str());
  
  // Channel Information
  String bandInfo = "";
  if (channel >= 1 && channel <= 13) {
    bandInfo = "2.4GHz";
  } else if (channel >= 36 && channel <= 165) {
    bandInfo = "5GHz";
  } else {
    bandInfo = "Unknown";
  }
  
  String channelString = String(channel) + " (" + bandInfo + ")";
  Serial.printf("│ 📻 Channel:      %-38s │\n", channelString.c_str());
  
  // Channel congestion analysis
  int channelUsage = 0;
  for (int i = 0; i < networkCount; i++) {
    if (WiFi.channel(i) == channel) {
      channelUsage++;
    }
  }
  
  String congestionLevel = "";
  if (channelUsage == 1) congestionLevel = "Clear";
  else if (channelUsage <= 3) congestionLevel = "Light";
  else if (channelUsage <= 6) congestionLevel = "Moderate";
  else if (channelUsage <= 10) congestionLevel = "Heavy";
  else congestionLevel = "Severe";
  
  String congestionString = congestionLevel + " (" + String(channelUsage) + " networks on this channel)";
  Serial.printf("│ 🚦 Congestion:   %-42s │\n", congestionString.c_str());
  
  // Security Information
  String encIcon = "";
  String encDescription = "";
  String securityLevel = "";
  
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      encIcon = "🔓"; encDescription = "Open (No Security)"; securityLevel = "❌ None";
      break;
    case WIFI_AUTH_WEP:
      encIcon = "🔒"; encDescription = "WEP (Deprecated)"; securityLevel = "🟡 Weak";
      break;
    case WIFI_AUTH_WPA_PSK:
      encIcon = "🔒"; encDescription = "WPA Personal"; securityLevel = "🟠 Moderate";
      break;
    case WIFI_AUTH_WPA2_PSK:
      encIcon = "🔒"; encDescription = "WPA2 Personal"; securityLevel = "🟢 Good";
      break;
    case WIFI_AUTH_WPA_WPA2_PSK:
      encIcon = "🔒"; encDescription = "WPA/WPA2 Mixed"; securityLevel = "🟢 Good";
      break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      encIcon = "🏢"; encDescription = "WPA2 Enterprise"; securityLevel = "🟢 Excellent";
      break;
    case WIFI_AUTH_WPA3_PSK:
      encIcon = "🔐"; encDescription = "WPA3 Personal"; securityLevel = "🟢 Excellent";
      break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
      encIcon = "🔐"; encDescription = "WPA2/WPA3 Mixed"; securityLevel = "🟢 Excellent";
      break;
    case WIFI_AUTH_WAPI_PSK:
      encIcon = "🔒"; encDescription = "WAPI"; securityLevel = "🟢 Good";
      break;
    default:
      encIcon = "❓"; encDescription = "Unknown"; securityLevel = "❓ Unknown";
  }
  
  String securityString = encIcon + " " + encDescription;
  Serial.printf("│ 🔐 Security:     %-40s │\n", securityString.c_str());
  Serial.printf("│ 🛡️  Security Level: %-37s │\n", securityLevel.c_str());
  
  // Connection recommendations
  Serial.println("├─────────────────────────────────────────────────────────┤");
  Serial.println("│ 💡 Connection Analysis:                              │");
  
  if (encryptionType == WIFI_AUTH_OPEN) {
    Serial.println("│ ⚠️  WARNING: Open network - no encryption!           │");
    Serial.println("│    Your data will be transmitted unencrypted.        │");
  } else if (encryptionType == WIFI_AUTH_WEP) {
    Serial.println("│ ⚠️  WARNING: WEP is severely outdated and insecure!  │");
    Serial.println("│    Avoid connecting to this network if possible.     │");
  }
  
  if (rssi < -80) {
    Serial.println("│ 📶 Signal is weak - connection may be unstable       │");
    Serial.println("│    Consider moving closer to the access point.       │");
  } else if (rssi > -50) {
    Serial.println("│ 📶 Excellent signal strength for stable connection   │");
  }
  
  if (channelUsage > 6) {
    Serial.println("│ 🚦 Channel is congested - may affect performance     │");
  }
  
  // Estimated data rates based on signal strength and channel
  String estimatedSpeed = "";
  if (rssi > -50 && bandInfo == "5GHz") {
    estimatedSpeed = "100-300 Mbps (802.11ac)";
  } else if (rssi > -60 && bandInfo == "5GHz") {
    estimatedSpeed = "50-100 Mbps (802.11n/ac)";
  } else if (rssi > -50 && bandInfo == "2.4GHz") {
    estimatedSpeed = "50-150 Mbps (802.11n)";
  } else if (rssi > -70) {
    estimatedSpeed = "10-50 Mbps";
  } else {
    estimatedSpeed = "1-10 Mbps (Basic connectivity)";
  }
  
  Serial.printf("│ Est. Speed:   %-40s │\n", estimatedSpeed.c_str());
  
  Serial.println("└─────────────────────────────────────────────────────────┘");
  
  // Connection command suggestion
  if (encryptionType != WIFI_AUTH_OPEN) {
    Serial.printf("💡 To connect: connect \"%s\" <password>\n", ssid.c_str());
  } else {
    Serial.printf("💡 To connect: connect \"%s\" \"\"\n", ssid.c_str());
  }
  
  Serial.println();
  
  // Cleanup
  WiFi.scanDelete();
  RESET_PROMPT();
}

// ==========================================
// NETWORK CONNECTION
// ==========================================

/**
 * @brief Connects to a WiFi network using SSID and password (non-blocking)
 * 
 * This function initiates a connection to a specific WiFi network when
 * the ESP32 is in station mode. The connection process is asynchronous -
 * this function starts the connection and returns immediately. Call
 * handleWiFiConnection() periodically to monitor connection progress.
 * 
 * @param ssid Network name to connect to
 * @param password Network password for authentication
 * 
 * @pre Device must be in station mode (MODE_STATION)
 * @pre SSID must be available and in range
 * @pre Password must be correct for the target network
 * 
 * @note Connection timeout is set to 10 seconds
 * @note This is a non-blocking call - returns immediately after initiating connection
 * @note Use handleWiFiConnection() in main loop to monitor progress
 * 
 * @return void Outputs connection start message to Serial
 */
void connectToNetwork(String ssid, String password) {
  if (currentMode != MODE_STATION) {
    LOG_ERROR(TAG_WIFI, "Must be in station mode to connect. Use 'mode station' first");
    return;
  }
  
  // Cancel any existing connection attempt
  if (isConnecting) {
    Serial.println("⚠️  Canceling previous connection attempt");
    isConnecting = false;
  }
  
  LOG_INFO(TAG_WIFI, "Connecting to '%s'...", ssid.c_str());
  
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
  // Show connecting animation on TFT
  sendTFTConnecting();
#endif
  
#ifdef USE_NEOPIXEL
  // Show yellow while connecting
  setNeoPixelColor(255, 255, 0);
#endif
  
  // Start connection asynchronously
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // Track connection state
  isConnecting = true;
  connectionStartTime = millis();
  connectingSSID = ssid;
  connectingPassword = password;
  connectionAttempts = 0;
  
  Serial.println("  Connection initiated (non-blocking)");
  Serial.println("  Monitoring connection progress...");
}

/**
 * @brief Monitors and handles WiFi connection progress (non-blocking)
 * 
 * This function should be called periodically from the main loop to monitor
 * the progress of an ongoing WiFi connection attempt. It provides visual
 * feedback and handles connection success/failure without blocking.
 * 
 * @note Call this from main loop when isConnecting is true
 * @note Automatically times out after 10 seconds
 * @note Provides visual feedback during connection attempts
 * 
 * @return void Updates connection state and outputs progress to Serial
 */
void handleWiFiConnection() {
  if (!isConnecting) {
    return; // Nothing to do
  }
  
  wl_status_t status = WiFi.status();
  
  // Check if connected successfully
  if (status == WL_CONNECTED) {
    Serial.println();
    LOG_INFO(TAG_WIFI, "Connected to '%s'", WiFi.SSID().c_str());
    LOG_INFO(TAG_WIFI, "IP Address: %s", WiFi.localIP().toString().c_str());
    LOG_DEBUG(TAG_WIFI, "Gateway: %s", WiFi.gatewayIP().toString().c_str());
    LOG_DEBUG(TAG_WIFI, "DNS: %s", WiFi.dnsIP().toString().c_str());
    
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
    // Send station mode info to TFT display task via queue (with password for QR code)
    sendTFTStationUpdate(connectingSSID.c_str(), connectingPassword.c_str(), 
                        WiFi.localIP().toString().c_str(), WiFi.RSSI());
#endif
    
#ifdef USE_NEOPIXEL
    // Show solid green for successful connection
    setNeoPixelColor(0, 255, 0);
#endif

#ifdef USE_WEB_SERVER 
    // Automatically start web server after successful connection
    if (!isWebServerRunning()) {
      LOG_INFO(TAG_WEB, "Starting web server...");
      if (startWebServer()) {
        LOG_INFO(TAG_WEB, "Web server ready at: %s", getWebServerURL().c_str());
      }
    }
#endif
    
    // Reset connection state
    isConnecting = false;
    RESET_PROMPT();
    return;
  }
  
  // Check for timeout (10 seconds)
  unsigned long elapsed = millis() - connectionStartTime;
  if (elapsed > 10000) {
    Serial.println();
    LOG_ERROR(TAG_WIFI, "Failed to connect to '%s'", WiFi.SSID().c_str());
    LOG_WARN(TAG_WIFI, "Check SSID, password, and signal strength");
    
#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT)
    // Show connection failed status on TFT
    sendTFTStatus("Connection\nFailed");
#endif
    
#ifdef USE_NEOPIXEL
    // Show red for connection failure
    setNeoPixelColor(255, 0, 0);
#endif
    
    // Reset connection state
    isConnecting = false;
    WiFi.disconnect();
    RESET_PROMPT();
    return;
  }
  
  // Provide periodic visual feedback (every 100ms)
  if (elapsed / 100 > connectionAttempts) {
    Serial.print(".");
    connectionAttempts = elapsed / 100;
    
#ifdef USE_NEOPIXEL
    // Blink yellow during connection attempts
    if (connectionAttempts % 2 == 0) {
      setNeoPixelColor(255, 255, 0); // Yellow
    } else {
      setNeoPixelColor(100, 100, 0); // Dim yellow
    }
#endif
  }
}

/**
 * @brief Disconnects from current WiFi network
 * 
 * Disconnects from the currently connected WiFi network while maintaining
 * station mode for future connections.
 */
void disconnectFromNetwork() {
  if (currentMode != MODE_STATION) {
    LOG_ERROR(TAG_WIFI, "Must be in station mode");
    return;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    String ssid = WiFi.SSID();
    
#ifdef USE_NEOPIXEL
    // Stop web server before disconnecting
    if (isWebServerRunning()) {
      LOG_INFO(TAG_WEB, "Stopping web server...");
      stopWebServer();
    }
#endif
    
    WiFi.disconnect();
    LOG_INFO(TAG_WIFI, "Disconnected from '%s'", ssid.c_str());
  } else {
    LOG_INFO(TAG_WIFI, "Not connected to any network");
  }
  
  RESET_PROMPT();
}



// ==========================================
// QR CODE FUNCTIONALITY
// ==========================================

/**
 * @brief Generates and displays a QR code for AP connection information
 * 
 * This function creates a WiFi QR code that can be scanned by mobile devices
 * to automatically connect to the ESP32 Access Point. The QR code follows
 * the WiFi QR code standard format: WIFI:T:<security>;S:<SSID>;P:<password>;;
 * 
 * The generated QR code is displayed as ASCII art in the serial console using
 * block characters (█ and ░) for maximum visibility. Mobile devices can scan
 * this QR code to automatically configure WiFi connection parameters.
 * 
 * QR Code Format Examples:
 * - WPA2: WIFI:T:WPA;S:ESP32-WiFiScanner;P:123456789;;
 * - Open: WIFI:T:nopass;S:ESP32-WiFiScanner;P:;;
 * - WEP:  WIFI:T:WEP;S:ESP32-WiFiScanner;P:123456789;;
 * 
 * @param ssid The Access Point SSID (network name)
 * @param password The Access Point password (can be empty for open networks)
 * @param security Security type: "WPA", "WEP", or "nopass" for open networks
 * 
 * @note The QR code size is automatically determined based on the data length
 * @note Uses Unicode block characters for optimal mobile camera recognition
 * @note Error correction level is set to LOW for faster generation
 * 
 * @see startAccessPoint() for integration with AP startup
 * @see https://github.com/zxing/zxing/wiki/Barcode-Contents#wifi-network-config-android
 * 
 * @return void No return value, outputs QR code directly to Serial
 */
void generateAPQRCode(const String& ssid, const String& password, const String& security) {
  // Create WiFi QR code string format: WIFI:T:<security>;S:<SSID>;P:<password>;;
  String qrData = "WIFI:T:" + security + ";S:" + ssid + ";P:" + password + ";;";
  
  Serial.println();
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println("                    AP CONNECTION QR CODE");
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println("Scan this QR code with your mobile device to connect:");
  Serial.println();
  
  // Create QR code object
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];  // Version 3 should be sufficient for WiFi data
  
  // Generate QR code
  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrData.c_str());
  
  // Print QR code as ASCII art
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        Serial.print("██");  // Full block for black modules
      } else {
        Serial.print("  ");  // Two spaces for white modules
      }
    }
    Serial.println();
  }
  
  Serial.println();
  Serial.println("Network Information:");
  Serial.printf("  SSID: %s\n", ssid.c_str());
  Serial.printf("  Security: %s\n", security.c_str());
  if (password.length() > 0) {
    Serial.printf("  Password: %s\n", password.c_str());
  }
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println();
}
