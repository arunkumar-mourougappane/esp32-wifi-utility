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

// Web server status
bool isWebServerRunning();
String getWebServerURL();

// Page handlers
void handleRoot();
void handleStatus();
void handleScan();
void handleNetworkAnalysis();
void handleChannelAnalysis();
void handleChannelScan();
void handleLatency();
void handleLatencyStart();
void handleLatencyStop();
void handleIperf();
void handleIperfStart();
void handleIperfStop();
void handleIperfResults();
void handleNotFound();

#endif // USE_WEBSERVER

#endif // WEB_SERVER_H
