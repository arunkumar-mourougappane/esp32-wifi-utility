# iPerf Support for ESP32 WiFi Utility

## Overview

The ESP32 WiFi Utility now includes comprehensive iPerf performance testing capabilities. This allows you to measure network throughput and performance both as a client connecting to external iPerf servers and as a server accepting connections from other devices.

## Features

- **TCP and UDP Testing**: Support for both TCP throughput and UDP bandwidth/jitter testing
- **Client and Server Modes**: Act as either iPerf client or server
- **Configurable Parameters**: Customizable ports, duration, bandwidth limits, and buffer sizes
- **Real-time Statistics**: Live updates during testing with interval reporting
- **Comprehensive Results**: Detailed performance metrics including throughput, packet loss, and jitter

## Commands

### Basic Commands

| Command        | Description                                        |
| -------------- | -------------------------------------------------- |
| `iperf`        | Show iPerf help and available commands             |
| `iperf help`   | Show detailed iPerf command reference              |
| `iperf status` | Display current iPerf test status and last results |
| `iperf stop`   | Stop any running iPerf test                        |

### Server Mode Commands

| Command                   | Description                                        | Example                 |
| ------------------------- | -------------------------------------------------- | ----------------------- |
| `iperf server tcp [port]` | Start TCP server on specified port (default: 5201) | `iperf server tcp 5201` |
| `iperf server udp [port]` | Start UDP server on specified port (default: 5201) | `iperf server udp 5201` |

### Client Mode Commands

| Command                                               | Description                       | Example                                    |
| ----------------------------------------------------- | --------------------------------- | ------------------------------------------ |
| `iperf client tcp <ip> [port] [duration]`             | Run TCP throughput test to server | `iperf client tcp 192.168.1.100 5201 30`   |
| `iperf client udp <ip> [port] [duration] [bandwidth]` | Run UDP bandwidth test to server  | `iperf client udp 192.168.1.100 5201 10 5` |

### Parameters

- **`<ip>`**: Server IP address (required for client mode)
- **`[port]`**: Port number (default: 5201)
- **`[duration]`**: Test duration in seconds (default: 10, max: 3600)
- **`[bandwidth]`**: UDP bandwidth limit in Mbps (default: 1, max: 1000)

## Usage Examples

### 1. Basic TCP Throughput Test (Client)

```text
ESP32> iperf client tcp 192.168.1.100
```

This runs a 10-second TCP throughput test to server at 192.168.1.100:5201.

### 2. Custom TCP Test with Different Port and Duration

```text
ESP32> iperf client tcp 192.168.1.100 8080 30
```

This runs a 30-second TCP test to server at 192.168.1.100:8080.

### 3. UDP Bandwidth Test

```text
ESP32> iperf client udp 192.168.1.100 5201 15 10
```

This runs a 15-second UDP test at 10 Mbps to server at 192.168.1.100:5201.

### 4. Start TCP Server

```text
ESP32> iperf server tcp
```

This starts a TCP server listening on port 5201, waiting for client connections.

### 5. Start UDP Server on Custom Port

```text
ESP32> iperf server udp 9999
```

This starts a UDP server listening on port 9999.

## Test Results

The ESP32 displays comprehensive results after each test:

```text
📊 IPERF TEST RESULTS:
═══════════════════════
📦 Bytes transferred: 15.2 MB
⏱️ Duration: 10.00 seconds
 Throughput: 12.16 Mbps
📊 Packets: 15625 total, 23 lost (0.15%)
📈 Jitter: 1.23 ms
═══════════════════════
```

## Performance Considerations

### Memory Usage

- **RAM Impact**: ~1.8KB additional RAM usage for iPerf functionality
- **Flash Impact**: ~45KB additional flash storage
- **Buffer Size**: 1KB default buffer size for optimal performance

### Network Performance

- **TCP Tests**: Can achieve near line-rate performance depending on WiFi conditions
- **UDP Tests**: Configurable bandwidth limiting prevents network flooding
- **Concurrent Limits**: Only one iPerf test can run at a time

### Typical Performance Results

- **Local Network (WiFi)**: 10-50 Mbps depending on signal strength and interference
- **Internet Tests**: Limited by ISP bandwidth and latency
- **AP Mode**: Performance varies based on number of connected clients

## Integration with Existing Features

### WiFi Mode Compatibility

- **Station Mode**: Full client and server functionality when connected to network
- **AP Mode**: Server functionality available, clients can connect from other devices
- **Idle/Off Modes**: iPerf commands available but require active WiFi connection

### Status Integration

The `status` command now includes iPerf information when tests are running:

```text
📊 CURRENT STATUS:
──────────────────
WiFi Mode: 🔍 Station (Scanner)
Scanning: Enabled
iPerf: 🟢 Running TCP Client (192.168.1.100:5201)
Elapsed: 5 seconds
Current: 8.2 MB, 13.1 Mbps
```

## Troubleshooting

### Common Issues

1. **"Not connected to WiFi"**

   - Ensure WiFi is connected in station mode or AP mode is active
   - Use `status` to check current WiFi state

2. **"Failed to connect to server"**

   - Verify server IP address is correct and reachable
   - Check that iPerf server is running on target port
   - Ensure firewall allows iPerf traffic

3. **Low Performance Results**

   - Check WiFi signal strength and interference
   - Try different channels or closer proximity
   - Ensure no other high-bandwidth applications are running

4. **UDP Packet Loss**
   - Reduce bandwidth parameter for UDP tests
   - Check network congestion and WiFi conditions
   - Consider using TCP for more reliable throughput testing

### Getting Help

Use these commands for diagnostics:

- `status` - Check overall system status
- `iperf status` - Check iPerf-specific status
- `iperf help` - Review command syntax
- `iperf stop` - Stop any stuck tests

## Technical Details

### Protocol Implementation

- **TCP**: Standard TCP socket with configurable buffer sizes
- **UDP**: Packet-based with sequence numbers for loss detection
- **Statistics**: Real-time calculation of throughput, loss, and jitter

### Safety Features

- **Timeout Protection**: Automatic test termination after 2x configured duration
- **Memory Management**: Proper cleanup of network resources
- **Error Handling**: Graceful handling of network failures and disconnections

### Compatibility

- **Standard iPerf**: Compatible with iPerf3 servers on other platforms
- **Cross-Platform**: Works with Windows, Linux, and macOS iPerf implementations
- **Network Types**: Supports both 2.4GHz and 5GHz WiFi networks
