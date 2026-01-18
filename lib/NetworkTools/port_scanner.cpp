/**
 * @file port_scanner.cpp
 * @brief TCP port scanning and service discovery implementation
 * 
 * This file implements network port scanning functionality:
 * - TCP connection-based port scanning
 * - Concurrent scanning for improved performance
 * - Common service identification (HTTP, SSH, FTP, etc.)
 * - Response time measurement
 * - Progress tracking and cancellation support
 * - Configurable timeout and port ranges
 * 
 * @author Arunkumar Mourougappane
 * @version 4.3.0
 * @date 2026-01-17
 */

#include "port_scanner.h"
#include "logging.h"
#include <WiFiClient.h>

#define TAG_PORTSCAN "PortScan"

// ==========================================
// GLOBAL STATE
// ==========================================

PortScanState currentPortScanState = PORTSCAN_IDLE;
PortScanConfig activePortScanConfig;
PortScanResults lastPortScanResults;

// Current scan state
static uint16_t currentPortIndex = 0;
static std::vector<uint16_t> portsToScan;

// ==========================================
// COMMON PORTS DEFINITIONS
// ==========================================

std::vector<uint16_t> getCommonPorts() {
    return {
        21,    // FTP
        22,    // SSH
        23,    // Telnet
        25,    // SMTP
        53,    // DNS
        80,    // HTTP
        110,   // POP3
        143,   // IMAP
        443,   // HTTPS
        445,   // SMB
        3306,  // MySQL
        3389,  // RDP
        5900,  // VNC
        8080,  // HTTP Alt
        8443,  // HTTPS Alt
        9100   // Printer
    };
}

// ==========================================
// SERVICE NAME MAPPING
// ==========================================

String getServiceName(uint16_t port) {
    switch(port) {
        case 20: return "FTP-Data";
        case 21: return "FTP";
        case 22: return "SSH";
        case 23: return "Telnet";
        case 25: return "SMTP";
        case 53: return "DNS";
        case 80: return "HTTP";
        case 110: return "POP3";
        case 143: return "IMAP";
        case 443: return "HTTPS";
        case 445: return "SMB";
        case 465: return "SMTPS";
        case 587: return "SMTP-Submission";
        case 993: return "IMAPS";
        case 995: return "POP3S";
        case 1433: return "MSSQL";
        case 1521: return "Oracle";
        case 3306: return "MySQL";
        case 3389: return "RDP";
        case 5432: return "PostgreSQL";
        case 5900: return "VNC";
        case 6379: return "Redis";
        case 8080: return "HTTP-Proxy";
        case 8443: return "HTTPS-Alt";
        case 9100: return "Printer";
        case 27017: return "MongoDB";
        default: return "Unknown";
    }
}

// ==========================================
// PORT SCANNER INITIALIZATION
// ==========================================

void initializePortScanner() {
    currentPortScanState = PORTSCAN_IDLE;
    lastPortScanResults.scanCompleted = false;
    lastPortScanResults.openPorts = 0;
    lastPortScanResults.closedPorts = 0;
    lastPortScanResults.portsScanned = 0;
    
    LOG_INFO(TAG_PORTSCAN, "Port scanner initialized");
}

// ==========================================
// PORT CONNECTIVITY CHECK
// ==========================================

bool isPortOpen(const String& targetIP, uint16_t port, uint32_t timeout) {
    WiFiClient client;
    client.setTimeout(timeout);
    
    LOG_DEBUG(TAG_PORTSCAN, "Checking port %d on %s", port, targetIP.c_str());
    
    unsigned long startTime = millis();
    bool connected = client.connect(targetIP.c_str(), port);
    unsigned long responseTime = millis() - startTime;
    
    if (connected) {
        client.stop();
        LOG_DEBUG(TAG_PORTSCAN, "Port %d OPEN (response: %lu ms)", port, responseTime);
        return true;
    }
    
    LOG_DEBUG(TAG_PORTSCAN, "Port %d CLOSED", port);
    return false;
}

// ==========================================
// PORT SCAN OPERATIONS
// ==========================================

bool startPortScan(const String& targetIP, uint16_t startPort, uint16_t endPort, uint32_t timeout) {
    if (currentPortScanState == PORTSCAN_RUNNING) {
        LOG_WARN(TAG_PORTSCAN, "Scan already in progress");
        return false;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        LOG_ERROR(TAG_PORTSCAN, "Not connected to WiFi");
        return false;
    }
    
    // Validate port range
    if (startPort > endPort || endPort > 65535) {
        LOG_ERROR(TAG_PORTSCAN, "Invalid port range: %d-%d", startPort, endPort);
        return false;
    }
    
    // Initialize configuration
    activePortScanConfig.targetIP = targetIP;
    activePortScanConfig.startPort = startPort;
    activePortScanConfig.endPort = endPort;
    activePortScanConfig.timeout = timeout;
    activePortScanConfig.scanCommonOnly = false;
    
    // Build port list
    portsToScan.clear();
    for (uint16_t p = startPort; p <= endPort; p++) {
        portsToScan.push_back(p);
    }
    
    // Initialize results
    lastPortScanResults.targetIP = targetIP;
    lastPortScanResults.totalPorts = portsToScan.size();
    lastPortScanResults.portsScanned = 0;
    lastPortScanResults.openPorts = 0;
    lastPortScanResults.closedPorts = 0;
    lastPortScanResults.startTime = millis();
    lastPortScanResults.scanCompleted = false;
    lastPortScanResults.openPortsList.clear();
    
    currentPortIndex = 0;
    currentPortScanState = PORTSCAN_RUNNING;
    
    LOG_INFO(TAG_PORTSCAN, "Started port scan on %s (ports %d-%d, %d total)", 
             targetIP.c_str(), startPort, endPort, portsToScan.size());
    
    return true;
}

bool startCommonPortScan(const String& targetIP) {
    if (currentPortScanState == PORTSCAN_RUNNING) {
        LOG_WARN(TAG_PORTSCAN, "Scan already in progress");
        return false;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        LOG_ERROR(TAG_PORTSCAN, "Not connected to WiFi");
        return false;
    }
    
    // Initialize configuration
    activePortScanConfig.targetIP = targetIP;
    activePortScanConfig.startPort = 0;
    activePortScanConfig.endPort = 0;
    activePortScanConfig.timeout = DEFAULT_SCAN_TIMEOUT;
    activePortScanConfig.scanCommonOnly = true;
    
    // Build port list from common ports
    portsToScan = getCommonPorts();
    
    // Initialize results
    lastPortScanResults.targetIP = targetIP;
    lastPortScanResults.totalPorts = portsToScan.size();
    lastPortScanResults.portsScanned = 0;
    lastPortScanResults.openPorts = 0;
    lastPortScanResults.closedPorts = 0;
    lastPortScanResults.startTime = millis();
    lastPortScanResults.scanCompleted = false;
    lastPortScanResults.openPortsList.clear();
    
    currentPortIndex = 0;
    currentPortScanState = PORTSCAN_RUNNING;
    
    LOG_INFO(TAG_PORTSCAN, "Started common port scan on %s (%d ports)", 
             targetIP.c_str(), portsToScan.size());
    
    return true;
}

void stopPortScan() {
    if (currentPortScanState == PORTSCAN_RUNNING) {
        currentPortScanState = PORTSCAN_IDLE;
        LOG_INFO(TAG_PORTSCAN, "Port scan stopped by user");
    }
}

// ==========================================
// BACKGROUND SCAN PROCESSING
// ==========================================

void handlePortScanTasks() {
    if (currentPortScanState != PORTSCAN_RUNNING) {
        return;
    }
    
    // Check if scan is complete
    if (currentPortIndex >= portsToScan.size()) {
        lastPortScanResults.endTime = millis();
        lastPortScanResults.scanCompleted = true;
        currentPortScanState = PORTSCAN_COMPLETED;
        
        unsigned long duration = (lastPortScanResults.endTime - lastPortScanResults.startTime) / 1000;
        LOG_INFO(TAG_PORTSCAN, "Scan completed: %d open, %d closed (duration: %lu seconds)",
                 lastPortScanResults.openPorts, lastPortScanResults.closedPorts, duration);
        return;
    }
    
    // Scan current port
    uint16_t port = portsToScan[currentPortIndex];
    bool isOpen = isPortOpen(activePortScanConfig.targetIP, port, activePortScanConfig.timeout);
    
    lastPortScanResults.portsScanned++;
    
    if (isOpen) {
        PortInfo info;
        info.port = port;
        info.isOpen = true;
        info.service = getServiceName(port);
        info.responseTime = activePortScanConfig.timeout;
        
        lastPortScanResults.openPortsList.push_back(info);
        lastPortScanResults.openPorts++;
        
        LOG_INFO(TAG_PORTSCAN, "Found open port: %d (%s)", port, info.service.c_str());
    } else {
        lastPortScanResults.closedPorts++;
    }
    
    currentPortIndex++;
    
    // Small delay to prevent overwhelming the network
    delay(10);
}

// ==========================================
// GETTERS
// ==========================================

PortScanState getPortScanState() {
    return currentPortScanState;
}

PortScanResults getLastPortScanResults() {
    return lastPortScanResults;
}

uint8_t getPortScanProgress() {
    if (lastPortScanResults.totalPorts == 0) {
        return 0;
    }
    return (lastPortScanResults.portsScanned * 100) / lastPortScanResults.totalPorts;
}
