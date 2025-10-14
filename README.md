# ESP32 WiFi Scanner & Access Point

A comprehensive ESP32 project that can function as both a WiFi network scanner and a WiFi Access Point, with full serial command control and LED status indication.

## 🚀 Features

### 📡 Multiple WiFi Modes

- **Idle Mode**: Standby state - ready but not consuming WiFi resources
- **Station Mode**: Scan for nearby WiFi networks with detailed information
- **Access Point Mode**: Create a WiFi hotspot for other devices to connect
- **Off Mode**: Completely disable WiFi functionality
- **Seamless switching** between all modes via serial commands

### 🔍 Advanced WiFi Scanner

- **Enhanced Network Analysis** with comprehensive information display
- **Visual Signal Quality Indicators** (🟢🟡🟠🔴) with percentage calculation
- **Security Assessment** with encryption type icons and vulnerability detection
- **Professional Table Layout** with Unicode box drawing characters
- **Detailed network information display**:
  - SSID (Network Name) with hidden network detection
  - Signal Strength (RSSI) with quality percentage
  - Channel Number and frequency band identification
  - Encryption Type (🔓 Open, 🔒 WEP/WPA2, 🔐 WPA3, 🏢 Enterprise)
  - BSSID (Access Point MAC Address)
  - Channel congestion analysis and optimization recommendations
- **Interactive Commands**: `scan now` for immediate analysis, `scan info <id>` for detailed network examination
- **Network Statistics**: Summary of security types, signal strengths, and channel usage
- Real-time scanning control (on/off) with configurable intervals

### 📱 Access Point Features

- Creates WiFi hotspot with configurable SSID and password
- **QR Code Generation**: Automatic QR code display for instant mobile connection
- Real-time connected client monitoring with MAC addresses and signal strength
- Client management: list, disconnect individual clients or all clients
- Deauthentication capability for access control and testing
- Displays comprehensive AP network information
- Easy connection for mobile devices and computers

### 🌐 Network Connection & Testing

- **WiFi Connection**: Connect to any WPA/WPA2/WPA3 network with SSID and password
- **Connection Management**: Easy connect and disconnect commands
- **Connectivity Testing**: Advanced ping-like functionality using TCP connections
- **Multi-Port Testing**: Tests multiple ports (DNS, HTTP, HTTPS) for maximum compatibility
- **DNS Resolution**: Automatic hostname resolution for connectivity tests
- **Firewall Friendly**: Works even when ICMP ping is blocked by firewalls
- **Response Time Measurement**: Measures and reports connection establishment times
- **Network Diagnostics**: Provides detailed feedback on connectivity issues

### ⚡ iPerf Network Performance Testing

- **Comprehensive iPerf Implementation**: Full TCP and UDP performance testing
- **Dual Mode Operation**: Both client and server modes for flexible testing scenarios
- **Real-time Statistics**: Live bandwidth, transfer rate, and connection monitoring
- **Protocol Support**: TCP for reliable throughput testing, UDP for latency and packet loss analysis
- **Configurable Parameters**: Adjustable test duration, buffer sizes, and reporting intervals
- **Professional Metrics**: Detailed performance reports with transfer rates, timing, and efficiency statistics
- **Interactive Control**: Start, stop, and monitor tests via simple serial commands
- **Network Optimization**: Identify bottlenecks and optimize network performance

### 💡 Visual Status Indicators

- **LED Slow Pulse**: Idle mode - ready and waiting (every 2 seconds)
- **LED Fast Blink**: Active scanning or AP mode (250ms intervals)
- **LED Off**: WiFi completely disabled
- **Serial Feedback**: Detailed status messages with emoji indicators (🟡🔍📡🔴)

### ⌨️ Interactive Serial Interface

- **Smart Command Prompt**: Visual mode indicators (🟡📡🔍🔴) in prompt
- **Real-time Typing**: See characters as you type with backspace support
- **Command History**: Interactive command line with immediate feedback
- **Auto-completion Ready**: Extensible for future auto-complete features
- **Error Handling**: Clear error messages and user guidance
- **Status Integration**: Prompt shows current device mode at all times

### 🎯 Interactive Prompt Format

The command prompt dynamically shows the current device state:

| Prompt       | Mode             | Description                        |
| ------------ | ---------------- | ---------------------------------- |
| `🟡 ESP32> ` | **Idle**         | Device ready, WiFi disabled        |
| `🔍 ESP32> ` | **Station**      | Ready to scan or actively scanning |
| `📡 ESP32> ` | **Access Point** | WiFi hotspot active                |
| `🔴 ESP32> ` | **Off**          | WiFi completely disabled           |

**Typing Features:**

- **Real-time echo**: Characters appear as you type
- **Backspace support**: Use Backspace or Delete to correct mistakes
- **Command execution**: Press Enter to execute commands
- **Screen clearing**: Use `clear` command to clean up terminal output
- **Error feedback**: Invalid commands show helpful error messages

## 📚 Documentation

This project includes comprehensive documentation for all features and capabilities:

### 📖 Feature Documentation

| Document | Description | Key Topics |
|----------|-------------|------------|
| **[Enhanced WiFi Scanning](ENHANCED_SCANNING.md)** | Complete guide to advanced network scanning features | Visual indicators, security analysis, channel optimization |
| **[iPerf Network Testing](IPERF_GUIDE.md)** | Network performance measurement and analysis | TCP/UDP testing, bandwidth measurement, connectivity diagnostics |
| **[Unit Testing Guide](TEST_DOCUMENTATION.md)** | Comprehensive testing framework and validation | Test suite structure, hardware-in-loop testing, quality assurance |

### 🎯 Quick Links

- **[Enhanced Scanning Commands](ENHANCED_SCANNING.md#new-commands)** - `scan now`, `scan info <id>` with detailed network analysis
- **[iPerf Performance Testing](IPERF_GUIDE.md#quick-start)** - Network bandwidth and connectivity measurement
- **[Test Suite Execution](TEST_DOCUMENTATION.md#running-tests)** - Automated testing and validation procedures

### 📊 Feature Highlights

- **🔍 Advanced Scanning**: Signal quality visualization, encryption analysis, BSSID display
- **⚡ Network Performance**: iPerf TCP/UDP client/server with comprehensive metrics
- **🧪 Quality Assurance**: 17+ unit tests covering all functionality with hardware-in-loop testing

## 🏗️ Modular Architecture

This project features a **clean, modular codebase** designed for maintainability and extensibility:

### 📦 Core Modules

- **`config.h`** - Centralized configuration (pins, timings, credentials)
- **`wifi_manager`** - WiFi state management and enhanced network scanning
- **`ap_manager`** - Access Point client management and deauthentication
- **`iperf_manager`** - Network performance testing with TCP/UDP support
- **`led_controller`** - Visual status indication via LED
- **`command_interface`** - Interactive serial command processing and iPerf integration
- **`main.cpp`** - Clean entry point (setup/loop only)

### 🎯 Design Benefits

- **Separation of Concerns**: Each module handles specific functionality
- **Easy Maintenance**: Modify features without affecting others
- **Code Reusability**: Modules can be used independently
- **Clean Dependencies**: Well-defined interfaces between components
- **Scalability**: Easy to add new features or modify existing ones

### 🔗 Module Interactions

```text
main.cpp
    ├── config.h (hardware & timing constants)
    ├── wifi_manager (network operations & enhanced scanning)
    ├── ap_manager (client management & deauthentication)
    ├── iperf_manager (network performance testing)
    ├── led_controller (status indication)
    └── command_interface (user interaction & iPerf integration)
```

## 🛠️ Hardware Requirements

- **ESP32 Development Board** (any variant)
- **Built-in LED** on GPIO 2 (standard on most ESP32 boards)
- **USB Cable** for programming and serial communication
- **Computer** with serial terminal or Arduino IDE Serial Monitor

## ⚡ Quick Start

### 1. Hardware Setup

1. Connect your ESP32 to your computer via USB
2. Ensure the built-in LED is available (GPIO 2)

### 2. Software Setup

1. Install [PlatformIO](https://platformio.org/) or [Arduino IDE](https://www.arduino.cc/en/software)
2. Clone or download this project
3. Open the project in your IDE

### 3. Build and Upload

```bash
# Using PlatformIO (recommended)
pio run --target upload

# Or build only to check compilation
pio run

# Using Arduino IDE: File → Open → select main.cpp
```

**Note**: The project uses a modular structure with multiple source files. PlatformIO automatically handles the compilation of all `.cpp` files in the `src/` directory.

### 4. Connect to Serial Monitor

- **Baud Rate**: 115200
- **Line Ending**: Newline (NL)

## 📋 Command Reference

### Mode Commands

| Command                     | Description                                             |
| --------------------------- | ------------------------------------------------------- |
| `mode idle`                 | Set device to idle/standby mode (default startup state) |
| `mode station`              | Switch to station mode for WiFi scanning                |
| `mode ap`                   | Start Access Point mode with default settings           |
| `mode ap <ssid> <password>` | Start Access Point with custom SSID and password        |
| `mode off`                  | Disable WiFi completely                                 |

### Scanning Commands (Station Mode)

| Command           | Description                                        |
| ----------------- | -------------------------------------------------- |
| `scan on`         | Start automatic WiFi network scanning              |
| `scan off`        | Stop WiFi scanning                                 |
| `scan now`        | Immediate detailed scan with enhanced analysis     |
| `scan info <id>`  | Show comprehensive details for specific network    |

💡 **Enhanced Scanning**: The new scanning features provide detailed network analysis including signal quality indicators, security assessment, BSSID display, and channel congestion analysis. See [Enhanced Scanning Guide](ENHANCED_SCANNING.md) for complete details.

### Network Connection Commands (Station Mode)

| Command                     | Description                                          |
| --------------------------- | ---------------------------------------------------- |
| `connect <ssid> <password>` | Connect to a WiFi network with specified credentials |
| `disconnect`                | Disconnect from current WiFi network                 |

### Network Performance Testing Commands

| Command           | Description                                    |
| ----------------- | ---------------------------------------------- |
| `iperf`          | Show iPerf performance test help               |
| `iperf status`   | Show current iPerf test status                 |

💡 **Network Performance Testing**: The iPerf functionality provides comprehensive network bandwidth and performance analysis with both TCP and UDP protocols. See [iPerf Testing Guide](IPERF_GUIDE.md) for detailed usage, configuration options, and examples.

### Information Commands

| Command      | Description                                                  |
| ------------ | ------------------------------------------------------------ |
| `status`     | Show current device status and statistics                    |
| `ap info`    | Show detailed Access Point information (AP mode only)        |
| `ap clients` | List all connected clients with MAC addresses (AP mode only) |
| `qr`         | Display QR code for easy mobile connection (AP mode only)    |
| `clear`      | Clear console screen and redisplay header                    |
| `help`       | Display all available commands                               |

### Client Management Commands (AP Mode Only)

| Command        | Description                                                         |
| -------------- | ------------------------------------------------------------------- |
| `deauth <ID>`  | Disconnect client by ID number (e.g., `deauth 1`, `deauth 2`)       |
| `deauth <MAC>` | Disconnect client by MAC address (e.g., `deauth AA:BB:CC:DD:EE:FF`) |
| `deauth all`   | Disconnect all connected clients from the Access Point              |

## 🔧 Configuration

### WiFi Access Point Settings

Edit the following constants in `include/config.h` for default AP configuration:

```cpp
#define AP_SSID "ESP32-WiFiScanner"     // Default AP network name
#define AP_PASSWORD "123456789"         // Default AP password (min 8 chars)
```

**Custom Access Point Requirements:**

- **SSID**: 1-32 characters (any printable characters)
- **Password**: 8-63 characters for WPA2 security
- **Usage**: `mode ap <ssid> <password>`
- **Quotes**: Use quotes for SSIDs/passwords with spaces
- **Examples**:
  - `mode ap MyHotspot MyPassword123`
  - `mode ap "My WiFi Network" "My Complex Password!"`
- **Reset**: Use `mode ap` without parameters to return to default settings

### Timing Settings

```cpp
#define BLINK_INTERVAL 250              // LED blink rate (ms)
#define SCAN_INTERVAL 5000              // WiFi scan interval (ms)
```

### Hardware Configuration

```cpp
#define LED_PIN 2                       // Built-in LED pin
```

## 📊 Usage Examples

### Example 1: Interactive Command Prompt

```text
🟡 Device in IDLE mode - Ready for commands
Type 'help' for available commands

🟡 ESP32> help
📋 AVAILABLE COMMANDS:
┌─────────────────┬──────────────────────────────────────┐
│ Command         │ Description                          │
├─────────────────┼──────────────────────────────────────┤
│ mode idle       │ Set device to idle/standby mode      │
│ mode station    │ Enable station mode for scanning     │
│ mode ap         │ Start as Access Point                │
└─────────────────┴──────────────────────────────────────┘

🟡 ESP32> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 12345678
  IP Address: 192.168.4.1

📡 ESP32> status
📊 CURRENT STATUS:
──────────────────
WiFi Mode: 📡 Access Point
Connected Clients: 0

📡 ESP32> mode station
✓ Station mode activated - Ready to scan for networks

🔍 ESP32> scan on
✓ WiFi scanning ENABLED

🔍 ESP32> clear
==========================================
       ESP32 WiFi Scanner & AP
==========================================
🔍 STATION mode - Scanning active
==========================================

🔍 ESP32> _
```

### Example 2: Device Startup (Idle Mode)

```text
🟡 Device in IDLE mode - Ready for commands
Type 'help' for available commands

🟡 ESP32> status
📊 CURRENT STATUS:
──────────────────
WiFi Mode: 🟡 Idle (Ready)
Scanning: Disabled
LED Status: Slow Pulse (every 2s)
Free Heap: 298756 bytes

> help
📋 AVAILABLE COMMANDS:
┌─────────────────┬──────────────────────────────────────┐
│ Command         │ Description                          │
├─────────────────┼──────────────────────────────────────┤
│ mode idle       │ Set device to idle/standby mode      │
│ mode station    │ Enable station mode for scanning     │
│ mode ap         │ Start as Access Point                │
│ mode off        │ Disable WiFi completely              │
└─────────────────┴──────────────────────────────────────┘
```

### Example 3: Enhanced WiFi Network Scanner

```text
> mode station
✓ Station mode activated - Ready to scan for networks

> scan now
🔍 === WiFi Network Scanner === 🔍
Scanning for available networks...
✅ Discovered 5 networks:

╔════╤═══════════════════════════╤══════╤════╤═══════════════════╤═════════╤═══════════════════╗
║ ID │         SSID              │ RSSI │ CH │    Encryption     │ Quality │      BSSID        ║
╠════╪═══════════════════════════╪══════╪════╪═══════════════════╪═════════╪═══════════════════╣
║  1 │ HomeWiFi                  │  -42 │  6 │ 🔒 WPA2           │ 🟢  95% │ AA:BB:CC:DD:EE:FF ║
║  2 │ OfficeNet                 │  -67 │ 11 │ 🔒 WPA2           │ 🟡  65% │ 11:22:33:44:55:66 ║
║  3 │ CafeGuest                 │  -78 │  1 │ 🔓 Open           │ 🟠  35% │ 22:33:44:55:66:77 ║
║  4 │ Neighbor5G                │  -52 │ 36 │ 🔐 WPA3           │ 🟢  85% │ 33:44:55:66:77:88 ║
║  5 │ <Hidden Network>          │  -85 │  9 │ 🔒 WPA2           │ 🔴  25% │ 44:55:66:77:88:99 ║
╚════╧═══════════════════════════╧══════╧════╧═══════════════════╧═════════╧═══════════════════╝

📈 Network Summary:
├─ 🔓 Open: 1  🔒 WEP: 0  🔒 WPA2: 3  🔐 WPA3: 1
├─ 📶 Strong signals (>-60dBm): 2
├─ 📱 Weak signals (<-80dBm): 1
├─ 📡 Most congested channel: 6 (2 networks)
└─ 💡 Recommended channels for AP: 1, 6, 11 (least interference)

> scan info 1
📡 === Detailed Network Information === 📡
┌─────────────────────────────────────────────────────────┐
│ 🏷️  Network Name: HomeWiFi                             │
├─────────────────────────────────────────────────────────┤
│ 🔗 BSSID (MAC):  AA:BB:CC:DD:EE:FF                     │
│ 📶 Signal (RSSI): -42 dBm                              │
│ 📊 Signal Quality: 95% (Excellent) 🟢🟢🟢🟢              │
│ 📻 Channel:      6 (2.4GHz)                            │
│ 🚦 Congestion:   Light (2 networks on this channel)    │
│ 🔐 Security:     🔒 WPA2 Personal                      │
│ 🛡️  Security Level: 🟢 Good                            │
├─────────────────────────────────────────────────────────┤
│ 💡 Connection Analysis:                              │
│ 📶 Excellent signal strength for stable connection   │
│ 🚀 Est. Speed:   50-150 Mbps (802.11n)               │
└─────────────────────────────────────────────────────────┘
💡 To connect: connect "HomeWiFi" <password>
```

### Example 4: Access Point Mode with QR Code

```text
> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 12345678
  IP Address: 192.168.4.1
  Use 'ap info' for detailed information

═══════════════════════════════════════════════════════════
                    AP CONNECTION QR CODE
═══════════════════════════════════════════════════════════
Scan this QR code with your mobile device to connect:

██████████████  ██  ██  ██████████████
██          ██  ████    ██          ██
██  ██████  ██  ██  ██  ██  ██████  ██
██  ██████  ██    ██    ██  ██████  ██
██  ██████  ██  ██      ██  ██████  ██
██          ██    ██    ██          ██
██████████████  ██  ██  ██████████████
                ██  ██
██████  ████████    ████  ██    ██████
      ██    ██  ██  ██████  ██    ████
██  ██████    ████  ██    ████      ██
████████  ██  ██    ██  ████████  ████
████    ██  ██    ████  ██        ████
                ██    ██  ████  ██████
██████████████  ██████    ████    ████
██          ██    ██    ████    ██  ██
██  ██████  ██  ██████████████  ██████
██  ██████  ██    ██      ██████  ████
██  ██████  ██  ████    ████  ████  ██
██          ██  ██    ██    ██
██████████████  ██    ████████  ██  ██

Network Information:
  SSID: ESP32-WiFiScanner
  Security: WPA
  Password: 12345678
═══════════════════════════════════════════════════════════

> ap info
📡 ACCESS POINT INFORMATION:
─────────────────────────────
SSID: ESP32-WiFiScanner
Password: 12345678
IP Address: 192.168.4.1
MAC Address: 24:6F:28:XX:XX:XX
Connected Clients: 2
Channel: 1
```

### Example 5: Custom Access Point Configuration

```text
> mode ap "MyCustomHotspot" "SecurePassword123"
✓ Custom Access Point mode activated
  SSID: MyCustomHotspot
  Password: SecurePassword123
  IP Address: 192.168.4.1
  Use 'ap info' for detailed information

═══════════════════════════════════════════════════════════
                    AP CONNECTION QR CODE
═══════════════════════════════════════════════════════════
Scan this QR code with your mobile device to connect:

██████████████  ██  ██  ██████████████
██          ██  ████    ██          ██
██  ██████  ██  ██  ██  ██  ██████  ██
██  ██████  ██    ██    ██  ██████  ██
██  ██████  ██  ██      ██  ██████  ██
██          ██    ██    ██          ██
██████████████  ██  ██  ██████████████
                ██  ██
██████  ████████    ████  ██    ██████
      ██    ██  ██  ██████  ██    ████
██  ██████    ████  ██    ████      ██
████████  ██  ██    ██  ████████  ████
████    ██  ██    ████  ██        ████
                ██    ██  ████  ██████
██████████████  ██████    ████    ████
██          ██    ██    ████    ██  ██
██  ██████  ██  ██████████████  ██████
██  ██████  ██    ██      ██████  ████
██  ██████  ██  ████    ████  ████  ██
██          ██  ██    ██    ██
██████████████  ██    ████████  ██  ██

Network Information:
  SSID: MyCustomHotspot
  Security: WPA
  Password: SecurePassword123
═══════════════════════════════════════════════════════════

> ap info
📡 ACCESS POINT INFORMATION:
─────────────────────────────
SSID: MyCustomHotspot
Password: SecurePassword123
IP Address: 192.168.4.1
MAC Address: 24:6F:28:XX:XX:XX
Connected Clients: 1
Channel: 1

> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 123456789
  IP Address: 192.168.4.1

Note: Switching to 'mode ap' without parameters resets to default configuration
```

### Example 6: Status Monitoring & Mode Switching

```text
> status
📊 CURRENT STATUS:
──────────────────
WiFi Mode: 📡 Access Point
Scanning: Disabled
LED Status: Fast Blinking
AP IP: 192.168.4.1
Connected Clients: 1
Free Heap: 298756 bytes

> mode idle
🟡 Device set to IDLE mode - Ready for commands
  Use 'mode station' or 'mode ap' to activate WiFi

> mode off
✓ WiFi disabled
```

### Example 7: QR Code Command

```text
> qr
═══════════════════════════════════════════════════════════
                    AP CONNECTION QR CODE
═══════════════════════════════════════════════════════════
Scan this QR code with your mobile device to connect:

██████████████  ██  ██  ██████████████
██          ██  ████    ██          ██
██  ██████  ██  ██  ██  ██  ██████  ██
██  ██████  ██    ██    ██  ██████  ██
██  ██████  ██  ██      ██  ██████  ██
██          ██    ██    ██          ██
██████████████  ██  ██  ██████████████
                ██  ██
██████  ████████    ████  ██    ██████
      ██    ██  ██  ██████  ██    ████
██  ██████    ████  ██    ████      ██
████████  ██  ██    ██  ████████  ████
████    ██  ██    ████  ██        ████
                ██    ██  ████  ██████
██████████████  ██████    ████    ████
██          ██    ██    ████    ██  ██
██  ██████  ██  ██████████████  ██████
██  ██████  ██    ██      ██████  ████
██  ██████  ██  ████    ████  ████  ██
██          ██  ██    ██    ██
██████████████  ██    ████████  ██  ██

Network Information:
  SSID: ESP32-WiFiScanner
  Security: WPA
  Password: 12345678
═══════════════════════════════════════════════════════════
```

### Example 8: AP Client Management

```text
> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 12345678
  IP Address: 192.168.4.1

> ap clients
👥 CONNECTED CLIENTS:
─────────────────────
ID │ MAC Address       │ RSSI
───┼───────────────────┼──────
 1 │ AA:BB:CC:DD:EE:FF │  -45
 2 │ 11:22:33:44:55:66 │  -52

Total clients: 2
Use 'deauth <ID>' (e.g., 'deauth 1') or 'deauth <MAC>' to disconnect
Use 'deauth all' to disconnect all clients

> deauth 1
✓ Client disconnected: AA:BB:CC:DD:EE:FF

> deauth AA:BB:CC:DD:EE:FF
✗ Error: MAC address not found in connected clients

> deauth all
⚠️  Deauthenticating all 1 connected clients...
✓ Disconnected: 11:22:33:44:55:66
✓ Successfully deauthenticated 1 out of 1 clients
```

### Example 9: Network Connection and iPerf Testing

```text
> mode station
✓ Station mode activated - Ready to scan for networks

> connect "HomeWiFi" "mypassword123"
🔗 Connecting to 'HomeWiFi'...
........
✓ Connected to 'HomeWiFi'
  IP Address: 192.168.1.105
  Gateway: 192.168.1.1
  DNS: 192.168.1.1

> iperf
� === iPerf Network Performance Testing === 📊
Available Commands:
├─ iperf start server [tcp|udp] [port]
├─ iperf start client <host> [tcp|udp] [port] [duration]
├─ iperf stop
├─ iperf status

Examples:
├─ iperf start server tcp 5201
├─ iperf start client 192.168.1.100 udp 5201 30
└─ iperf status

> iperf status
📊 iPerf Status: Idle
💡 Use 'iperf start server' or 'iperf start client <host>' to begin testing

> disconnect
✓ Disconnected from 'HomeWiFi'
```

## 🔍 LED Status Indicators

| LED Pattern               | Mode            | Meaning                                      |
| ------------------------- | --------------- | -------------------------------------------- |
| **Slow Pulse** (every 2s) | 🟡 IDLE         | Ready and waiting for commands               |
| **Fast Blinking** (250ms) | 🔍📡 STATION/AP | WiFi scanning active OR Access Point running |
| **Off**                   | 🔴 OFF          | WiFi completely disabled                     |

## 🔄 Mode State Diagram

```text
    Startup
       ↓
🟡 IDLE (Ready) ←→ 🔍 STATION (Scanner) ←→ 📡 AP (Hotspot)
       ↓                    ↓                      ↓
      🔴 OFF (Disabled) ←────────────────────────────
```

## ⚠️ Security & Usage Notes

### Client Deauthentication

- **Purpose**: Deauthentication is useful for testing, access control, and network management
- **Legal Notice**: Only use on networks you own or have explicit permission to test
- **Automatic Reconnection**: Most devices will automatically attempt to reconnect after deauthentication
- **Range**: Deauthentication only affects devices connected to your ESP32 Access Point

### Best Practices

- Use `ap clients` to monitor connected devices regularly
- Use `deauth <MAC>` for targeted disconnection of specific devices
- Use `deauth all` for quickly clearing all connections (useful for testing)
- Monitor the serial output for successful deauthentication confirmations

### Client Identification Methods

The `deauth` command accepts clients in these formats:

- **Client ID**: `1`, `2`, `3` (easiest method - use numbers from `ap clients` list)
- **MAC with colons**: `AA:BB:CC:DD:EE:FF`
- **MAC without colons**: `AABBCCDDEEFF`
- **Case insensitive**: both uppercase and lowercase MAC addresses work

## 🐛 Troubleshooting

### Common Issues

**1. Compilation Errors**

- Ensure you're using ESP32 board package
- Check that all includes are available
- Verify PlatformIO configuration
- Make sure all `.cpp` files in `src/` directory are present
- Confirm header files in `include/` directory are accessible

**2. Serial Commands Not Working**

- Check baud rate is set to 115200
- Ensure line ending is set to "Newline" (NL)
- Verify USB connection

**3. WiFi Not Scanning**

- Make sure you're in station mode (`mode station`)
- Enable scanning with `scan on`
- Check if WiFi is available in your area

**4. Access Point Not Visible**

- Verify AP mode is active (`mode ap`)
- Check if SSID appears in nearby networks
- Ensure password is at least 8 characters

**5. LED Not Working**

- Verify GPIO 2 has built-in LED on your board
- Check LED_PIN definition matches your hardware

**6. Device Not Responding**

- Check if device is in the right mode (`status`)
- Try `mode idle` to return to standby state
- Power cycle if completely unresponsive

**7. Deauthentication Not Working**

- Ensure you're in Access Point mode (`mode ap`)
- Verify the MAC address format is correct
- Check that the client is actually connected (`ap clients`)
- Some devices may reconnect automatically after deauth

**8. Client List Empty**

- Make sure devices are connected to your ESP32 AP
- Wait a few seconds after connection for the list to update
- Try refreshing with `ap clients` command

**9. Network Connection Issues**

- Verify SSID spelling and case sensitivity in `connect` command
- Ensure password is correct and properly quoted if it contains spaces
- Check signal strength - move closer to router if needed
- Some networks require additional authentication (enterprise, captive portals)

**10. iPerf Test Issues**

- Ensure you're connected to a network first (`connect` command)
- Verify target host is reachable and has iPerf server running
- Check firewall settings on both client and server
- Use `iperf status` to monitor test progress and results
- Corporate/school networks may have restrictive firewalls

### Debug Commands

```
> status          # Check current mode and settings
> help            # Review all available commands
> mode idle       # Return to standby state
> mode off        # Reset WiFi if having issues
> mode station    # Restart in station mode
```

## 📁 Project Structure

```text
esp32-wifi-utility/
├── platformio.ini              # PlatformIO configuration
├── README.md                   # Main project documentation
├── ENHANCED_SCANNING.md        # Enhanced WiFi scanning features guide
├── IPERF_GUIDE.md             # iPerf network testing documentation
├── TEST_DOCUMENTATION.md       # Unit testing and validation guide
├── LICENSE                     # Project license
├── src/                        # Source implementation files
│   ├── main.cpp               # Main application entry point
│   ├── wifi_manager.cpp       # WiFi state management and enhanced scanning
│   ├── ap_manager.cpp         # Access Point client management
│   ├── iperf_manager.cpp      # Network performance testing implementation
│   ├── led_controller.cpp     # LED status indication
│   └── command_interface.cpp  # Serial command processing and feature integration
├── include/                    # Header files
│   ├── config.h              # Hardware and configuration constants
│   ├── wifi_manager.h        # WiFi management and scanning declarations
│   ├── ap_manager.h          # AP client management declarations
│   ├── iperf_manager.h       # iPerf performance testing declarations
│   ├── led_controller.h      # LED control declarations
│   └── command_interface.h   # Command interface declarations
├── lib/                       # Custom libraries
└── test/                      # Comprehensive unit test suite
    └── test_iperf_comprehensive.cpp # 17+ unit tests for all functionality
```

## 🔧 Technical Details

### Memory Usage

- **Flash**: ~805KB total (61.4% of ESP32 capacity with all features)
  - Core WiFi functionality: ~200KB
  - Enhanced scanning features: ~8KB
  - iPerf implementation: ~25KB
  - QR Code generation: ~12KB
  - Command interface and utilities: ~15KB
- **RAM**: ~45KB baseline usage with enhanced features (13.9% of available RAM)
- **Free Heap**: Typically 280KB+ available during operation

### WiFi Specifications

- **Protocols**: 802.11 b/g/n
- **Frequency**: 2.4 GHz
- **Security**: Open, WEP, WPA/WPA2/WPA3 PSK, WPA2 Enterprise
- **Range**: Typical ESP32 WiFi range (varies by antenna and environment)

### Performance

- **Scan Speed**: ~3-5 seconds per enhanced scan with full analysis
- **AP Capacity**: Up to 4 simultaneous clients (ESP32 limitation)
- **Response Time**: <100ms for serial commands
- **Connection Time**: Typical WiFi connection in 5-10 seconds
- **iPerf Throughput**:
  - TCP: Up to 10-20 Mbps depending on network conditions
  - UDP: Configurable rates up to network capacity
  - Real-time statistics with <1 second update intervals

### Networking Features

- **Connection Methods**: WPA/WPA2/WPA3 PSK authentication
- **Network Performance Testing**: iPerf TCP/UDP client and server modes
- **Real-time Statistics**: Live bandwidth, transfer rate, and connection monitoring
- **Network Diagnostics**: Connection status, IP assignment, gateway detection
- **Protocol Support**: Comprehensive TCP and UDP performance analysis

### Code Organization

- **Total Lines**: ~1200+ lines organized across multiple modules
- **Main Loop**: Clean entry point in `main.cpp`
- **Modular Design**: Functions grouped by responsibility with clear separation
- **Enhanced Features**: Comprehensive WiFi scanning, iPerf testing, unit test suite
- **Memory Efficient**: Optimized global state management with proper extern declarations  
- **Quality Assurance**: 17+ unit tests covering all functionality
- **Compilation**: Zero warnings with optimized builds
- **Documentation**: Comprehensive guides for all features and capabilities

## 🚧 Future Enhancements

### 🔧 Core Features

- [ ] **Power Management**: Sleep mode for battery operation
- [ ] **Web Interface**: Wireless control via built-in web server
- [ ] **Network Storage**: Save and auto-connect to known networks
- [ ] **Signal Analysis**: Real-time signal strength graphing
- [ ] **Multi-AP Support**: Multiple AP configurations and switching
- [ ] **Security Scanner**: Network vulnerability assessment

### 📊 Data & Connectivity

- [ ] **Data Logging**: Save scan results to SD card/flash
- [ ] **Mobile App**: Companion smartphone application
- [ ] **Mesh Networking**: ESP-NOW mesh communication
- [ ] **Custom Protocols**: Support for additional wireless protocols
- [ ] **Cloud Integration**: Upload scan data to cloud services
- [ ] **JSON API**: RESTful API for external applications

### 🏗️ Architecture Improvements

- [ ] **Event System**: Asynchronous event-driven architecture
- [ ] **Plugin System**: Loadable modules for extended functionality
- [ ] **Configuration Manager**: Runtime configuration changes
- [ ] **State Machine**: Formal state management system
- [ ] **Error Handling**: Comprehensive error recovery system
- [ ] **Unit Tests**: Automated testing framework

## 📄 License

This project is open source. Feel free to modify and distribute according to your needs.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## 📞 Support

If you encounter any issues or have questions:

1. Check the troubleshooting section above
2. Review the command reference
3. Use the `status` and `help` commands for debugging
4. Open an issue on the project repository

---

**Happy WiFi Scanning! 📡**
