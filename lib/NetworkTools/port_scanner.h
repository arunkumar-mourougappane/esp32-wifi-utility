#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

// ==========================================
// PORT SCANNER CONFIGURATION
// ==========================================

// Common port definitions
#define MAX_PORTS_TO_SCAN 100
#define DEFAULT_SCAN_TIMEOUT 1000  // milliseconds
#define CONCURRENT_CONNECTIONS 5   // Number of simultaneous port checks

// ==========================================
// PORT SCANNER STATE
// ==========================================

enum PortScanState {
    PORTSCAN_IDLE,
    PORTSCAN_RUNNING,
    PORTSCAN_COMPLETED,
    PORTSCAN_ERROR
};

// ==========================================
// DATA STRUCTURES
// ==========================================

struct PortInfo {
    uint16_t port;
    bool isOpen;
    String service;      // Common service name for the port
    uint32_t responseTime;  // Response time in milliseconds
};

struct PortScanConfig {
    String targetIP;
    uint16_t startPort;
    uint16_t endPort;
    uint32_t timeout;
    bool scanCommonOnly;  // If true, scan only common ports
};

struct PortScanResults {
    String targetIP;
    uint16_t totalPorts;
    uint16_t portsScanned;
    uint16_t openPorts;
    uint16_t closedPorts;
    unsigned long startTime;
    unsigned long endTime;
    std::vector<PortInfo> openPortsList;
    bool scanCompleted;
};

// ==========================================
// GLOBAL STATE
// ==========================================

extern PortScanState currentPortScanState;
extern PortScanConfig activePortScanConfig;
extern PortScanResults lastPortScanResults;

// ==========================================
// PORT SCANNER FUNCTIONS
// ==========================================

/**
 * @brief Initialize port scanner
 */
void initializePortScanner();

/**
 * @brief Start a port scan
 * @param targetIP Target IP address to scan
 * @param startPort Starting port number
 * @param endPort Ending port number
 * @param timeout Connection timeout in milliseconds
 * @return true if scan started successfully
 */
bool startPortScan(const String& targetIP, uint16_t startPort, uint16_t endPort, uint32_t timeout = DEFAULT_SCAN_TIMEOUT);

/**
 * @brief Start a scan of common ports only
 * @param targetIP Target IP address to scan
 * @return true if scan started successfully
 */
bool startCommonPortScan(const String& targetIP);

/**
 * @brief Stop current port scan
 */
void stopPortScan();

/**
 * @brief Handle port scanning background tasks (call in loop)
 */
void handlePortScanTasks();

/**
 * @brief Get current port scan state
 * @return Current scan state
 */
PortScanState getPortScanState();

/**
 * @brief Get last port scan results
 * @return Last scan results structure
 */
PortScanResults getLastPortScanResults();

/**
 * @brief Get scan progress percentage
 * @return Progress percentage (0-100)
 */
uint8_t getPortScanProgress();

/**
 * @brief Get service name for a port number
 * @param port Port number
 * @return Service name or "Unknown"
 */
String getServiceName(uint16_t port);

/**
 * @brief Check if a specific port is open
 * @param targetIP Target IP address
 * @param port Port number to check
 * @param timeout Connection timeout in milliseconds
 * @return true if port is open
 */
bool isPortOpen(const String& targetIP, uint16_t port, uint32_t timeout);

// ==========================================
// COMMON PORTS LIST
// ==========================================

// Returns list of common ports to scan
std::vector<uint16_t> getCommonPorts();
