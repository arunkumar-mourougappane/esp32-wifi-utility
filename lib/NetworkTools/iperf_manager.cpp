#include "iperf_manager.h"
#include "config.h"

// ==========================================
// GLOBAL VARIABLES
// ==========================================
IperfState currentIperfState = IPERF_IDLE;
IperfConfig activeConfig;
IperfResults lastResults;
WiFiServer* iperfTcpServer = nullptr;
WiFiUDP iperfUdp;
unsigned long iperfStartTime = 0;
bool iperfServerRunning = false;

// Internal variables
static WiFiClient iperfClient;
static unsigned long lastUpdateTime = 0;
static unsigned long bytesTransferred = 0;
static unsigned long packetsTransferred = 0;
static unsigned long packetsLost = 0;
static float jitterSum = 0;
static unsigned long lastPacketTime = 0;

// ==========================================
// INITIALIZATION AND CLEANUP
// ==========================================
void initializeIperf() {
  currentIperfState = IPERF_IDLE;
  iperfServerRunning = false;
  bytesTransferred = 0;
  packetsTransferred = 0;
  packetsLost = 0;
  jitterSum = 0;
  
  // Initialize default configuration
  activeConfig = getDefaultConfig();
  
  Serial.println("🔧 iPerf manager initialized");
}

void shutdownIperf() {
  stopIperfTest();
  
  if (iperfTcpServer) {
    delete iperfTcpServer;
    iperfTcpServer = nullptr;
  }
  
  iperfUdp.stop();
  currentIperfState = IPERF_IDLE;
  iperfServerRunning = false;
  
  Serial.println("🔧 iPerf manager shutdown");
}

// ==========================================
// CLIENT FUNCTIONS
// ==========================================
bool startIperfClient(const IperfConfig& config) {
  if (currentIperfState != IPERF_IDLE) {
    Serial.println("❌ iPerf test already running. Stop current test first.");
    return false;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Not connected to WiFi. Connect first.");
    return false;
  }
  
  activeConfig = config;
  currentIperfState = IPERF_RUNNING;
  iperfStartTime = millis();
  bytesTransferred = 0;
  packetsTransferred = 0;
  packetsLost = 0;
  jitterSum = 0;
  
  Serial.println("🚀 Starting iPerf client test...");
  printIperfConfig(config);
  
  if (config.protocol == IPERF_TCP) {
    runIperfTcpClient(config);
  } else {
    runIperfUdpClient(config);
  }
  
  return true;
}

void runIperfTcpClient(const IperfConfig& config) {
  Serial.print("🔗 Connecting to TCP server ");
  Serial.print(config.serverIP);
  Serial.print(":");
  Serial.println(config.port);
  
  if (!iperfClient.connect(config.serverIP.c_str(), config.port)) {
    Serial.println("❌ Failed to connect to server");
    currentIperfState = IPERF_IDLE;
    lastResults.testCompleted = false;
    lastResults.errorMessage = "Connection failed";
    return;
  }
  
  Serial.println("✅ Connected to server");
  
  uint8_t buffer[IPERF_BUFFER_SIZE];
  memset(buffer, 0xAA, sizeof(buffer)); // Fill with test pattern
  
  unsigned long startTime = millis();
  unsigned long endTime = startTime + (config.duration * 1000);
  unsigned long lastIntervalTime = startTime;
  unsigned long intervalBytes = 0;
  
  while (millis() < endTime && currentIperfState == IPERF_RUNNING) {
    size_t written = iperfClient.write(buffer, config.bufferSize);
    if (written > 0) {
      bytesTransferred += written;
      intervalBytes += written;
    } else {
      delay(1); // Prevent tight loop if write fails
    }
    
    // Print interval statistics
    if (millis() - lastIntervalTime >= (config.interval * 1000)) {
      float intervalThroughput = (intervalBytes * 8.0) / (1024.0 * 1024.0 * config.interval);
      Serial.print("📊 Interval: ");
      Serial.print(intervalBytes);
      Serial.print(" bytes, ");
      Serial.print(intervalThroughput, 2);
      Serial.println(" Mbps");
      
      intervalBytes = 0;
      lastIntervalTime = millis();
    }
    
    // Handle client disconnection
    if (!iperfClient.connected() && written == 0) {
      Serial.println("⚠️ Server disconnected");
      break;
    }
  }
  
  iperfClient.stop();
  
  // Calculate final results
  unsigned long actualDuration = millis() - startTime;
  lastResults.bytesTransferred = bytesTransferred;
  lastResults.durationMs = actualDuration;
  lastResults.throughputMbps = (bytesTransferred * 8.0) / (1024.0 * 1024.0 * (actualDuration / 1000.0));
  lastResults.throughputKbps = lastResults.throughputMbps * 1024.0;
  lastResults.testCompleted = true;
  lastResults.errorMessage = "";
  
  currentIperfState = IPERF_IDLE;
  printIperfResults(lastResults);
}

void runIperfUdpClient(const IperfConfig& config) {
  Serial.print("📡 Starting UDP client to ");
  Serial.print(config.serverIP);
  Serial.print(":");
  Serial.println(config.port);
  
  if (!iperfUdp.begin(config.port + 1)) { // Use different port for client
    Serial.println("❌ Failed to initialize UDP");
    currentIperfState = IPERF_IDLE;
    lastResults.testCompleted = false;
    lastResults.errorMessage = "UDP initialization failed";
    return;
  }
  
  uint8_t buffer[IPERF_BUFFER_SIZE];
  memset(buffer, 0xBB, sizeof(buffer)); // Fill with test pattern
  
  unsigned long startTime = millis();
  unsigned long endTime = startTime + (config.duration * 1000);
  unsigned long lastIntervalTime = startTime;
  unsigned long intervalBytes = 0;
  unsigned long packetDelay = 0;
  
  // Calculate packet delay based on bandwidth
  if (config.bandwidth > 0) {
    packetDelay = (config.bufferSize * 8 * 1000) / config.bandwidth; // ms between packets
  }
  
  unsigned long lastPacketTime = millis();
  
  while (millis() < endTime && currentIperfState == IPERF_RUNNING) {
    if (packetDelay == 0 || (millis() - lastPacketTime >= packetDelay)) {
      // Add sequence number to packet
      uint32_t seqNum = packetsTransferred;
      memcpy(buffer, &seqNum, sizeof(seqNum));
      
      iperfUdp.beginPacket(config.serverIP.c_str(), config.port);
      size_t written = iperfUdp.write(buffer, config.bufferSize);
      iperfUdp.endPacket();
      
      if (written > 0) {
        bytesTransferred += written;
        intervalBytes += written;
        packetsTransferred++;
      }
      
      lastPacketTime = millis();
    }
    
    // Print interval statistics
    if (millis() - lastIntervalTime >= (config.interval * 1000)) {
      float intervalThroughput = (intervalBytes * 8.0) / (1024.0 * 1024.0 * config.interval);
      Serial.print("📊 Interval: ");
      Serial.print(intervalBytes);
      Serial.print(" bytes, ");
      Serial.print(intervalThroughput, 2);
      Serial.print(" Mbps, ");
      Serial.print(packetsTransferred);
      Serial.println(" packets");
      
      intervalBytes = 0;
      lastIntervalTime = millis();
    }
    
    delay(1); // Small delay to prevent overwhelming
  }
  
  iperfUdp.stop();
  
  // Calculate final results
  unsigned long actualDuration = millis() - startTime;
  lastResults.bytesTransferred = bytesTransferred;
  lastResults.durationMs = actualDuration;
  lastResults.throughputMbps = (bytesTransferred * 8.0) / (1024.0 * 1024.0 * (actualDuration / 1000.0));
  lastResults.throughputKbps = lastResults.throughputMbps * 1024.0;
  lastResults.totalPackets = packetsTransferred;
  lastResults.packetsLost = 0; // Client can't easily detect packet loss
  lastResults.testCompleted = true;
  lastResults.errorMessage = "";
  
  currentIperfState = IPERF_IDLE;
  printIperfResults(lastResults);
}

// ==========================================
// SERVER FUNCTIONS
// ==========================================
bool startIperfServer(const IperfConfig& config) {
  if (currentIperfState != IPERF_IDLE) {
    Serial.println("❌ iPerf test already running. Stop current test first.");
    return false;
  }
  
  if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
    Serial.println("❌ WiFi not active. Enable WiFi first.");
    return false;
  }
  
  activeConfig = config;
  
  if (config.protocol == IPERF_TCP) {
    if (iperfTcpServer) {
      delete iperfTcpServer;
    }
    iperfTcpServer = new WiFiServer(config.port);
    iperfTcpServer->begin();
    Serial.print("🏁 TCP server listening on port ");
    Serial.println(config.port);
  } else {
    if (!iperfUdp.begin(config.port)) {
      Serial.println("❌ Failed to start UDP server");
      return false;
    }
    Serial.print("🏁 UDP server listening on port ");
    Serial.println(config.port);
  }
  
  currentIperfState = IPERF_RUNNING;
  iperfServerRunning = true;
  bytesTransferred = 0;
  packetsTransferred = 0;
  packetsLost = 0;
  
  Serial.println("⏳ Waiting for client connections...");
  return true;
}

void runIperfTcpServer(const IperfConfig& config) {
  if (!iperfTcpServer || !iperfServerRunning) return;
  
  WiFiClient client = iperfTcpServer->available();
  if (client) {
    Serial.println("🔗 Client connected to TCP server");
    
    iperfStartTime = millis();
    unsigned long lastIntervalTime = iperfStartTime;
    unsigned long intervalBytes = 0;
    uint8_t buffer[IPERF_BUFFER_SIZE];
    
    while (client.connected() && iperfServerRunning) {
      if (client.available()) {
        int bytesRead = client.read(buffer, sizeof(buffer));
        if (bytesRead > 0) {
          bytesTransferred += bytesRead;
          intervalBytes += bytesRead;
        }
      }
      
      // Print interval statistics
      if (millis() - lastIntervalTime >= (config.interval * 1000)) {
        float intervalThroughput = (intervalBytes * 8.0) / (1024.0 * 1024.0 * config.interval);
        Serial.print("📊 Received: ");
        Serial.print(intervalBytes);
        Serial.print(" bytes, ");
        Serial.print(intervalThroughput, 2);
        Serial.println(" Mbps");
        
        intervalBytes = 0;
        lastIntervalTime = millis();
      }
      
      delay(1);
    }
    
    client.stop();
    Serial.println("🔚 Client disconnected");
    
    // Calculate results
    unsigned long actualDuration = millis() - iperfStartTime;
    lastResults.bytesTransferred = bytesTransferred;
    lastResults.durationMs = actualDuration;
    lastResults.throughputMbps = (bytesTransferred * 8.0) / (1024.0 * 1024.0 * (actualDuration / 1000.0));
    lastResults.throughputKbps = lastResults.throughputMbps * 1024.0;
    lastResults.testCompleted = true;
    lastResults.errorMessage = "";
    
    printIperfResults(lastResults);
  }
}

void runIperfUdpServer(const IperfConfig& config) {
  if (!iperfServerRunning) return;
  
  int packetSize = iperfUdp.parsePacket();
  if (packetSize > 0) {
    // First packet starts the timer
    if (bytesTransferred == 0) {
      iperfStartTime = millis();
      Serial.println("📡 First UDP packet received, starting measurement");
    }
    
    uint8_t buffer[IPERF_BUFFER_SIZE];
    int bytesRead = iperfUdp.read(buffer, sizeof(buffer));
    
    if (bytesRead > 0) {
      bytesTransferred += bytesRead;
      packetsTransferred++;
      
      // Extract sequence number for loss detection
      if (bytesRead >= sizeof(uint32_t)) {
        uint32_t seqNum;
        memcpy(&seqNum, buffer, sizeof(seqNum));
        // Simple loss detection (gaps in sequence)
        static uint32_t expectedSeq = 0;
        if (seqNum > expectedSeq) {
          packetsLost += (seqNum - expectedSeq);
        }
        expectedSeq = seqNum + 1;
      }
    }
    
    // Print periodic updates
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= (config.interval * 1000)) {
      unsigned long elapsed = millis() - iperfStartTime;
      float throughput = (bytesTransferred * 8.0) / (1024.0 * 1024.0 * (elapsed / 1000.0));
      Serial.print("📊 Received: ");
      Serial.print(packetsTransferred);
      Serial.print(" packets, ");
      Serial.print(bytesTransferred);
      Serial.print(" bytes, ");
      Serial.print(throughput, 2);
      Serial.println(" Mbps");
      
      lastPrint = millis();
    }
  }
}

// ==========================================
// TEST MANAGEMENT
// ==========================================
void stopIperfTest() {
  if (currentIperfState == IPERF_IDLE) {
    Serial.println("ℹ️ No iPerf test running");
    return;
  }
  
  currentIperfState = IPERF_STOPPING;
  iperfServerRunning = false;
  
  if (iperfClient && iperfClient.connected()) {
    iperfClient.stop();
  }
  
  if (iperfTcpServer) {
    iperfTcpServer->stop();
  }
  
  iperfUdp.stop();
  
  currentIperfState = IPERF_IDLE;
  Serial.println("🛑 iPerf test stopped");
}

bool isIperfRunning() {
  return currentIperfState == IPERF_RUNNING;
}

IperfResults getIperfResults() {
  return lastResults;
}

void updateIperfStatus() {
  if (currentIperfState == IPERF_RUNNING && iperfServerRunning) {
    if (activeConfig.protocol == IPERF_TCP) {
      runIperfTcpServer(activeConfig);
    } else {
      runIperfUdpServer(activeConfig);
    }
  }
}

// ==========================================
// UTILITY FUNCTIONS
// ==========================================
void printIperfResults(const IperfResults& results) {
  Serial.println("\n📊 IPERF TEST RESULTS:");
  Serial.println("═══════════════════════");
  
  if (!results.testCompleted) {
    Serial.print("❌ Test failed: ");
    Serial.println(results.errorMessage);
    return;
  }
  
  Serial.print("📦 Bytes transferred: ");
  Serial.println(formatBytes(results.bytesTransferred));
  
  Serial.print("⏱️ Duration: ");
  Serial.print(results.durationMs / 1000.0, 2);
  Serial.println(" seconds");
  
  Serial.print("🚀 Throughput: ");
  Serial.println(formatThroughput(results.throughputMbps));
  
  if (results.totalPackets > 0) {
    Serial.print("📊 Packets: ");
    Serial.print(results.totalPackets);
    Serial.print(" total, ");
    Serial.print(results.packetsLost);
    Serial.print(" lost (");
    Serial.print((results.packetsLost * 100.0) / results.totalPackets, 2);
    Serial.println("%)");
    
    if (results.jitterMs > 0) {
      Serial.print("📈 Jitter: ");
      Serial.print(results.jitterMs, 2);
      Serial.println(" ms");
    }
  }
  
  Serial.println("═══════════════════════\n");
}

void printIperfConfig(const IperfConfig& config) {
  Serial.println("⚙️ Test Configuration:");
  Serial.print("   Protocol: ");
  Serial.println(config.protocol == IPERF_TCP ? "TCP" : "UDP");
  Serial.print("   Mode: ");
  Serial.println(config.mode == IPERF_CLIENT ? "Client" : "Server");
  if (config.mode == IPERF_CLIENT) {
    Serial.print("   Server: ");
    Serial.print(config.serverIP);
    Serial.print(":");
    Serial.println(config.port);
  } else {
    Serial.print("   Port: ");
    Serial.println(config.port);
  }
  Serial.print("   Duration: ");
  Serial.print(config.duration);
  Serial.println(" seconds");
  if (config.protocol == IPERF_UDP && config.bandwidth > 0) {
    Serial.print("   Bandwidth: ");
    Serial.print(config.bandwidth / 1000000.0, 1);
    Serial.println(" Mbps");
  }
  Serial.println();
}

String formatThroughput(float mbps) {
  if (mbps >= 1000.0) {
    return String(mbps / 1000.0, 2) + " Gbps";
  } else if (mbps >= 1.0) {
    return String(mbps, 2) + " Mbps";
  } else {
    return String(mbps * 1000.0, 1) + " Kbps";
  }
}

String formatBytes(unsigned long bytes) {
  if (bytes >= 1073741824) { // 1GB
    return String(bytes / 1073741824.0, 2) + " GB";
  } else if (bytes >= 1048576) { // 1MB
    return String(bytes / 1048576.0, 2) + " MB";
  } else if (bytes >= 1024) { // 1KB
    return String(bytes / 1024.0, 1) + " KB";
  } else {
    return String(bytes) + " bytes";
  }
}

IperfConfig getDefaultConfig() {
  IperfConfig config;
  config.protocol = IPERF_TCP;
  config.mode = IPERF_CLIENT;
  config.serverIP = "";
  config.port = IPERF_DEFAULT_PORT;
  config.duration = IPERF_DEFAULT_DURATION;
  config.interval = IPERF_DEFAULT_INTERVAL;
  config.bandwidth = 1000000; // 1 Mbps for UDP
  config.bufferSize = IPERF_BUFFER_SIZE;
  config.reverse = false;
  config.bidir = false;
  config.parallel = 1;
  return config;
}

// ==========================================
// COMMAND INTERFACE FUNCTIONS
// ==========================================
void executeIperfCommand(const String& command) {
  String cmd = command;
  cmd.trim();
  cmd.toLowerCase();
  
  if (cmd == "iperf help") {
    printIperfHelp();
  }
  else if (cmd == "iperf status") {
    printIperfStatus();
  }
  else if (cmd == "iperf stop") {
    stopIperfTest();
  }
  else if (cmd.startsWith("iperf server tcp")) {
    String params = cmd.substring(16);
    params.trim();
    
    IperfConfig config = getDefaultConfig();
    config.protocol = IPERF_TCP;
    config.mode = IPERF_SERVER;
    
    if (params.length() > 0) {
      config.port = params.toInt();
      if (config.port < 1024 || config.port > 65535) {
        config.port = IPERF_DEFAULT_PORT;
      }
    }
    
    startIperfServer(config);
  }
  else if (cmd.startsWith("iperf server udp")) {
    String params = cmd.substring(16);
    params.trim();
    
    IperfConfig config = getDefaultConfig();
    config.protocol = IPERF_UDP;
    config.mode = IPERF_SERVER;
    
    if (params.length() > 0) {
      config.port = params.toInt();
      if (config.port < 1024 || config.port > 65535) {
        config.port = IPERF_DEFAULT_PORT;
      }
    }
    
    startIperfServer(config);
  }
  else if (cmd.startsWith("iperf client tcp ")) {
    String params = cmd.substring(17);
    params.trim();
    
    int spaceIndex = params.indexOf(' ');
    if (spaceIndex == -1) {
      Serial.println("❌ Usage: iperf client tcp <server_ip> [port] [duration]");
      return;
    }
    
    IperfConfig config = getDefaultConfig();
    config.protocol = IPERF_TCP;
    config.mode = IPERF_CLIENT;
    config.serverIP = params.substring(0, spaceIndex);
    
    String remaining = params.substring(spaceIndex + 1);
    remaining.trim();
    
    if (remaining.length() > 0) {
      int nextSpace = remaining.indexOf(' ');
      if (nextSpace == -1) {
        config.port = remaining.toInt();
        if (config.port < 1024 || config.port > 65535) {
          config.port = IPERF_DEFAULT_PORT;
        }
      } else {
        config.port = remaining.substring(0, nextSpace).toInt();
        if (config.port < 1024 || config.port > 65535) {
          config.port = IPERF_DEFAULT_PORT;
        }
        
        String durationStr = remaining.substring(nextSpace + 1);
        durationStr.trim();
        config.duration = durationStr.toInt();
        if (config.duration < 1 || config.duration > 3600) {
          config.duration = IPERF_DEFAULT_DURATION;
        }
      }
    }
    
    startIperfClient(config);
  }
  else if (cmd.startsWith("iperf client udp ")) {
    String params = cmd.substring(17);
    params.trim();
    
    int spaceIndex = params.indexOf(' ');
    if (spaceIndex == -1) {
      Serial.println("❌ Usage: iperf client udp <server_ip> [port] [duration] [bandwidth_mbps]");
      return;
    }
    
    IperfConfig config = getDefaultConfig();
    config.protocol = IPERF_UDP;
    config.mode = IPERF_CLIENT;
    config.serverIP = params.substring(0, spaceIndex);
    
    String remaining = params.substring(spaceIndex + 1);
    remaining.trim();
    
    // Parse port, duration, and bandwidth
    int paramCount = 0;
    String paramValues[3];
    
    while (remaining.length() > 0 && paramCount < 3) {
      int nextSpace = remaining.indexOf(' ');
      if (nextSpace == -1) {
        paramValues[paramCount] = remaining;
        remaining = "";
      } else {
        paramValues[paramCount] = remaining.substring(0, nextSpace);
        remaining = remaining.substring(nextSpace + 1);
        remaining.trim();
      }
      paramCount++;
    }
    
    // Apply parameters
    if (paramCount >= 1 && paramValues[0].length() > 0) {
      config.port = paramValues[0].toInt();
      if (config.port < 1024 || config.port > 65535) {
        config.port = IPERF_DEFAULT_PORT;
      }
    }
    
    if (paramCount >= 2 && paramValues[1].length() > 0) {
      config.duration = paramValues[1].toInt();
      if (config.duration < 1 || config.duration > 3600) {
        config.duration = IPERF_DEFAULT_DURATION;
      }
    }
    
    if (paramCount >= 3 && paramValues[2].length() > 0) {
      float bandwidthMbps = paramValues[2].toFloat();
      if (bandwidthMbps > 0 && bandwidthMbps <= 1000) {
        config.bandwidth = (int)(bandwidthMbps * 1000000); // Convert to bps
      }
    }
    
    startIperfClient(config);
  }
  else {
    Serial.println("❌ Unknown iPerf command. Type 'iperf help' for available commands.");
  }
}

void printIperfHelp() {
  Serial.println("📋 IPERF COMMANDS:");
  Serial.println("┌───────────────────────────────┬────────────────────────────────────┐");
  Serial.println("│ Command                       │ Description                        │");
  Serial.println("├───────────────────────────────┼────────────────────────────────────┤");
  Serial.println("│ iperf help                    │ Show iPerf help                    │");
  Serial.println("│ iperf status                  │ Show current iPerf status          │");
  Serial.println("│ iperf stop                    │ Stop running test                  │");
  Serial.println("│ iperf server tcp [port]       │ Start TCP server (def: 5201)       │");
  Serial.println("│ iperf server udp [port]       │ Start UDP server (def: 5201)       │");
  Serial.println("│ iperf client tcp <ip> [p] [d] │ TCP client test                    │");
  Serial.println("│ iperf client udp <ip> [p] [d] │ UDP client test                    │");
  Serial.println("└───────────────────────────────┴────────────────────────────────────┘");
  Serial.println();
  Serial.println("Parameters:");
  Serial.println("  <ip> = Server IP address (required)");
  Serial.println("  [p]  = Port number (default: 5201)");
  Serial.println("  [d]  = Duration in seconds (default: 10)");
  Serial.println("  [b]  = Bandwidth in Mbps for UDP (default: 1)");
  Serial.println();
  Serial.println("Examples:");
  Serial.println("  iperf server tcp 5201");
  Serial.println("  iperf client tcp 192.168.1.100 5201 30");
  Serial.println("  iperf client udp 192.168.1.100 5201 10 5");
  Serial.println();
}

void printIperfStatus() {
  Serial.println("\n📊 IPERF STATUS:");
  Serial.println("──────────────────");
  
  Serial.print("State: ");
  switch (currentIperfState) {
    case IPERF_IDLE:
      Serial.println("🟡 Idle");
      break;
    case IPERF_RUNNING:
      Serial.println("🟢 Running");
      break;
    case IPERF_STOPPING:
      Serial.println("🟠 Stopping");
      break;
  }
  
  if (currentIperfState == IPERF_RUNNING) {
    Serial.print("Mode: ");
    Serial.print(activeConfig.mode == IPERF_CLIENT ? "Client" : "Server");
    Serial.print(" (");
    Serial.print(activeConfig.protocol == IPERF_TCP ? "TCP" : "UDP");
    Serial.println(")");
    
    if (activeConfig.mode == IPERF_CLIENT) {
      Serial.print("Server: ");
      Serial.print(activeConfig.serverIP);
      Serial.print(":");
      Serial.println(activeConfig.port);
    } else {
      Serial.print("Listening Port: ");
      Serial.println(activeConfig.port);
    }
    
    if (iperfStartTime > 0) {
      unsigned long elapsed = (millis() - iperfStartTime) / 1000;
      Serial.print("Elapsed: ");
      Serial.print(elapsed);
      Serial.println(" seconds");
      
      if (bytesTransferred > 0) {
        float currentThroughput = (bytesTransferred * 8.0) / (1024.0 * 1024.0 * elapsed);
        Serial.print("Current: ");
        Serial.print(formatBytes(bytesTransferred));
        Serial.print(", ");
        Serial.println(formatThroughput(currentThroughput));
      }
    }
  }
  
  if (lastResults.testCompleted) {
    Serial.println("\nLast Test Results:");
    Serial.print("  ");
    Serial.print(formatBytes(lastResults.bytesTransferred));
    Serial.print(" in ");
    Serial.print(lastResults.durationMs / 1000.0, 1);
    Serial.print("s = ");
    Serial.println(formatThroughput(lastResults.throughputMbps));
  }
  
  Serial.println();
}

// ==========================================
// BACKGROUND TASK MANAGEMENT
// ==========================================
void handleIperfTasks() {
  // Update server status if running
  updateIperfStatus();
  
  // Check for test timeout (safety mechanism)
  if (currentIperfState == IPERF_RUNNING && iperfStartTime > 0) {
    unsigned long elapsed = millis() - iperfStartTime;
    // Timeout after 2x the configured duration + 30 seconds buffer
    unsigned long timeout = (activeConfig.duration * 2 + 30) * 1000;
    
    if (elapsed > timeout) {
      Serial.println("⚠️ iPerf test timeout - stopping");
      stopIperfTest();
    }
  }
}