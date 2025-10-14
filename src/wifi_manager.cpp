#include "wifi_manager.h"
#include "command_interface.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>
#include <qrcode.h>

// ==========================================
// GLOBAL STATE VARIABLES
// ==========================================
WiFiMode currentMode = MODE_IDLE;
bool scanningEnabled = false;
unsigned long lastScan = 0;
unsigned long lastBlink = 0;
bool ledState = false;

// ==========================================
// ACCESS POINT CONFIGURATION VARIABLES
// ==========================================
String currentAPSSID = AP_SSID;
String currentAPPassword = AP_PASSWORD;

// ==========================================
// WIFI INITIALIZATION
// ==========================================
void initializeWiFi() {
  // WiFi will be initialized when mode is selected
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
  
  Serial.println("✓ Station mode activated - Ready to scan for networks");
  Serial.println("  Use 'scan on' to start scanning");
  promptShown = false;
}

void startAccessPoint() {
  stopWiFi();
  delay(100);
  
  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP(currentAPSSID, currentAPPassword);
  
  if (apStarted) {
    currentMode = MODE_AP;
    scanningEnabled = false;
    
    Serial.println("✓ Access Point mode activated");
    Serial.print("  SSID: ");
    Serial.println(currentAPSSID);
    Serial.print("  Password: ");
    Serial.println(currentAPPassword);
    Serial.print("  IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("  Use 'ap info' for detailed information");
    
    // Generate and display QR code for easy mobile connection
    generateAPQRCode(currentAPSSID, currentAPPassword, "WPA");
    
    promptShown = false;
  } else {
    currentMode = MODE_OFF;
    Serial.println("✗ Failed to start Access Point");
    promptShown = false;
  }
}

void startAccessPoint(const String& ssid, const String& password) {
  // Validate SSID and password
  if (ssid.length() == 0 || ssid.length() > 32) {
    Serial.println("✗ Error: SSID must be 1-32 characters long");
    return;
  }
  
  if (password.length() < 8 || password.length() > 63) {
    Serial.println("✗ Error: Password must be 8-63 characters long for WPA2");
    return;
  }
  
  // Update current AP configuration
  currentAPSSID = ssid;
  currentAPPassword = password;
  
  // Start AP with new configuration
  stopWiFi();
  delay(100);
  
  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP(currentAPSSID, currentAPPassword);
  
  if (apStarted) {
    currentMode = MODE_AP;
    scanningEnabled = false;
    
    Serial.println("✓ Custom Access Point mode activated");
    Serial.print("  SSID: ");
    Serial.println(currentAPSSID);
    Serial.print("  Password: ");
    Serial.println(currentAPPassword);
    Serial.print("  IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("  Use 'ap info' for detailed information");
    
    // Generate and display QR code for easy mobile connection
    generateAPQRCode(currentAPSSID, currentAPPassword, "WPA");
    
    promptShown = false;
  } else {
    currentMode = MODE_OFF;
    Serial.println("✗ Failed to start custom Access Point");
    Serial.println("  Check SSID and password requirements");
    
    // Restore default configuration on failure
    currentAPSSID = AP_SSID;
    currentAPPassword = AP_PASSWORD;
    
    promptShown = false;
  }
}

void stopWiFi() {
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  currentMode = MODE_OFF;
  scanningEnabled = false;
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("✓ WiFi disabled");
}

void setIdleMode() {
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  currentMode = MODE_IDLE;
  scanningEnabled = false;
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("🟡 Device set to IDLE mode - Ready for commands");
  Serial.println("  Use 'mode station' or 'mode ap' to activate WiFi");
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
  promptShown = false; // Show prompt after scan results
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
  
  Serial.printf("│ 🚀 Est. Speed:   %-40s │\n", estimatedSpeed.c_str());
  
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
  promptShown = false;
}

// ==========================================
// NETWORK CONNECTION
// ==========================================

/**
 * @brief Connects to a WiFi network using SSID and password
 * 
 * This function establishes a connection to a specific WiFi network when
 * the ESP32 is in station mode. It handles the connection process and
 * provides feedback on the connection status.
 * 
 * @param ssid Network name to connect to
 * @param password Network password for authentication
 * 
 * @pre Device must be in station mode (MODE_STATION)
 * @pre SSID must be available and in range
 * @pre Password must be correct for the target network
 * 
 * @note Connection timeout is set to 10 seconds
 * @note Displays connection progress and final status
 * 
 * @return void Outputs connection results to Serial
 */
void connectToNetwork(String ssid, String password) {
  if (currentMode != MODE_STATION) {
    Serial.println("✗ Error: Must be in station mode to connect. Use 'mode station' first");
    return;
  }
  
  Serial.printf("🔗 Connecting to '%s'...\n", ssid.c_str());
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {  // 10 seconds timeout
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.printf("✓ Connected to '%s'\n", ssid.c_str());
    Serial.printf("  IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("  DNS: %s\n", WiFi.dnsIP().toString().c_str());
  } else {
    Serial.println();
    Serial.printf("✗ Failed to connect to '%s'\n", ssid.c_str());
    Serial.println("  Check SSID, password, and signal strength");
  }
  
  promptShown = false;
}

/**
 * @brief Disconnects from current WiFi network
 * 
 * Disconnects from the currently connected WiFi network while maintaining
 * station mode for future connections.
 */
void disconnectFromNetwork() {
  if (currentMode != MODE_STATION) {
    Serial.println("✗ Error: Must be in station mode");
    return;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    String ssid = WiFi.SSID();
    WiFi.disconnect();
    Serial.printf("✓ Disconnected from '%s'\n", ssid.c_str());
  } else {
    Serial.println("ℹ Not connected to any network");
  }
  
  promptShown = false;
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
