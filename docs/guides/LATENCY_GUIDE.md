# 📊 Latency & Jitter Analysis Guide

This guide covers the comprehensive network latency and jitter analysis features of the ESP32 WiFi Utility.

## 🎯 Overview

The Latency & Jitter Analysis module provides advanced network performance testing capabilities including:

- **Multiple Test Types**: UDP Echo, TCP Connect, and HTTP Request testing
- **Real-time Statistics**: Min/Max/Average latency, jitter calculation, packet loss tracking  
- **Network Quality Assessment**: Overall quality scoring (0-100)
- **Background Processing**: Non-blocking test execution with periodic updates
- **Visual Feedback**: NeoPixel status indicators (Feather board only)

## 📋 Available Commands

### Basic Commands

| Command | Description |
|---------|-------------|
| `latency` | Show latency & jitter test help |
| `latency test` | Start basic UDP echo latency test |
| `latency test tcp` | Start TCP connection latency test |
| `latency test http` | Start HTTP request latency test |
| `latency test <ip>` | Test latency to specific host/IP |
| `latency stop` | Stop current latency test |
| `latency status` | Show current test status |
| `latency results` | Show last test results |
| `jitter` | Quick jitter analysis (20 packets) |
| `network analysis` | Comprehensive network quality test |

### Usage Examples

```bash
# Basic UDP echo test to Google DNS
ESP32> latency test

# TCP connection test to Google
ESP32> latency test tcp  

# HTTP response time test
ESP32> latency test http

# Custom host testing
ESP32> latency test 192.168.1.1

# Quick jitter analysis
ESP32> jitter

# Check test progress
ESP32> latency status

# View completed results
ESP32> latency results
```

## 🔧 Test Types Explained

### 1. UDP Echo Test (`latency test`)
- **Method**: Sends UDP packets and measures round-trip time
- **Default Target**: 8.8.8.8:53 (Google DNS)
- **Best For**: Basic network latency measurement
- **Packet Count**: 10 packets (default)
- **Interval**: 1000ms between packets

### 2. TCP Connect Test (`latency test tcp`)
- **Method**: Measures TCP connection establishment time
- **Default Target**: 8.8.8.8:80 (HTTP port)
- **Best For**: Testing connection overhead and firewall impact
- **Measures**: SYN → SYN-ACK → ACK handshake time

### 3. HTTP Request Test (`latency test http`)
- **Method**: Full HTTP GET request and response measurement
- **Default Target**: www.google.com:80
- **Best For**: Application-level latency including DNS resolution
- **Measures**: Complete request/response cycle

## 📈 Metrics & Statistics

### Latency Measurements
- **Minimum Latency**: Fastest round-trip time observed
- **Maximum Latency**: Slowest round-trip time observed  
- **Average Latency**: Mean latency across all successful packets
- **Jitter (Average)**: Average variation between consecutive packets
- **Maximum Jitter**: Largest variation observed between packets

### Reliability Metrics
- **Packets Sent**: Total number of test packets transmitted
- **Packets Received**: Number of successful responses received
- **Packets Lost**: Number of packets that didn't receive responses
- **Packet Loss %**: Percentage of lost packets

### Network Quality Score
The system calculates an overall network quality score (0-100) based on:
- **Packet Loss**: Heavy penalty for lost packets (2x multiplier)
- **High Latency**: Penalties for >50ms, >100ms, >200ms latency
- **High Jitter**: Penalties for >10ms, >20ms, >50ms jitter

**Quality Scoring Guidelines**:
- **90-100**: Excellent (Gaming/VoIP ready)
- **70-89**: Good (General browsing/streaming)
- **50-69**: Fair (Basic connectivity)
- **0-49**: Poor (Connection issues present)

## 🎨 Visual Status Indicators (Feather Board)

When using the Adafruit Feather ESP32-S3 TFT board with NeoPixel:

| Color | Status |
|-------|---------|
| 🔵 **Cyan** | Latency test in progress |
| 🟢 **Green** | Test completed successfully |
| 🔴 **Red** | Test error or failure |
| 🟡 **Yellow** | Warning (high latency/jitter) |

## 📊 Sample Output

```
 Starting Latency & Jitter Analysis...
📊 === Latency Test Configuration ===
Target: 8.8.8.8:7
Test Type: UDP Echo
Packet Count: 50
Packet Size: 32 bytes
Interval: 1000 ms
Timeout: 5000 ms
Continuous: No
=====================================

📤 UDP ping sent: seq=1
📥 UDP pong received: seq=1, latency=23.45ms
📤 UDP ping sent: seq=2
📥 UDP pong received: seq=2, latency=25.12ms

📊 === Running Statistics ===
Sent: 25 | Received: 25 | Lost: 0 (0.0%)
Latency: 18.34/24.67/45.23 ms (min/avg/max)
Jitter: 3.21 ms (avg), 8.45 ms (max)
==============================

🎯 === Latency & Jitter Analysis Results ===
📊 Test Duration: 51240 ms
📤 Packets Sent: 50
📥 Packets Received: 48
📉 Packets Lost: 2 (4.0%)
⚡ Min Latency: 18.34 ms
⚡ Max Latency: 67.89 ms
⚡ Avg Latency: 26.45 ms
📈 Jitter (Avg): 4.23 ms
📈 Max Jitter: 12.34 ms
🌐 Network Quality: 82/100
==========================================
```

## ⚙️ Configuration Options

### Default Test Parameters
```cpp
// UDP Echo Test Defaults
Target Host: "8.8.8.8"
Target Port: 53 (DNS)
Packet Count: 10
Packet Size: 32 bytes
Interval: 1000ms
Timeout: 5000ms

// TCP Connect Test Defaults  
Target Host: "8.8.8.8"
Target Port: 80 (HTTP)

// HTTP Request Test Defaults
Target Host: "www.google.com"
Target Port: 80
```

### Memory Usage
- **Latency Buffer**: 32 recent measurements for jitter calculation
- **Results Storage**: Up to 100 individual ping results
- **Heap Usage**: ~8KB additional RAM during testing

## 🔍 Troubleshooting

### Common Issues

**"Not connected to WiFi"**
- Ensure device is connected: `mode station` → `connect <ssid> <password>`

**"Connection failed" (TCP Test)**
- Target host may be blocking connections
- Try different target: `latency test tcp 8.8.8.8`

**High packet loss**
- Check WiFi signal strength
- Network congestion may be present
- Try different target host

**Inconsistent results**
- Network conditions may be variable
- Run multiple tests for average performance
- Use `jitter` command for quick stability check

### Debug Commands
```bash
# Check current connection
ESP32> status

# Test basic connectivity  
ESP32> latency test 8.8.8.8

# Monitor test progress
ESP32> latency status

# View detailed results
ESP32> latency results
```

## Integration with Other Features

### iPerf3 Compatibility
The latency analyzer works alongside iPerf3 testing:
- Run latency tests to assess baseline performance
- Use iPerf3 for throughput testing
- Compare results for complete network characterization

### NeoPixel Integration
On Feather boards, latency testing automatically provides visual feedback:
- Status changes are immediately visible
- No additional configuration required
- Works with existing NeoPixel features

## 📚 Technical Implementation

### Architecture
- **Non-blocking Design**: Tests run in background without freezing interface
- **Modular Structure**: Separate test engines for each protocol
- **Memory Efficient**: Circular buffers and configurable limits
- **Thread-safe**: Safe concurrent access to shared data

### Timing Precision
- **Microsecond Resolution**: Uses `micros()` for sub-millisecond accuracy
- **Jitter Calculation**: RFC 3393 compliant inter-arrival variation
- **Statistical Analysis**: Running averages and variance tracking

## 🔄 Future Enhancements

### Planned Features
- **Continuous Monitoring Mode**: Long-term network quality tracking
- **Historical Data**: Trend analysis and alerting
- **Custom Test Profiles**: User-defined test configurations  
- **Export Functionality**: CSV/JSON result export
- **Advanced Analytics**: Pattern detection and recommendations

### Performance Improvements
- **Parallel Testing**: Multiple concurrent test streams
- **IPv6 Support**: Dual-stack testing capabilities
- **Enhanced Protocols**: ICMP ping, DNS resolution timing
- **Bandwidth Correlation**: Link latency with throughput data

### Dedicated Testing Tools
To facilitate accurate latency testing, this utility comes with two external UDP echo server implementations:

#### 1. Python Echo Server (`scripts/udp_echo_server.py`)
A simple, portable server for quick local testing.
```bash
# Run on your PC/Laptop
python3 scripts/udp_echo_server.py --port 5000
```
Then point your ESP32 to your PC's IP:
`ESP32> latency test <pc_ip>:5000`

#### 2. C++ High-Performance Server (`pc_test_apps/udp_echo_server`)
An optimized, multi-threaded server for ultra-low latency testing (Linux only).
```bash
# Compile
cd pc_test_apps
make

# Run (requires sudo for real-time priority)
sudo ./udp_echo_server 5000
```
Features:
- `SCHED_FIFO` real-time process priority
- `SO_BUSY_POLL` for reduced interrupt latency
- Optimized socket buffers
- Nanosecond-level processing

---

This comprehensive latency and jitter analysis system transforms your ESP32 into a powerful network
diagnostic tool, providing professional-grade metrics for optimizing connectivity and troubleshooting
performance issues.