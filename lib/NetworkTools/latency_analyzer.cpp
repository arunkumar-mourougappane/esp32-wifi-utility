#include "latency_analyzer.h"
#include "config.h"
#ifdef USE_NEOPIXEL
#include "led_controller.h"
#endif
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <AsyncUDP.h>
#include <WiFi.h>

// ==========================================
// GLOBAL VARIABLES
// ==========================================
LatencyTestState currentLatencyState = LATENCY_IDLE;
LatencyConfig activeLatencyConfig;
LatencyTestResults lastLatencyResults;

// Internal test variables
static WiFiUDP latencyUdp;
static AsyncUDP asyncUdp;
static unsigned long testStartTime = 0;
static unsigned long lastPingTime = 0;
static uint16_t currentSequence = 0;
static float latencyBuffer[JITTER_BUFFER_SIZE];
static uint8_t bufferIndex = 0;
static bool bufferFull = false;

// Running statistics
static JitterStats runningStats;

// ==========================================
// INITIALIZATION AND CLEANUP
// ==========================================
void initializeLatencyAnalysis() {
  currentLatencyState = LATENCY_IDLE;
  memset(&lastLatencyResults, 0, sizeof(LatencyTestResults));
  memset(&runningStats, 0, sizeof(JitterStats));
  
  // Initialize latency buffer
  memset(latencyBuffer, 0, sizeof(latencyBuffer));
  bufferIndex = 0;
  bufferFull = false;
  
  Serial.println("🔧 Latency Analysis system initialized");
}

void shutdownLatencyAnalysis() {
  stopLatencyTest();
  latencyUdp.stop();
  asyncUdp.close();
  currentLatencyState = LATENCY_IDLE;
  
  Serial.println("🔧 Latency Analysis system shutdown");
}

// ==========================================
// TEST EXECUTION FUNCTIONS
// ==========================================
bool startLatencyTest(const LatencyConfig& config) {
  if (currentLatencyState == LATENCY_RUNNING) {
    Serial.println("❌ Latency test already running. Stop current test first.");
    return false;
  }
  
  // Reset state if previous test was completed or had an error
  if (currentLatencyState == LATENCY_COMPLETED || currentLatencyState == LATENCY_ERROR) {
    currentLatencyState = LATENCY_IDLE;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Not connected to WiFi. Connect to network first.");
    return false;
  }
  
  if (!validateLatencyConfig(config)) {
    Serial.println("❌ Invalid latency test configuration");
    return false;
  }
  
  // Ensure clean state - stop any existing UDP connections
  latencyUdp.stop();
  asyncUdp.close();
  
  activeLatencyConfig = config;
  currentLatencyState = LATENCY_RUNNING;
  testStartTime = millis();
  currentSequence = 0;
  lastPingTime = 0;
  
  // Initialize results structure
  memset(&lastLatencyResults, 0, sizeof(LatencyTestResults));
  lastLatencyResults.state = LATENCY_RUNNING;
  
  // Initialize running statistics
  runningStats.min_latency_ms = 999999;
  runningStats.max_latency_ms = 0;
  runningStats.avg_latency_ms = 0;
  runningStats.jitter_ms = 0;
  runningStats.max_jitter_ms = 0;
  runningStats.packets_sent = 0;
  runningStats.packets_received = 0;
  runningStats.packets_lost = 0;
  
  Serial.println("🚀 Starting Latency & Jitter Analysis...");
  printLatencyConfig(config);
  
#ifdef USE_NEOPIXEL
  // Show cyan color during latency testing
  setNeoPixelColor(0, 255, 255);
#endif
  
  bool result = false;
  switch (config.test_type) {
    case LATENCY_UDP_ECHO:
      result = executeUdpEchoTest(config);
      break;
    case LATENCY_TCP_CONNECT:
      result = executeTcpConnectTest(config);
      break;
    case LATENCY_HTTP_REQUEST:
      result = executeHttpLatencyTest(config);
      break;
    default:
      Serial.println("❌ Unsupported test type");
      currentLatencyState = LATENCY_IDLE;
      return false;
  }
  
  if (!result) {
    currentLatencyState = LATENCY_ERROR;
    lastLatencyResults.state = LATENCY_ERROR;
    lastLatencyResults.error_message = "Failed to start test";
    Serial.println("❌ Error: Failed to start latency test. Check configuration and network connection.");
  }
  
  return result;
}

bool executeUdpEchoTest(const LatencyConfig& config) {
  Serial.printf("🔍 Starting UDP Echo test to %s:%d\n", config.target_host.c_str(), config.target_port);
  
  // Ensure UDP is properly stopped first
  latencyUdp.stop();
  delay(100);  // Small delay to ensure socket is released
  
  if (!latencyUdp.begin(0)) {  // Use any available port
    Serial.println("❌ Failed to initialize UDP socket");
    return false;
  }
  
  Serial.println("✅ UDP Echo test initialized");
  return true;
}

bool executeTcpConnectTest(const LatencyConfig& config) {
  Serial.printf("🔍 Starting TCP Connect test to %s:%d\n", config.target_host.c_str(), config.target_port);
  Serial.println("✅ TCP Connect test initialized");
  return true;
}

bool executeHttpLatencyTest(const LatencyConfig& config) {
  Serial.printf("🔍 Starting HTTP Latency test to %s\n", config.target_host.c_str());
  Serial.println("✅ HTTP Latency test initialized");
  return true;
}

void stopLatencyTest() {
  if (currentLatencyState == LATENCY_RUNNING) {
    currentLatencyState = LATENCY_COMPLETED;
    lastLatencyResults.state = LATENCY_COMPLETED;
    lastLatencyResults.test_duration_ms = millis() - testStartTime;
    lastLatencyResults.test_completed = true;
    
    // Calculate final statistics
    lastLatencyResults.statistics = calculateJitterStats(lastLatencyResults.results, lastLatencyResults.results_count);
    
    Serial.println("⏹️ Latency test stopped");
    printLatencyResults(lastLatencyResults);
    
#ifdef USE_NEOPIXEL
    // Show green for completed test
    setNeoPixelColor(0, 255, 0);
    delay(1000);
#endif
  }
  
  // Always clean up resources
  latencyUdp.stop();
  asyncUdp.close();
  
  // Auto-reset to IDLE after showing results to allow new tests
  if (currentLatencyState == LATENCY_COMPLETED) {
    delay(500);  // Brief delay before reset
    currentLatencyState = LATENCY_IDLE;
    Serial.println("🔄 Ready for new latency test");
  }
}

// ==========================================
// BACKGROUND TASK HANDLER
// ==========================================
void handleLatencyTasks() {
  if (currentLatencyState != LATENCY_RUNNING) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // Check for test completion
  if (!activeLatencyConfig.continuous_mode && 
      runningStats.packets_sent >= activeLatencyConfig.packet_count) {
    stopLatencyTest();
    return;
  }
  
  // Check if it's time for next ping
  if (currentTime - lastPingTime >= activeLatencyConfig.interval_ms) {
    sendLatencyProbe();
    lastPingTime = currentTime;
  }
  
  // Process incoming responses
  processLatencyResponses();
  
  // Print periodic updates
  static unsigned long lastStatsUpdate = 0;
  if (currentTime - lastStatsUpdate >= 5000) {  // Every 5 seconds
    printRunningLatencyStats();
    lastStatsUpdate = currentTime;
  }
}

void sendLatencyProbe() {
  unsigned long sendTime = micros();
  
  switch (activeLatencyConfig.test_type) {
    case LATENCY_UDP_ECHO:
      sendUdpEchoProbe(sendTime);
      break;
    case LATENCY_TCP_CONNECT:
      sendTcpConnectProbe(sendTime);
      break;
    case LATENCY_HTTP_REQUEST:
      sendHttpLatencyProbe(sendTime);
      break;
  }
  
  runningStats.packets_sent++;
  currentSequence++;
}

void sendUdpEchoProbe(unsigned long sendTime) {
  // Create UDP ping packet
  char packet[64];
  snprintf(packet, sizeof(packet), "PING %lu %d", sendTime, currentSequence);
  
  latencyUdp.beginPacket(activeLatencyConfig.target_host.c_str(), activeLatencyConfig.target_port);
  latencyUdp.write((uint8_t*)packet, strlen(packet));
  latencyUdp.endPacket();
  
  Serial.printf("📤 UDP ping sent: seq=%d\n", currentSequence);
}

void sendTcpConnectProbe(unsigned long sendTime) {
  WiFiClient tcpClient;
  unsigned long startConnect = micros();
  
  bool connected = tcpClient.connect(activeLatencyConfig.target_host.c_str(), 
                                   activeLatencyConfig.target_port, 
                                   activeLatencyConfig.timeout_ms);
  
  unsigned long endConnect = micros();
  float latency = (endConnect - startConnect) / 1000.0;  // Convert to ms
  
  PingResult result;
  result.success = connected;
  result.latency_ms = latency;
  result.timestamp = millis();
  result.sequence = currentSequence;
  
  if (!connected) {
    result.errorMessage = "Connection failed";
    runningStats.packets_lost++;
  } else {
    runningStats.packets_received++;
    tcpClient.stop();
  }
  
  // Store result
  if (lastLatencyResults.results_count < PING_MAX_COUNT) {
    lastLatencyResults.results[lastLatencyResults.results_count] = result;
    lastLatencyResults.results_count++;
  }
  
  updateRunningStats(result);
  
  Serial.printf("📤 TCP connect: seq=%d, latency=%.2fms, %s\n", 
                currentSequence, latency, connected ? "SUCCESS" : "FAILED");
}

void sendHttpLatencyProbe(unsigned long sendTime) {
  HTTPClient http;
  unsigned long startRequest = micros();
  
  String url = "http://" + activeLatencyConfig.target_host;
  if (activeLatencyConfig.target_port != 80) {
    url += ":" + String(activeLatencyConfig.target_port);
  }
  
  http.begin(url);
  http.setTimeout(activeLatencyConfig.timeout_ms);
  
  int httpCode = http.GET();
  unsigned long endRequest = micros();
  float latency = (endRequest - startRequest) / 1000.0;  // Convert to ms
  
  PingResult result;
  result.success = (httpCode > 0);
  result.latency_ms = latency;
  result.timestamp = millis();
  result.sequence = currentSequence;
  
  if (httpCode < 0) {
    result.errorMessage = "HTTP request failed: " + String(httpCode);
    runningStats.packets_lost++;
  } else {
    runningStats.packets_received++;
  }
  
  http.end();
  
  // Store result
  if (lastLatencyResults.results_count < PING_MAX_COUNT) {
    lastLatencyResults.results[lastLatencyResults.results_count] = result;
    lastLatencyResults.results_count++;
  }
  
  updateRunningStats(result);
  
  Serial.printf("📤 HTTP request: seq=%d, latency=%.2fms, code=%d\n", 
                currentSequence, latency, httpCode);
}

void processLatencyResponses() {
  // For UDP echo test, check for incoming responses
  if (activeLatencyConfig.test_type == LATENCY_UDP_ECHO) {
    int packetSize = latencyUdp.parsePacket();
    if (packetSize > 0) {
      char buffer[64];
      int len = latencyUdp.read(buffer, sizeof(buffer) - 1);
      buffer[len] = 0;
      
      // Parse response
      unsigned long sendTime;
      int sequence;
      if (sscanf(buffer, "PONG %lu %d", &sendTime, &sequence) == 2) {
        unsigned long receiveTime = micros();
        float latency = (receiveTime - sendTime) / 1000.0;  // Convert to ms
        
        PingResult result;
        result.success = true;
        result.latency_ms = latency;
        result.timestamp = millis();
        result.sequence = sequence;
        
        // Store result
        if (lastLatencyResults.results_count < PING_MAX_COUNT) {
          lastLatencyResults.results[lastLatencyResults.results_count] = result;
          lastLatencyResults.results_count++;
        }
        
        updateRunningStats(result);
        runningStats.packets_received++;
        
        Serial.printf("📥 UDP pong received: seq=%d, latency=%.2fms\n", sequence, latency);
      }
    }
  }
}

// ==========================================
// STATISTICS CALCULATION
// ==========================================
JitterStats calculateJitterStats(PingResult* results, uint16_t count) {
  JitterStats stats;
  memset(&stats, 0, sizeof(JitterStats));
  
  if (count == 0) return stats;
  
  stats.min_latency_ms = 999999;
  stats.max_latency_ms = 0;
  float total_latency = 0;
  uint32_t successful_pings = 0;
  
  // Calculate basic statistics
  for (uint16_t i = 0; i < count; i++) {
    if (results[i].success) {
      float latency = results[i].latency_ms;
      
      if (latency < stats.min_latency_ms) stats.min_latency_ms = latency;
      if (latency > stats.max_latency_ms) stats.max_latency_ms = latency;
      total_latency += latency;
      successful_pings++;
    }
  }
  
  stats.packets_sent = count;
  stats.packets_received = successful_pings;
  stats.packets_lost = count - successful_pings;
  stats.packet_loss_percent = (float)stats.packets_lost / count * 100.0;
  
  if (successful_pings > 0) {
    stats.avg_latency_ms = total_latency / successful_pings;
    
    // Calculate jitter (average of absolute differences between consecutive latencies)
    float jitter_sum = 0;
    uint32_t jitter_count = 0;
    float last_latency = -1;
    
    for (uint16_t i = 0; i < count; i++) {
      if (results[i].success) {
        if (last_latency >= 0) {
          float diff = fabs(results[i].latency_ms - last_latency);
          jitter_sum += diff;
          if (diff > stats.max_jitter_ms) stats.max_jitter_ms = diff;
          jitter_count++;
        }
        last_latency = results[i].latency_ms;
      }
    }
    
    if (jitter_count > 0) {
      stats.jitter_ms = jitter_sum / jitter_count;
    }
  }
  
  return stats;
}

void updateRunningStats(const PingResult& result) {
  if (!result.success) return;
  
  // Update latency buffer for jitter calculation
  latencyBuffer[bufferIndex] = result.latency_ms;
  bufferIndex = (bufferIndex + 1) % JITTER_BUFFER_SIZE;
  if (bufferIndex == 0) bufferFull = true;
  
  // Update running statistics
  if (result.latency_ms < runningStats.min_latency_ms) {
    runningStats.min_latency_ms = result.latency_ms;
  }
  if (result.latency_ms > runningStats.max_latency_ms) {
    runningStats.max_latency_ms = result.latency_ms;
  }
  
  // Calculate running average
  float total = 0;
  uint8_t count = bufferFull ? JITTER_BUFFER_SIZE : bufferIndex;
  for (uint8_t i = 0; i < count; i++) {
    total += latencyBuffer[i];
  }
  runningStats.avg_latency_ms = total / count;
  
  // Calculate running jitter
  if (count > 1) {
    float jitter_sum = 0;
    for (uint8_t i = 1; i < count; i++) {
      uint8_t prev_idx = (bufferIndex - count + i - 1 + JITTER_BUFFER_SIZE) % JITTER_BUFFER_SIZE;
      uint8_t curr_idx = (bufferIndex - count + i + JITTER_BUFFER_SIZE) % JITTER_BUFFER_SIZE;
      float diff = fabs(latencyBuffer[curr_idx] - latencyBuffer[prev_idx]);
      jitter_sum += diff;
      if (diff > runningStats.max_jitter_ms) runningStats.max_jitter_ms = diff;
    }
    runningStats.jitter_ms = jitter_sum / (count - 1);
  }
}

// ==========================================
// UTILITY AND DISPLAY FUNCTIONS
// ==========================================
LatencyConfig getDefaultLatencyConfig(LatencyTestType test_type) {
  LatencyConfig config;
  config.target_host = "8.8.8.8";  // Google DNS
  config.target_port = 53;         // DNS port
  config.test_type = test_type;
  config.packet_count = PING_DEFAULT_COUNT;
  config.packet_size = 32;
  config.interval_ms = PING_DEFAULT_INTERVAL;
  config.timeout_ms = PING_DEFAULT_TIMEOUT;
  config.continuous_mode = false;
  
  switch (test_type) {
    case LATENCY_UDP_ECHO:
      config.target_port = 7;  // Echo port
      break;
    case LATENCY_TCP_CONNECT:
      config.target_port = 80;  // HTTP port
      break;
    case LATENCY_HTTP_REQUEST:
      config.target_host = "www.google.com";
      config.target_port = 80;
      break;
  }
  
  return config;
}

bool validateLatencyConfig(const LatencyConfig& config) {
  if (config.target_host.length() == 0) return false;
  if (config.target_port == 0) return false;
  if (config.packet_count == 0 || config.packet_count > PING_MAX_COUNT) return false;
  if (config.interval_ms == 0) return false;
  if (config.timeout_ms == 0) return false;
  
  return true;
}

void printLatencyConfig(const LatencyConfig& config) {
  Serial.println("📊 === Latency Test Configuration ===");
  Serial.printf("Target: %s:%d\n", config.target_host.c_str(), config.target_port);
  Serial.printf("Test Type: %s\n", latencyTestTypeToString(config.test_type).c_str());
  Serial.printf("Packet Count: %d\n", config.packet_count);
  Serial.printf("Packet Size: %d bytes\n", config.packet_size);
  Serial.printf("Interval: %d ms\n", config.interval_ms);
  Serial.printf("Timeout: %d ms\n", config.timeout_ms);
  Serial.printf("Continuous: %s\n", config.continuous_mode ? "Yes" : "No");
  Serial.println("=====================================");
}

void printLatencyResults(const LatencyTestResults& results) {
  Serial.println("\n🎯 === Latency & Jitter Analysis Results ===");
  
  if (!results.test_completed) {
    Serial.printf("❌ Test incomplete: %s\n", results.error_message.c_str());
    return;
  }
  
  const JitterStats& stats = results.statistics;
  
  Serial.printf("📊 Test Duration: %lu ms\n", results.test_duration_ms);
  Serial.printf("📤 Packets Sent: %u\n", stats.packets_sent);
  Serial.printf("📥 Packets Received: %u\n", stats.packets_received);
  Serial.printf("📉 Packets Lost: %u (%.1f%%)\n", stats.packets_lost, stats.packet_loss_percent);
  
  if (stats.packets_received > 0) {
    Serial.printf("⚡ Min Latency: %.2f ms\n", stats.min_latency_ms);
    Serial.printf("⚡ Max Latency: %.2f ms\n", stats.max_latency_ms);
    Serial.printf("⚡ Avg Latency: %.2f ms\n", stats.avg_latency_ms);
    Serial.printf("📈 Jitter (Avg): %.2f ms\n", stats.jitter_ms);
    Serial.printf("📈 Max Jitter: %.2f ms\n", stats.max_jitter_ms);
    
    // Network quality assessment
    uint8_t quality = assessNetworkQuality("");
    Serial.printf("🌐 Network Quality: %d/100\n", quality);
  }
  
  Serial.println("==========================================\n");
}

void printRunningLatencyStats() {
  if (currentLatencyState != LATENCY_RUNNING) return;
  
  Serial.println("📊 === Running Statistics ===");
  Serial.printf("Sent: %u | Received: %u | Lost: %u (%.1f%%)\n", 
                runningStats.packets_sent, runningStats.packets_received, 
                runningStats.packets_lost, runningStats.packet_loss_percent);
  
  if (runningStats.packets_received > 0) {
    Serial.printf("Latency: %.2f/%.2f/%.2f ms (min/avg/max)\n", 
                  runningStats.min_latency_ms, runningStats.avg_latency_ms, runningStats.max_latency_ms);
    Serial.printf("Jitter: %.2f ms (avg), %.2f ms (max)\n", 
                  runningStats.jitter_ms, runningStats.max_jitter_ms);
  }
  Serial.println("==============================");
}

String latencyTestTypeToString(LatencyTestType type) {
  switch (type) {
    case LATENCY_ICMP_PING: return "ICMP Ping";
    case LATENCY_UDP_ECHO: return "UDP Echo";
    case LATENCY_TCP_CONNECT: return "TCP Connect";
    case LATENCY_HTTP_REQUEST: return "HTTP Request";
    default: return "Unknown";
  }
}

String latencyTestStateToString(LatencyTestState state) {
  switch (state) {
    case LATENCY_IDLE: return "Idle";
    case LATENCY_RUNNING: return "Running";
    case LATENCY_COMPLETED: return "Completed";
    case LATENCY_ERROR: return "Error";
    default: return "Unknown";
  }
}

String getLatencyStatus() {
  String status = "Latency Test Status: " + latencyTestStateToString(currentLatencyState);
  
  if (currentLatencyState == LATENCY_RUNNING) {
    unsigned long elapsed = (millis() - testStartTime) / 1000;
    status += " (Running " + String(elapsed) + "s)";
    status += " | Sent: " + String(runningStats.packets_sent);
    status += " | Received: " + String(runningStats.packets_received);
    
    if (runningStats.packets_received > 0) {
      status += " | Avg Latency: " + String(runningStats.avg_latency_ms, 1) + "ms";
      status += " | Jitter: " + String(runningStats.jitter_ms, 1) + "ms";
    }
  }
  
  return status;
}

uint8_t assessNetworkQuality(const String& target_host) {
  if (runningStats.packets_received == 0) return 0;
  
  uint8_t quality = 100;
  
  // Penalize based on packet loss
  quality -= (uint8_t)(runningStats.packet_loss_percent * 2);
  
  // Penalize based on latency
  if (runningStats.avg_latency_ms > 200) quality -= 30;
  else if (runningStats.avg_latency_ms > 100) quality -= 20;
  else if (runningStats.avg_latency_ms > 50) quality -= 10;
  
  // Penalize based on jitter
  if (runningStats.jitter_ms > 50) quality -= 25;
  else if (runningStats.jitter_ms > 20) quality -= 15;
  else if (runningStats.jitter_ms > 10) quality -= 5;
  
  return max(0, (int)quality);
}

JitterStats executeNetworkAnalysis(const String& target_host) {
  // Execute comprehensive analysis with multiple test types
  Serial.println("🔍 Starting Comprehensive Network Analysis...");
  
  // This would run multiple test types and aggregate results
  // For now, return the current running stats
  return runningStats;
}

// ==========================================
// GETTER FUNCTIONS FOR EXTERNAL ACCESS
// ==========================================
LatencyTestState getLatencyTestState() {
  return currentLatencyState;
}

LatencyTestResults getLastLatencyResults() {
  return lastLatencyResults;
}