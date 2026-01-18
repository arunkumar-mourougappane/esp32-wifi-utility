# ESP32 WiFi Utility - Features

Comprehensive guide to all features and capabilities.

## Table of Contents

- [WiFi Mode Management](#wifi-mode-management)
- [Network Scanning & Analysis](#network-scanning--analysis)
- [Access Point Features](#access-point-features)
- [Channel Analysis](#channel-analysis)
- [Network Performance Testing](#network-performance-testing)
- [Web Interface](#web-interface)
- [TFT Display Support](#tft-display-support)
- [Configuration Persistence](#configuration-persistence)
- [Visual Status Indicators](#visual-status-indicators)
- [Serial Interface](#serial-interface)

---

## WiFi Mode Management

### 📡 Multiple WiFi Modes

The device supports four distinct operating modes:

- **Idle Mode**: Standby state - ready but not consuming WiFi resources
- **Station Mode**: Scan for nearby WiFi networks with comprehensive analysis
- **Access Point Mode**: Create a WiFi hotspot for other devices to connect
- **Off Mode**: Completely disable WiFi functionality
- **Seamless switching** between all modes via serial commands

### Mode State Diagram

```text
    Startup
       ↓
🟡 IDLE (Ready) ←→ 🔍 STATION (Scanner) ←→ 📡 AP (Hotspot)
       ↓                    ↓                      ↓
      🔴 OFF (Disabled) ←────────────────────────────
```

---

## Network Scanning & Analysis

### 🔍 Advanced WiFi Scanner & Spectrum Analysis

- **Professional Spectrum Analyzer** with enterprise-grade channel analysis
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
  - Real-time channel congestion scoring (0-100%)
- **Network Statistics**: Summary of security types, signal strengths, and channel usage
- **Interactive Commands**: `scan now` for immediate analysis, `scan info <id>` for detailed examination
- Real-time scanning control (on/off) with configurable intervals

### Clickable Network Details (v4.0.0+)

Interactive WiFi scanning with detailed network information pages:

- 8-level signal quality scale with recommendations
- Channel congestion analysis (Clear to Severe ratings)
- Security evaluation with encryption type details
- Connection recommendations based on signal and channel conditions
- Visual indicators with emoji icons and color-coded ratings
- Smart caching (50 networks, 5 minutes) for minimal RAM impact

---

## Access Point Features

### 📱 WiFi Hotspot Creation

Create a WiFi access point with full control:

- Creates WiFi hotspot with configurable SSID and password
- **Security Types**: Open, WPA2, WPA3, Mixed mode
- **QR Code Generation**: Automatic QR code display for instant mobile connection
- Real-time connected client monitoring with MAC addresses and signal strength
- Client management: list, disconnect individual clients or all clients
- Deauthentication capability for access control and testing
- Displays comprehensive AP network information
- Easy connection for mobile devices and computers

### Client Management

- List all connected devices with MAC addresses and signal strength
- Disconnect specific clients by ID or MAC address
- Disconnect all clients with a single command
- Real-time client monitoring
- Automatic reconnection detection

---

## Channel Analysis

### 🆕 Professional Channel Analysis System

Enterprise-grade spectrum analysis and optimization:

- **Real-time Spectrum Scanning**: Complete 2.4GHz band analysis (channels 1-14)
- **Advanced Congestion Analysis**: AI-powered 0-100% congestion scoring
- **Channel Overlap Detection**: Mathematical interference analysis and optimization
- **Smart Recommendations**: Automated optimal channel selection with rationale
- **Interference Classification**: Microwave, Bluetooth, and continuous wave detection
- **Background Monitoring**: Configurable 1-60 second monitoring intervals
- **Professional Reporting**: JSON export and detailed optimization reports
- **Visual Feedback**: Real-time LED status indication during analysis

### Channel Commands

- `channel scan` - Quick 5-second spectrum analysis
- `channel scan detailed` - Comprehensive 30-second analysis
- `channel monitor start/stop` - Background monitoring
- `channel recommend` - AI-powered optimal channel suggestions
- `channel export` - JSON data export for analysis

See [Channel Analysis Guide](../guides/CHANNEL_GUIDE.md) for complete details.

---

## Network Performance Testing

### 📊 Network Performance & Latency Analysis

Comprehensive network diagnostics and performance measurement:

- **Advanced Latency Testing**: Comprehensive ping-style analysis with jitter calculations
- **Statistical Analysis**: Mean, median, min/max latency with standard deviation
- **Packet Loss Detection**: Real-time packet loss monitoring and reporting
- **Network Quality Assessment**: Professional network performance scoring
- **Historical Tracking**: Trend analysis and performance monitoring over time

### ⚡ iPerf Network Performance Testing

Full-featured iPerf implementation for bandwidth testing:

- **Comprehensive iPerf Implementation**: Full TCP and UDP performance testing
- **Dual Mode Operation**: Both client and server modes for flexible testing scenarios
- **Real-time Statistics**: Live bandwidth, transfer rate, and connection monitoring
- **Protocol Support**: TCP for reliable throughput testing, UDP for latency and packet loss analysis
- **Configurable Parameters**: Adjustable test duration, buffer sizes, and reporting intervals
- **Professional Metrics**: Detailed performance reports with transfer rates, timing, and efficiency statistics
- **Interactive Control**: Start, stop, and monitor tests via simple serial commands
- **Network Optimization**: Identify bottlenecks and optimize network performance

See [iPerf Testing Guide](../guides/IPERF_GUIDE.md) and [Latency Testing Guide](../guides/LATENCY_GUIDE.md) for complete details.

---

## Web Interface

### 🌐 Web Server Interface

Professional browser-based control interface (available on both ESP32dev and Feather boards):

- **Browser-Based Control**: Access device features via any web browser
- **Mobile-Responsive Design**: Beautiful interface optimized for phones, tablets, and desktops
- **Hierarchical Navigation**: Professional dropdown menu system for organized access
- **Progress Indicators**: Visual feedback with backdrop overlay during scan operations
- **Real-Time Monitoring**: Live system status, WiFi statistics, and client information
- **Network Visualization**: Interactive WiFi scanning with clickable network details

### Professional Dashboard Pages

- 🏠 **Home** - Quick stats and system overview with unified metrics
- 📊 **Status** - Detailed system information and configuration with modern grid layout
- 🔍 **Scan Networks** - Interactive WiFi scanning with clickable network details
  - Click any network to view comprehensive information
  - Signal quality assessment with 8-level scale
  - Channel congestion analysis and recommendations
  - Security evaluation with encryption details
- 🔬 **Analysis Dashboard** - Unified testing hub with quick actions
  - ⚡ iPerf Testing - Bandwidth measurement and throughput analysis
  - 📉 Latency Testing - Network quality and jitter analysis with live progress bars
  - 📡 Channel Analysis - Spectrum analysis and congestion monitoring
  - 📶 Signal Monitor - Real-time signal strength analysis with auto-scan
  - 🔒 Port Scanner - Network security auditing and open port detection

### Modern UI/UX Features

- Gradient design with professional styling and smooth animations
- Dropdown menus with hover states and mobile-responsive behavior
- Full-screen progress overlays with animated spinners
- Smart status badges for connection state and mode
- Responsive grid layouts and card-based design
- Hamburger menu for mobile devices
- Zero Configuration: Automatic IP detection and URL generation
- Multi-Mode Support: Works in both AP mode and Station mode

### Web Configuration Interface (v4.2.0+)

Professional web-based configuration and management:

- **AP Configuration Section**: SSID, password, channel, security type, auto-start toggle
- **Station Configuration Section**: WiFi network credentials, security preferences, auto-connect
- **Quick Mode Switch**: Toggle between AP and Station modes without rebooting (2-second switch)
- **Reboot Modal**: Countdown timer with confirm/cancel options
- **Password Security**: Passwords masked in input fields, saved passwords never displayed
- **Real-time Validation**: Immediate feedback on input errors

---

## TFT Display Support

### 📱 Built-in Screen Support

Display support for Adafruit Feather ESP32-S3 TFT boards with complete UI redesign (v5.2.0):

#### Welcome Screen

- **Branded Startup**: Purple ESP32 logo with "ESP32 WiFi Utility" title
- **Professional First Impression**: 2-second welcome screen on device boot
- **Consistent Branding**: Matches project identity across all interfaces

#### Color-Coded Status Screens

**Visual Status System**:

| Color     | Status           | Usage                                    |
|-----------|------------------|------------------------------------------|
| 🔵 Blue   | Connecting/Idle  | Station mode connecting or idle          |
| 🟢 Green  | Connected/Active | Successful connection or AP initializing |
| 🔴 Red    | Disabled         | WiFi stopped                             |
| 🟡 Yellow | Idle Mode        | Idle without text                        |

#### Mode-Specific Displays

- **AP Mode**:
  - SSID and password display
  - QR code for easy device connection
  - AP IP address
  - Real-time client count
  - Battery status with percentage
  - Current time display

- **Station Mode**:
  - Connection status
  - Connected network SSID
  - Local IP address
  - Real-time signal strength (RSSI in dBm)
  - Color-coded signal quality (Green/Yellow/Red)
  - Visual signal quality bar graph

#### Technical Features

- **Dynamic Updates**: Automatically updates based on WiFi mode
- **Compact Layout**: Optimized for 135x240 pixel display
- **Modular Architecture**: Queue-based messaging, non-blocking operations
- **Memory Efficient**: Shared bitmap rendering, minimal flash usage
- **Board Support**: Normal and Reverse TFT variants

---

## Configuration Persistence

### 💾 Configuration Persistence System (v4.2.0+)

Complete NVS-based storage for Access Point and Station configurations:

#### Access Point Configuration

- **Persistent Settings**: SSID, password (base64 encoded), channel, security type, auto-start
- **Survives Reboots**: Configuration stored securely in non-volatile memory
- **Default Fallback**: Automatic defaults if no config saved
- **Serial Commands**: `ap save/load/show/clear`
- **Web Interface**: Full configuration via `/config` page
- **Secure Storage**: Passwords encoded in base64 before NVS storage

#### Station Configuration

- **WiFi Credentials**: SSID and password (base64 encoded) storage
- **Security Preferences**: auto, wpa3prefer, wpa3only, wpa2min, wpa2only
- **Auto-Connect**: Automatic connection on boot when enabled
- **Secure Storage**: Passwords never exposed in plain text
- **Serial Commands**: `station save/load/show/clear`
- **Password Privacy**: Saved passwords never displayed on web interface

#### Boot Behavior (Priority Order)

1. Check for saved AP config with auto-start enabled → Start AP mode
2. Check for saved Station config with auto-connect enabled → Connect to WiFi
3. No saved config or auto-start/connect disabled → Start in IDLE mode

---

## Visual Status Indicators

### 💡 LED Status Indicators

Different patterns for different device states:

| LED Pattern               | Mode            | Meaning                                      |
| ------------------------- | --------------- | -------------------------------------------- |
| **Slow Pulse** (every 2s) | 🟡 IDLE         | Ready and waiting for commands               |
| **Fast Blinking** (250ms) | 🔍📡 STATION/AP | WiFi scanning active OR Access Point running |
| **Off**                   | 🔴 OFF          | WiFi completely disabled                     |

### Hardware Support

- **ESP32dev**: Standard LED on GPIO 2
- **Feather ESP32-S3**: NeoPixel RGB LED with enhanced color status indication

---

## Serial Interface

### ⌨️ Interactive Serial Interface

Professional command-line interface with rich features:

- **Smart Command Prompt**: Visual mode indicators (🟡📡🔍🔴) in prompt
- **Real-time Typing**: See characters as you type with backspace support
- **Command History**: Interactive command line with immediate feedback
- **Auto-completion Ready**: Extensible for future auto-complete features
- **Error Handling**: Clear error messages and user guidance
- **Status Integration**: Prompt shows current device mode at all times

### Prompt Format

| Prompt      | Mode             | Description                        |
| ----------- | ---------------- | ---------------------------------- |
| `🟡 ESP32>` | **Idle**         | Device ready, WiFi disabled        |
| `🔍 ESP32>` | **Station**      | Ready to scan or actively scanning |
| `📡 ESP32>` | **Access Point** | WiFi hotspot active                |
| `🔴 ESP32>` | **Off**          | WiFi completely disabled           |

### Typing Features

- **Real-time echo**: Characters appear as you type
- **Backspace support**: Use Backspace or Delete to correct mistakes
- **Command execution**: Press Enter to execute commands
- **Screen clearing**: Use `clear` command to clean up terminal output
- **Error feedback**: Invalid commands show helpful error messages

---

## Security Features

### 🔒 Security Configuration

Comprehensive WiFi security support:

- **Access Point Security**:
  - Open networks (no password)
  - WPA2-PSK (recommended)
  - WPA3-PSK (maximum security, ESP32-S2/S3/C3+ only)
  - WPA2/WPA3 Mixed mode (compatibility)

- **Station Security Preferences**:
  - Auto-negotiate (accept any security type)
  - WPA3 preferred (fallback to WPA2)
  - WPA3 only (reject WPA2 and lower)
  - Minimum WPA2 (reject WEP/Open)
  - WPA2 only (reject WPA3, WEP, Open)

- **Password Security**:
  - Base64 encoding for storage
  - Password masking in web interface
  - Saved passwords never displayed
  - Secure NVS storage

See [Security Configuration Guide](../security/SECURITY_CONFIGURATION.md) for complete details.

---

## Performance Metrics

### Memory Usage

#### ESP32 Development Board

- **Flash**: ~1018KB (77.6% of capacity)
- **RAM**: ~52KB baseline (15.8% of available)
- **Free Heap**: Typically 275KB+ during operation

#### Adafruit Feather ESP32-S3 TFT

- **Flash**: ~991KB (68.7% of capacity)
- **RAM**: ~62KB baseline (19.1% of available)
- **Free Heap**: Typically 265KB+ during operation
- **PSRAM**: 8MB additional high-speed memory

### WiFi Specifications

- **Protocols**: 802.11 b/g/n
- **Frequency**: 2.4 GHz
- **Security**: Open, WEP, WPA/WPA2/WPA3 PSK, WPA2 Enterprise
- **Range**: Typical ESP32 WiFi range

### Performance

- **Scan Speed**: 3-5 seconds per enhanced scan
- **AP Capacity**: Up to 4 simultaneous clients
- **Response Time**: <100ms for serial commands
- **Connection Time**: 5-10 seconds typical
- **Channel Analysis**: <5 seconds quick scan, <30 seconds detailed
- **Latency Testing**: <50ms command processing
- **iPerf Throughput**: 10-20 Mbps TCP, configurable UDP rates
- **Accuracy**: ±1 dBm RSSI measurement precision

---

## Related Documentation

- [Command Reference](../reference/COMMAND_REFERENCE.md) - Complete command documentation
- [Usage Examples](../reference/USAGE_EXAMPLES.md) - Practical examples
- [Architecture Guide](../architecture/ARCHITECTURE.md) - Technical implementation details
- [Quick Start Guide](../README.md#quick-start) - Getting started
- [Channel Analysis Guide](../guides/CHANNEL_GUIDE.md) - Spectrum analysis
- [Latency Testing Guide](../guides/LATENCY_GUIDE.md) - Performance testing
- [iPerf Guide](../guides/IPERF_GUIDE.md) - Bandwidth measurement
- [Security Guide](../security/SECURITY_CONFIGURATION.md) - Security configuration
