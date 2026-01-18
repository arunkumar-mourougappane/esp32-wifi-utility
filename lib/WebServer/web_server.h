/**
 * @file web_server.h
 * @brief Web server interface for browser-based configuration and monitoring
 * 
 * This header defines the web server interface for the ESP32 WiFi Utility,
 * providing HTTP endpoints for configuration, status monitoring, network
 * analysis, and performance testing through a responsive web interface.
 * 
 * Features:
 * - WiFi configuration (AP and Station modes with security options)
 * - Network scanning and analysis
 * - Channel analysis with graphical visualization
 * - Latency testing and monitoring
 * - iPerf performance testing
 * - System status and control
 * - Automatic reconnection on WiFi state changes
 * 
 * @author Arunkumar Mourougappane
 * @version 1.0.0
 * @date 2026-01-17
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#ifdef USE_WEBSERVER  // Web server available when enabled

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Web server configuration
#define WEB_SERVER_PORT 80

// ==========================================
// WEB SERVER STATE
// ==========================================

/**
 * @brief Web server instance pointer
 */
extern WebServer* webServer;

/**
 * @brief Flag indicating if web server is enabled
 */
extern bool webServerEnabled;

// ==========================================
// INITIALIZATION AND LIFECYCLE
// ==========================================

/**
 * @brief Initialize web server subsystem
 * @details Sets up web server instance and registers HTTP handlers
 */
void initializeWebServer();

/**
 * @brief Start the web server on port 80
 * @return true if server started successfully, false otherwise
 */
bool startWebServer();

/**
 * @brief Stop the web server
 */
void stopWebServer();

/**
 * @brief Handle incoming web server requests
 * @details Call this function in the main loop to process HTTP requests
 */
void handleWebServerRequests();

/**
 * @brief Monitor WiFi state and auto-restart web server if needed
 * @details Automatically restarts web server when WiFi connection is re-established
 */
void monitorWebServerState();

// ==========================================
// STATUS FUNCTIONS
// ==========================================

/**
 * @brief Check if web server is currently running
 * @return true if server is active, false otherwise
 */
bool isWebServerRunning();

/**
 * @brief Get the web server URL
 * @return URL string (e.g., "http://192.168.1.100")
 */
String getWebServerURL();

// ==========================================
// PAGE HANDLERS
// ==========================================

/**
 * @brief Handle root page (/)
 * @details Displays main dashboard with system overview
 */
void handleRoot();

/**
 * @brief Handle status page (/status)
 * @details Shows detailed system and network status
 */
void handleStatus();

/**
 * @brief Handle WiFi scan page (/scan)
 * @details Displays list of available WiFi networks
 */
void handleScan();

/**
 * @brief Handle network details page (/scan/details)
 * @details Shows detailed information about a specific network
 */
void handleScanDetails();

/**
 * @brief Handle network analysis page (/network)
 * @details Provides network analysis dashboard
 */
void handleNetworkAnalysis();

/**
 * @brief Handle channel analysis page (/channel)
 * @details Displays channel congestion analysis
 */
void handleChannelAnalysis();

/**
 * @brief Handle channel scan endpoint (/channel/scan)
 * @details Triggers channel congestion scan
 */
void handleChannelScan();

/**
 * @brief Handle channel graph page (/channel/graph)
 * @details Displays graphical channel visualization
 */
void handleChannelGraph();

/**
 * @brief Handle latency test page (/latency)
 * @details Provides latency testing interface
 */
void handleLatency();

/**
 * @brief Handle latency test start endpoint (/latency/start)
 * @details Starts latency testing with specified parameters
 */
void handleLatencyStart();

/**
 * @brief Handle latency test stop endpoint (/latency/stop)
 * @details Stops active latency test
 */
void handleLatencyStop();

/**
 * @brief Handle iPerf page (/iperf)
 * @details Provides iPerf testing interface
 */
void handleIperf();

/**
 * @brief Handle iPerf test start endpoint (/iperf/start)
 * @details Starts iPerf test with specified parameters
 */
void handleIperfStart();

/**
 * @brief Handle iPerf test stop endpoint (/iperf/stop)
 * @details Stops active iPerf test
 */
void handleIperfStop();

/**
 * @brief Handle iPerf results endpoint (/iperf/results)
 * @details Returns iPerf test results in JSON format
 */
void handleIperfResults();

/**
 * @brief Handle configuration page (/config)
 * @details Displays AP and Station configuration forms with security options
 */
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
