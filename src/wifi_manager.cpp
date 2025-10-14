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
  Serial.println("\n--- Scanning for WiFi networks ---");
  
  // Start scan
  int networkCount = WiFi.scanNetworks();
  
  if (networkCount == 0) {
    Serial.println("No networks found");
  } else {
    Serial.print("Found ");
    Serial.print(networkCount);
    Serial.println(" networks:");
    Serial.println();
    
    // Print network details
    Serial.println("ID | SSID                     | RSSI | CH | Encryption");
    Serial.println("---|--------------------------|------|----|-----------");
    
    for (int i = 0; i < networkCount; ++i) {
      // Print network number (1-indexed for user friendliness)
      Serial.printf("%2d | ", i + 1);
      
      // Print SSID (truncate if too long)
      String ssid = WiFi.SSID(i);
      if (ssid.length() > 24) {
        ssid = ssid.substring(0, 21) + "...";
      }
      Serial.printf("%-24s | ", ssid.c_str());
      
      // Print RSSI (signal strength)
      Serial.printf("%4d | ", WiFi.RSSI(i));
      
      // Print channel
      Serial.printf("%2d | ", WiFi.channel(i));
      
      // Print encryption type
      wifi_auth_mode_t encryptionType = WiFi.encryptionType(i);
      switch (encryptionType) {
        case WIFI_AUTH_OPEN:
          Serial.println("Open");
          break;
        case WIFI_AUTH_WEP:
          Serial.println("WEP");
          break;
        case WIFI_AUTH_WPA_PSK:
          Serial.println("WPA");
          break;
        case WIFI_AUTH_WPA2_PSK:
          Serial.println("WPA2");
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          Serial.println("WPA/WPA2");
          break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          Serial.println("WPA2-Enterprise");
          break;
        case WIFI_AUTH_WPA3_PSK:
          Serial.println("WPA3");
          break;
        default:
          Serial.println("Unknown");
      }
    }
    Serial.println("--- End of scan ---\n");
  }
  
  // Clean up
  WiFi.scanDelete();
  promptShown = false; // Show prompt after scan results
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
// PING TEST FUNCTIONALITY
// ==========================================

/**
 * @brief Simple connectivity test using UDP echo
 * 
 * This function performs a simple connectivity test by attempting to
 * connect to a target IP address on a known port (like DNS port 53).
 * It measures round-trip time for connection establishment.
 * 
 * @param targetIP Target IP address to test
 * @param port Port number to test (default 53 for DNS)
 * @param timeout_ms Connection timeout in milliseconds
 * 
 * @return Response time in milliseconds, or -1 if failed
 */
long testConnectivity(IPAddress targetIP, int port, int timeout_ms) {
  WiFiClient client;
  unsigned long startTime = millis();
  
  client.setTimeout(timeout_ms);
  bool connected = client.connect(targetIP, port);
  unsigned long responseTime = millis() - startTime;
  
  if (connected) {
    client.stop();
    return responseTime;
  } else {
    return -1;
  }
}

/**
 * @brief Performs a connectivity test to a specified IP address or hostname
 * 
 * This function executes a series of TCP connection tests to measure network
 * connectivity and round-trip times. It supports both IP addresses and
 * hostnames with automatic DNS resolution.
 * 
 * The connectivity test performs 4 connection attempts to different ports:
 * - Port 53 (DNS) - Most commonly open
 * - Port 80 (HTTP) - Web traffic
 * - Port 443 (HTTPS) - Secure web traffic  
 * - Port 8080 (Alternative HTTP) - Common alternate port
 * 
 * For each successful connection, it displays the response time. This provides
 * a practical connectivity test that works even when ICMP ping is blocked.
 * 
 * @param target String containing IP address (e.g., "8.8.8.8") or hostname (e.g., "google.com")
 * 
 * @pre WiFi must be connected (station mode with active connection or AP mode)
 * @pre Target must be reachable through current network configuration
 * 
 * @note Uses TCP connection establishment time as connectivity metric
 * @note DNS resolution is automatic for hostname targets
 * @note Works even when ICMP ping is blocked by firewalls
 * @note Multiple ports tested to increase success probability
 * 
 * @warning Some targets may not have all tested ports open
 * @warning Connection times may vary significantly based on target load
 * 
 * @see connectToNetwork() for establishing network connection
 * @see testConnectivity() for individual port testing
 * 
 * @return void Results are displayed via Serial output
 * 
 * @par Example Usage:
 * @code
 * performPingTest("8.8.8.8");        // Test Google DNS connectivity
 * performPingTest("google.com");      // Test with hostname resolution
 * performPingTest("192.168.1.1");    // Test local gateway
 * @endcode
 * 
 * @par Example Output:
 * @code
 * 🏓 Testing connectivity to google.com [142.250.191.14]
 * Port 53 (DNS): ✓ Connected in 24ms
 * Port 80 (HTTP): ✓ Connected in 22ms  
 * Port 443 (HTTPS): ✓ Connected in 26ms
 * Port 8080 (Alt HTTP): ✗ Timeout
 * 
 * Summary: 3/4 ports reachable, avg response: 24ms
 * @endcode
 */
void performPingTest(String target) {
  // Check if WiFi is connected
  if (currentMode != MODE_STATION && currentMode != MODE_AP) {
    Serial.println("✗ Error: WiFi not active. Use 'mode station' or 'mode ap' first");
    return;
  }
  
  if (currentMode == MODE_STATION && WiFi.status() != WL_CONNECTED) {
    Serial.println("✗ Error: Not connected to a network. Use 'connect <ssid> <password>' first");
    return;
  }
  
  // Resolve hostname to IP if needed
  IPAddress targetIP;
  if (!targetIP.fromString(target)) {
    Serial.printf("🔍 Resolving hostname '%s'...\n", target.c_str());
    if (!WiFi.hostByName(target.c_str(), targetIP)) {
      Serial.printf("✗ Error: Could not resolve hostname '%s'\n", target.c_str());
      return;
    }
  }
  
  Serial.printf("\n🏓 Testing connectivity to %s [%s]\n", 
                target.c_str(), targetIP.toString().c_str());
  
  // Test multiple ports to increase success probability
  struct {
    int port;
    const char* description;
  } testPorts[] = {
    {53, "DNS"},
    {80, "HTTP"}, 
    {443, "HTTPS"},
    {8080, "Alt HTTP"}
  };
  
  int successCount = 0;
  long totalTime = 0;
  int attempts = sizeof(testPorts) / sizeof(testPorts[0]);
  
  for (int i = 0; i < attempts; i++) {
    Serial.printf("Port %d (%s): ", testPorts[i].port, testPorts[i].description);
    
    long responseTime = testConnectivity(targetIP, testPorts[i].port, 3000);
    
    if (responseTime >= 0) {
      Serial.printf("✓ Connected in %ldms\n", responseTime);
      successCount++;
      totalTime += responseTime;
    } else {
      Serial.println("✗ Timeout");
    }
    
    delay(500); // Brief pause between tests
  }
  
  // Display summary
  Serial.println();
  if (successCount > 0) {
    long avgTime = totalTime / successCount;
    Serial.printf("✓ Summary: %d/%d ports reachable, avg response: %ldms\n", 
                  successCount, attempts, avgTime);
  } else {
    Serial.printf("✗ Summary: No ports reachable on %s\n", targetIP.toString().c_str());
    Serial.println("  Check network connectivity and target availability");
  }
  
  Serial.println();
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
