/**
 * @file ap_manager.cpp
 * @brief Access Point client management and deauthentication functionality
 * 
 * This module provides comprehensive management of WiFi clients connected to the ESP32
 * when operating in Access Point (AP) mode. It includes functionality for listing
 * connected clients, managing client connections, and performing selective or mass
 * deauthentication operations.
 * 
 * The module integrates with the ESP-IDF WiFi API to retrieve client information
 * and uses the Arduino WiFi library for client management operations.
 * 
 * @author ESP32 WiFi Scanner Project
 * @version 1.0
 * @date 2025-10-13
 * 
 * @see wifi_manager.h for WiFi mode management
 * @see command_interface.h for user command processing
 */

#include "ap_manager.h"
#include "wifi_manager.h"
#include "command_interface.h"

// ==========================================
// AP CLIENT MANAGEMENT VARIABLES
// ==========================================

/**
 * @brief Station list structure containing connected client information
 * 
 * This structure is populated by the ESP-IDF API and contains detailed information
 * about all clients currently connected to the Access Point, including MAC addresses
 * and signal strength (RSSI) values.
 */
wifi_sta_list_t stationList;

/**
 * @brief Current number of connected clients
 * 
 * This variable tracks the number of clients currently connected to the Access Point.
 * It is updated by updateClientList() and used throughout the module for bounds
 * checking and user feedback.
 */
uint8_t connectedClients = 0;

// ==========================================
// CLIENT LIST MANAGEMENT
// ==========================================

/**
 * @brief Updates the internal client list with current connected clients
 * 
 * This function retrieves the current list of clients connected to the Access Point
 * using the ESP-IDF WiFi API. It only operates when the device is in AP mode and
 * updates both the global stationList structure and connectedClients counter.
 * 
 * The function uses esp_wifi_ap_get_sta_list() to query the low-level WiFi driver
 * for current client information including MAC addresses and RSSI values.
 * 
 * @note This function should be called before any operations that require current
 *       client information (listing, deauthentication, etc.)
 * 
 * @warning Only functions correctly when currentMode == MODE_AP
 * 
 * @see listAPClients() for displaying the updated client list
 * @see deauthClient() for client-specific operations using this data
 * 
 * @return void No return value, updates global stationList and connectedClients
 */
void updateClientList() {
  if (currentMode != MODE_AP) return;
  
  esp_err_t result = esp_wifi_ap_get_sta_list(&stationList);
  if (result == ESP_OK) {
    connectedClients = stationList.num;
  }
}

/**
 * @brief Displays a formatted list of all connected Access Point clients
 * 
 * This function provides a comprehensive display of all clients currently connected
 * to the ESP32 Access Point. It shows client information in a tabular format with
 * client IDs (for easy reference in deauth commands), MAC addresses, and signal
 * strength values.
 * 
 * The function first updates the client list to ensure current information, then
 * formats and displays the data with the following columns:
 * - ID: Sequential number (1-based) for easy command reference
 * - MAC Address: Full 6-byte MAC address in XX:XX:XX:XX:XX:XX format
 * - RSSI: Signal strength in dBm (negative values, closer to 0 = stronger)
 * 
 * Additionally provides user guidance for deauthentication commands and displays
 * the total client count. Uses Unicode box-drawing characters for professional
 * table formatting.
 * 
 * @note Automatically calls updateClientList() to ensure current data
 * @note Handles both empty (no clients) and populated client lists
 * @note Client IDs shown are 1-based for user friendliness
 * 
 * @see updateClientList() for the underlying data retrieval
 * @see deauthClient() for using the displayed client IDs
 * 
 * @return void Outputs directly to Serial, no return value
 * 
 * @par Example Output:
 * @code
 * 👥 CONNECTED CLIENTS:
 * ─────────────────────
 * ID │ MAC Address       │ RSSI
 * ───┼───────────────────┼──────
 *  1 │ AA:BB:CC:DD:EE:FF │  -45
 *  2 │ 11:22:33:44:55:66 │  -52
 * 
 * Total clients: 2
 * @endcode
 */
void listAPClients() {
  updateClientList();
  
  Serial.println("\n👥 CONNECTED CLIENTS:");
  Serial.println("─────────────────────");
  
  if (connectedClients == 0) {
    Serial.println("No clients connected");
  } else {
    Serial.println("ID │ MAC Address       │ RSSI");
    Serial.println("───┼───────────────────┼──────");
    
    for (int i = 0; i < connectedClients && i < ESP_WIFI_MAX_CONN_NUM; i++) {
      Serial.printf("%2d │ ", i + 1);
      
      // Print MAC address
      for (int j = 0; j < 6; j++) {
        Serial.printf("%02X", stationList.sta[i].mac[j]);
        if (j < 5) Serial.print(":");
      }
      
      Serial.printf(" │ %4d\n", stationList.sta[i].rssi);
    }
  }
  
  Serial.print("\nTotal clients: ");
  Serial.println(connectedClients);
  Serial.println("Use 'deauth <ID>' (e.g., 'deauth 1') or 'deauth <MAC>' to disconnect");
  Serial.println("Use 'deauth all' to disconnect all clients\n");
}

// ==========================================
// CLIENT DEAUTHENTICATION
// ==========================================

/**
 * @brief Deauthenticates a specific client from the Access Point
 * 
 * This function disconnects a specific client from the ESP32 Access Point using
 * either the client ID (from listAPClients output) or the client's MAC address.
 * It provides flexible input parsing to accept multiple format variations.
 * 
 * The function supports two input methods:
 * 1. **Client ID**: Numeric identifier (1-based) from the client list display
 * 2. **MAC Address**: Full MAC address in various formats
 * 
 * Input parsing handles the following MAC address formats:
 * - Colon-separated: "AA:BB:CC:DD:EE:FF"
 * - No separators: "AABBCCDDEEFF" 
 * - Case insensitive: both upper and lowercase
 * 
 * The deauthentication uses WiFi.softAPdisconnect() which sends a deauth frame
 * to the target client, forcing disconnection. Most clients will attempt to
 * reconnect automatically unless blocked by other means.
 * 
 * @param input String containing either client ID number or MAC address
 *              - Client ID: "1", "2", "3", etc. (matches listAPClients output)
 *              - MAC formats: "AA:BB:CC:DD:EE:FF", "AABBCCDDEEFF"
 * 
 * @pre Device must be in Access Point mode (currentMode == MODE_AP)
 * @pre For client ID input: Must be valid ID from current client list (1 to connectedClients)
 * @pre For MAC input: Must be 12 hex characters (with or without colons)
 * 
 * @post Target client receives deauthentication frame and is disconnected
 * @post Client list is updated to reflect current connections
 * 
 * @note Automatically updates client list before processing
 * @note Validates input format and provides specific error messages
 * @note Uses case-insensitive MAC address comparison
 * 
 * @warning Only works when ESP32 is in Access Point mode
 * @warning Deauthenticated clients may automatically reconnect
 * 
 * @see listAPClients() for obtaining client IDs and MAC addresses
 * @see deauthAllClients() for disconnecting all clients simultaneously
 * @see updateClientList() for refreshing client information
 * 
 * @return void Outputs result to Serial, no return value
 * 
 * @par Example Usage:
 * @code
 * deauthClient("1");                    // Disconnect client ID 1
 * deauthClient("AA:BB:CC:DD:EE:FF");    // Disconnect by MAC (with colons)
 * deauthClient("aabbccddeeff");         // Disconnect by MAC (no colons, lowercase)
 * @endcode
 * 
 * @par Error Conditions:
 * - Not in AP mode: "Must be in AP mode to deauthenticate clients"
 * - Invalid client ID: "Invalid client ID. Use 'ap clients' to see valid IDs"
 * - Invalid MAC format: "Invalid format. Use client ID or MAC (XX:XX:XX:XX:XX:XX)"
 * - MAC not found: "MAC address not found in connected clients"
 * - Deauth failed: "Failed to disconnect client"
 */
void deauthClient(String input) {
  if (currentMode != MODE_AP) {
    Serial.println("✗ Error: Must be in AP mode to deauthenticate clients");
    return;
  }
  
  updateClientList();
  
  // Check if input is a client ID (number) or MAC address
  input.trim();
  bool isClientId = true;
  for (int i = 0; i < input.length(); i++) {
    if (!isdigit(input.charAt(i))) {
      isClientId = false;
      break;
    }
  }
  
  uint8_t targetMac[6];
  bool found = false;
  
  if (isClientId) {
    // Input is a client ID (1-based)
    int clientId = input.toInt();
    if (clientId >= 1 && clientId <= connectedClients) {
      memcpy(targetMac, stationList.sta[clientId - 1].mac, 6);
      found = true;
    } else {
      Serial.println("✗ Error: Invalid client ID. Use 'ap clients' to see valid IDs");
      return;
    }
  } else {
    // Input is a MAC address
    uint8_t inputMac[6];
    String macAddress = input;
    macAddress.replace(":", "");
    macAddress.toUpperCase();
    
    if (macAddress.length() != 12) {
      Serial.println("✗ Error: Invalid format. Use client ID (1,2,3...) or MAC (XX:XX:XX:XX:XX:XX)");
      return;
    }
    
    // Convert hex string to bytes
    for (int i = 0; i < 6; i++) {
      String byteString = macAddress.substring(i * 2, i * 2 + 2);
      inputMac[i] = strtol(byteString.c_str(), NULL, 16);
    }
    
    // Find matching client by MAC
    for (int i = 0; i < connectedClients && i < ESP_WIFI_MAX_CONN_NUM; i++) {
      if (memcmp(inputMac, stationList.sta[i].mac, 6) == 0) {
        memcpy(targetMac, stationList.sta[i].mac, 6);
        found = true;
        break;
      }
    }
    
    if (!found) {
      Serial.println("✗ Error: MAC address not found in connected clients");
      return;
    }
  }
  
  // Use WiFi.softAPdisconnect() method which is simpler
  bool result = WiFi.softAPdisconnect(targetMac);
  
  if (result) {
    Serial.print("✓ Client disconnected: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", targetMac[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  } else {
    Serial.println("✗ Failed to disconnect client");
  }
}

/**
 * @brief Deauthenticates all connected clients from the Access Point
 * 
 * This function performs a mass deauthentication of all clients currently connected
 * to the ESP32 Access Point. It iterates through the complete client list and
 * attempts to disconnect each client individually, providing detailed feedback
 * on the success/failure of each disconnection attempt.
 * 
 * The function provides comprehensive status reporting including:
 * - Initial warning with total client count
 * - Individual disconnect status for each client (with MAC addresses)
 * - Final summary of successful vs. failed disconnections
 * - Graceful handling of edge cases (no clients, partial failures)
 * 
 * Each client disconnection uses WiFi.softAPdisconnect() which sends a
 * deauthentication frame. The function tracks successes and failures separately
 * to provide accurate reporting even if some disconnections fail.
 * 
 * This is particularly useful for:
 * - Testing network resilience and client reconnection behavior
 * - Quickly clearing all connections for maintenance
 * - Access control scenarios requiring immediate disconnection
 * - Network administration and monitoring tasks
 * 
 * @pre Device must be in Access Point mode (currentMode == MODE_AP)
 * @pre Client list should contain valid connected clients
 * 
 * @post All connected clients receive deauthentication frames
 * @post Client list is updated to reflect current connections
 * @post Detailed operation summary is displayed via Serial
 * 
 * @note Automatically updates client list before processing
 * @note Handles empty client lists gracefully
 * @note Provides individual feedback for each disconnection attempt
 * @note Counts and reports both successful and failed attempts
 * 
 * @warning Only works when ESP32 is in Access Point mode
 * @warning Disconnected clients may automatically reconnect unless blocked
 * @warning Mass disconnection may cause temporary network disruption
 * 
 * @see deauthClient() for disconnecting specific clients
 * @see listAPClients() for viewing clients before mass disconnection
 * @see updateClientList() for refreshing client information
 * 
 * @return void Outputs detailed results to Serial, no return value
 * 
 * @par Example Output:
 * @code
 * ⚠️  Deauthenticating all 3 connected clients...
 * ✓ Disconnected: AA:BB:CC:DD:EE:FF
 * ✓ Disconnected: 11:22:33:44:55:66  
 * ✗ Failed to disconnect: 77:88:99:AA:BB:CC
 * ✓ Successfully deauthenticated 2 out of 3 clients
 * @endcode
 * 
 * @par Error Conditions:
 * - Not in AP mode: "Must be in AP mode to deauthenticate clients"
 * - No clients: "No clients connected to deauthenticate" 
 * - Individual failures: "Failed to disconnect: [MAC]" for each failed attempt
 */
void deauthAllClients() {
  if (currentMode != MODE_AP) {
    Serial.println("✗ Error: Must be in AP mode to deauthenticate clients");
    return;
  }
  
  updateClientList();
  
  if (connectedClients == 0) {
    Serial.println("ℹ No clients connected to deauthenticate");
    return;
  }
  
  Serial.print("⚠️  Deauthenticating all ");
  Serial.print(connectedClients);
  Serial.println(" connected clients...");
  
  int successful = 0;
  for (int i = 0; i < connectedClients && i < ESP_WIFI_MAX_CONN_NUM; i++) {
    bool result = WiFi.softAPdisconnect(stationList.sta[i].mac);
    if (result) {
      successful++;
      Serial.print("✓ Disconnected: ");
      for (int j = 0; j < 6; j++) {
        Serial.printf("%02X", stationList.sta[i].mac[j]);
        if (j < 5) Serial.print(":");
      }
      Serial.println();
    } else {
      Serial.print("✗ Failed to disconnect: ");
      for (int j = 0; j < 6; j++) {
        Serial.printf("%02X", stationList.sta[i].mac[j]);
        if (j < 5) Serial.print(":");
      }
      Serial.println();
    }
  }
  
  Serial.print("✓ Successfully deauthenticated ");
  Serial.print(successful);
  Serial.print(" out of ");
  Serial.print(connectedClients);
  Serial.println(" clients");
}

// ==========================================
// AP INFORMATION
// ==========================================

/**
 * @brief Displays comprehensive Access Point configuration and status information
 * 
 * This function provides a detailed overview of the ESP32 Access Point configuration
 * and current operational status. It presents all relevant network information in
 * a formatted, user-friendly display that helps with network administration and
 * troubleshooting.
 * 
 * The information displayed includes:
 * - **SSID**: Network name visible to clients during connection
 * - **Password**: Current WPA2 password for client authentication  
 * - **IP Address**: IPv4 address of the Access Point (typically 192.168.4.1)
 * - **MAC Address**: Hardware MAC address of the ESP32 WiFi interface
 * - **Connected Clients**: Current number of active client connections
 * - **Channel**: WiFi channel being used for the Access Point
 * 
 * This information is particularly useful for:
 * - Network configuration verification
 * - Troubleshooting connection issues
 * - Providing connection details to users
 * - Monitoring Access Point status
 * - Network administration tasks
 * 
 * The display uses Unicode box-drawing characters and emoji icons for
 * professional presentation and easy readability in terminal applications.
 * 
 * @note Information is retrieved in real-time from WiFi library functions
 * @note SSID and Password are retrieved from config.h constants
 * @note Client count reflects current active connections
 * @note Function can be called at any time when in AP mode
 * 
 * @warning Display shows actual password - use caution in shared environments
 * @warning Channel information may not be available immediately after AP start
 * 
 * @see config.h for AP_SSID and AP_PASSWORD configuration
 * @see listAPClients() for detailed client information
 * @see WiFi.softAPIP() for IP address configuration
 * 
 * @return void Outputs formatted information to Serial, no return value
 * 
 * @par Example Output:
 * @code
 * 📡 ACCESS POINT INFORMATION:
 * ─────────────────────────────
 * SSID: ESP32-WiFiScanner
 * Password: 12345678
 * IP Address: 192.168.4.1
 * MAC Address: 24:6F:28:XX:XX:XX
 * Connected Clients: 2
 * Channel: 1
 * @endcode
 */
void printAPInfo() {
  Serial.println("\n📡 ACCESS POINT INFORMATION:");
  Serial.println("─────────────────────────────");
  Serial.print("SSID: ");
  Serial.println(currentAPSSID);
  Serial.print("Password: ");
  Serial.println(currentAPPassword);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.print("Connected Clients: ");
  Serial.println(WiFi.softAPgetStationNum());
  Serial.print("Channel: ");
  Serial.println(WiFi.channel());
  Serial.println();
}
