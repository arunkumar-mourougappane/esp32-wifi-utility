/**
 * @file iperf_manager.h
 * @brief iPerf network performance testing interface
 * 
 * This header defines structures and functions for iPerf-compatible network
 * performance testing. Supports both TCP and UDP protocols in client/server
 * modes with configurable bandwidth, duration, and parallel streams.
 * Compatible with standard iPerf2/iPerf3 tools.
 * 
 * @author Arunkumar Mourougappane
 * @version 3.0.0
 * @date 2026-01-17
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

// ==========================================
// IPERF CONFIGURATION CONSTANTS
// ==========================================
#define IPERF_DEFAULT_PORT 5201
#define IPERF_BUFFER_SIZE 1024
#define IPERF_DEFAULT_DURATION 10
#define IPERF_DEFAULT_INTERVAL 1
#define IPERF_MAX_PARALLEL_STREAMS 4

// ==========================================
// IPERF TEST TYPES
// ==========================================
enum IperfProtocol {
  IPERF_TCP = 0,
  IPERF_UDP = 1
};

enum IperfMode {
  IPERF_CLIENT = 0,
  IPERF_SERVER = 1
};

// ==========================================
// IPERF TEST CONFIGURATION
// ==========================================
struct IperfConfig {
  IperfProtocol protocol;
  IperfMode mode;
  String serverIP;
  int port;
  int duration;
  int interval;
  int bandwidth;  // For UDP tests (bits per second)
  int bufferSize;
  bool reverse;   // Server sends, client receives
  bool bidir;     // Bidirectional test
  int parallel;   // Number of parallel streams
};

// ==========================================
// IPERF TEST RESULTS
// ==========================================
struct IperfResults {
  unsigned long bytesTransferred;
  unsigned long durationMs;
  float throughputMbps;
  float throughputKbps;
  int packetsLost;     // UDP only
  int totalPackets;    // UDP only
  float jitterMs;      // UDP only
  bool testCompleted;
  String errorMessage;
};

// ==========================================
// IPERF MANAGER STATE
// ==========================================
enum IperfState {
  IPERF_IDLE = 0,
  IPERF_RUNNING = 1,
  IPERF_STOPPING = 2
};

// ==========================================
// GLOBAL VARIABLES
// ==========================================
extern IperfState currentIperfState;
extern IperfConfig activeConfig;
extern IperfResults lastResults;
extern WiFiServer* iperfTcpServer;
extern WiFiUDP iperfUdp;
extern unsigned long iperfStartTime;
extern bool iperfServerRunning;

// ==========================================
// IPERF CORE FUNCTIONS
// ==========================================
void initializeIperf();
void shutdownIperf();

// Client functions
bool startIperfClient(const IperfConfig& config);
void runIperfTcpClient(const IperfConfig& config);
void runIperfUdpClient(const IperfConfig& config);

// Server functions
bool startIperfServer(const IperfConfig& config);
void runIperfTcpServer(const IperfConfig& config);
void runIperfUdpServer(const IperfConfig& config);

// Test management
void stopIperfTest();
bool isIperfRunning();
IperfResults getIperfResults();
void updateIperfStatus();

// Utility functions
void printIperfResults(const IperfResults& results);
void printIperfConfig(const IperfConfig& config);
String formatThroughput(float mbps);
String formatBytes(unsigned long bytes);
IperfConfig getDefaultConfig();

// Command interface functions
void executeIperfCommand(const String& command);
void printIperfHelp();
void printIperfStatus();

// Background task management
void handleIperfTasks();