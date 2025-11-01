#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#ifdef USE_WEBSERVER  // Web server available when enabled

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Web server configuration
#define WEB_SERVER_PORT 80

// Web server state
extern WebServer* webServer;
extern bool webServerEnabled;

// Initialize web server
void initializeWebServer();

// Start/stop web server
bool startWebServer();
void stopWebServer();

// Handle web server requests
void handleWebServerRequests();

// Monitor and auto-restart web server based on WiFi state
void monitorWebServerState();

// Web server status
bool isWebServerRunning();
String getWebServerURL();

// Page handlers
void handleRoot();
void handleStatus();
void handleScan();
void handleScanDetails();  // New handler for network details
void handleNetworkAnalysis();
void handleChannelAnalysis();
void handleChannelScan();
void handleChannelGraph();
void handleLatency();
void handleLatencyStart();
void handleLatencyStop();
void handleIperf();
void handleIperfStart();
void handleIperfStop();
void handleIperfResults();
void handleConfig();
void handleConfigAP();
void handleConfigStation();
void handleConfigClear();
void handleReboot();
void handleModeSwitch();
void handleSignalMonitor();
void handleSignalStrengthAPI();
void handlePortScanner();
void handlePortScanStart();
void handlePortScanStop();
void handlePortScanStatus();
void handlePortScanAPI();
void handleNotFound();

#endif // USE_WEBSERVER

#endif // WEB_SERVER_H
