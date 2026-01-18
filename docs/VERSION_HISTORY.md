# Version History

Complete changelog and release notes for all versions of ESP32 WiFi Utility.

## 🎉 What's New in v5.2.1

Version 5.2.1 is a **documentation patch release** that fixes image reference paths in the README to ensure proper display of the v5.2.0 TFT UI screenshots on GitHub.

### 🔧 **Documentation Fixes**

- **Image References**: Corrected all image paths in README.md to match actual filenames in `docs/images/`
- **Visual Documentation**: TFT UI screenshots now display properly on GitHub
- **Professional Presentation**: Accurate visual examples of v5.2.0 TFT interface redesign

**Corrected Image Paths:**

| Section | Corrected Filename |
|---------|-------------------|
| Welcome Screen | `WelcomScreen.png` |
| Connecting Screen | `STA_Connecting.png` |
| AP Initialize | `AP_Initialize.png` |
| WiFi Disabled | `WiFi_Disabled.png` |
| AP Mode Display | `AP_Info.png` |

### 🔗 **Quick Links**

- **Release Notes**: [v5.2.1 Details](RELEASE_NOTES_V5.2.1.md)
- **Changelog**: [Full Version History](../CHANGELOG.md)

---

## 🎉 What's New in v5.1.0

Version 5.1.0 adds **real-time Latency Testing UI** improvements, including live progress bars and statistics, along with a modern redesign of the Status page. It also introduces dedicated **Python and C++ UDP servers** for high-precision network testing.

### 📉 **Improved Latency & Jitter Testing**

- **Visual Progress Bar**: Watch tests run in real-time with a smooth progress indicator.
- **Live Statistics**: See Average Latency and Jitter values update dynamically as packets arrive.
- **Auto-Refresh**: No need to manually reload; the page updates automatically when tests finish.
- **Better Defaults**: Updated target ports (UDP:53, TCP:80) for more reliable out-of-the-box testing.

### 🛠️ **External Testing Tools**

Professional-grade tools for accurate network benchmarking:

- **Python Echo Server** (`scripts/udp_echo_server.py`): Quick and portable UDP testing server.
- **C++ Echo Server** (`pc_test_apps/udp_echo_server`): High-performance, low-latency server for Linux with real-time priority support.

### 📊 **Status Page Redesign**

- **Modern Grid Layout**: Key metrics organized into clean, responsive cards.
- **Visual Icons**: Intuitive icons for WiFi, Network, System, and Client status.
- **Improved Readability**: Clearer labels and formatted values for easier diagnostics.

---

## 🎉 What's New in v5.2.0

Version 5.2.0 introduces a **complete TFT display UI redesign** with modular architecture, color-coded status screens, and a branded welcome experience.

### 🎨 **TFT Display UI Redesign**

Professional user interface with intuitive visual feedback:

#### Welcome Screen

<p align="center">
  <img src="images/WelcomScreen.png" alt="ESP32 WiFi Utility Welcome Screen" width="300"/>
</p>

- **Branded Startup**: Purple ESP32 logo with "ESP32 WiFi Utility" title
- **Professional First Impression**: 2-second welcome screen on device boot
- **Consistent Branding**: Matches project identity across all interfaces

#### Color-Coded Status Screens

<p align="center">
  <img src="images/STA_Connecting.png" alt="Connecting Screen" width="200"/>
  <img src="images/AP_Initialize.png" alt="Initializing Access Point" width="200"/>
  <img src="images/WiFi_Disabled.png" alt="Disabled Screen" width="200"/>
</p>

**Visual Status System**:

| Color     | Status           | Usage                                    | Screen                                                      |
|-----------|------------------|------------------------------------------|-------------------------------------------------------------|
| 🔵 Blue   | Connecting/Idle  | Station mode connecting or idle          | "Station Mode" / "Connecting..." or "Station Mode" / "Idle" |
| 🟢 Green  | Connected/Active | Successful connection or AP initializing | "Connected!" or "Initializing Access Point"                 |
| 🔴 Red    | Disabled         | WiFi stopped                             | "Disabled!"                                                 |
| 🟡 Yellow | Idle Mode        | Idle without text                        | WiFi symbol only (centered)                                 |

**Status Screens Include**:

- **Connecting**: Blue WiFi icon with "Station Mode" / "Connecting..." text
- **Connected**: Green WiFi icon with "Connected!" (1-second confirmation)
- **Station Idle**: Blue WiFi icon with "Station Mode" / "Idle" text
- **Mode Idle**: Yellow WiFi icon, centered, no text
- **Initializing AP**: Green WiFi icon with "Initializing Access Point" text
- **Disabled**: Red WiFi icon with "Disabled!" text

#### AP Mode Display

<p align="center">
  <img src="images/AP_Info.png" alt="AP Mode with QR Code" width="300"/>
</p>

- **Comprehensive Information**: SSID, password, IP address, QR code
- **Client Count**: Real-time connected devices
- **Battery Status**: Visual battery indicator with percentage
- **Current Time**: Display timestamp
- **QR Code**: Instant device connection via mobile scan

### 🏗️ **Modular Architecture Improvements**

Technical enhancements for maintainability and code quality:

- **Modular `displayWiFiStatusScreen()` Function**:
  - Configurable icon color, text color, and dual-line text
  - Automatic vertical centering when text is omitted
  - Eliminates code duplication across all status screens
  - Single source of truth for WiFi icon display logic

- **Queue-Based Messaging**:
  - All TFT updates via FreeRTOS queue system
  - Non-blocking display operations
  - Clean separation between UI and business logic

- **Clean Architecture**:
  - Removed TFT dependencies from command interface
  - WiFi task handles mode switching and TFT updates
  - Improved separation of concerns across modules

- **Memory Efficiency**:
  - Shared bitmap rendering reduces flash usage
  - Two bitmaps: 50x50 WiFi icon (313 bytes), 60x60 ESP32 logo (450 bytes)
  - Total flash usage: 85.0% (1,225,857 bytes)
  - RAM usage: 16.4% (53,592 bytes)

### 📱 **Screen Modes**

Complete display mode system:

- `TFT_MODE_OFF` - Display off
- `TFT_MODE_AP` - Access Point with full information and QR code
- `TFT_MODE_STATION` - Connected station with signal strength
- `TFT_MODE_CONNECTING` - Blue connecting animation
- `TFT_MODE_IDLE` - Blue station idle with text
- `TFT_MODE_IDLE_YELLOW` - Yellow WiFi symbol, no text
- `TFT_MODE_DISABLED` - Red disabled indicator
- `TFT_MODE_STATUS` - General status display

---

## 🎉 What's New in v5.0.0

Version 5.0.0 introduces **TFT display enhancements**, **non-blocking WiFi operations**, **structured logging system**, and **QR code functionality** for significantly improved user experience and system responsiveness.

### 🔗 **QR Code Support for Station Mode**

Share WiFi credentials instantly with scannable QR codes:

- **Automatic QR Code Generation**: Creates QR code when connected to WiFi networks
- **WiFi Credential Sharing**: Mobile devices can scan and auto-connect to networks
- **Conditional Display**: QR code only shown when actively connected
- **Dynamic Updates**: Automatically clears on disconnection, reappears on reconnection
- **Consistent UX**: Same QR code experience for both AP and Station modes
- **Optimized Scanning**: White border and centered positioning for better mobile camera recognition
- **Standard Format**: Uses WIFI:T:WPA;S:<SSID>;P:<password>;; format
- **TFT Integration**: Displayed on both normal and reverse TFT variants

### ⚡ **Non-Blocking WiFi Connection**

Enhanced responsiveness with asynchronous WiFi operations:

- **Asynchronous Connection**: WiFi.begin() no longer blocks main loop for 10 seconds
- **Responsive Main Loop**: Device remains interactive during connection attempts
- **Dual Function Design**: `connectToNetwork()` initiates, `handleWiFiConnection()` monitors
- **Connection State Tracking**: SSID, password, start time, and attempt count tracked
- **Visual Feedback**: Progress indicators update every 100ms without blocking
- **10-Second Timeout**: Automatic failure detection with status reporting
- **Preserved Features**: All LED indicators, TFT updates, and web server auto-start maintained
- **FreeRTOS Compatible**: Uses proper task delay mechanisms for cooperative multitasking

### 📋 **Structured Logging System**

Professional logging framework with severity levels and component tags:

- **Four Log Levels**: LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR
- **Component Tags**: TAG_WIFI, TAG_AP, TAG_WEB, TAG_TASK for easy filtering
- **Runtime Configuration**: Adjustable log levels without recompilation
- **Consistent Formatting**: Timestamps and severity indicators
- **System-Wide Migration**: ~73 Serial.print statements replaced across codebase
- **User Interface Preserved**: Connection progress dots and user messages maintained
- **Enhanced Debugging**: Easier troubleshooting with filterable, structured logs
- **Production Ready**: Clean logs without excessive debug information

### 📊 **Enhanced Status Command**

Comprehensive network information at your fingertips:

- **Connection Status**: Clear Connected/Not Connected display
- **Network Details**: SSID, IP address, subnet mask, gateway, DNS servers
- **MAC Address**: Device hardware address for network identification
- **Signal Strength**: RSSI in dBm with quality indicators and emoji bars
- **Quality Classification**: Excellent/Good/Fair/Weak with visual feedback
- **WiFi Channel**: Current operating channel
- **Connection Uptime**: Time connected in HH:MM:SS format
- **Detailed Status Codes**: WiFi status codes when disconnected for troubleshooting
- **Lease Information**: DHCP lease details when available

### **FreeRTOS Task Architecture**

Enhanced multi-tasking with dedicated WiFi command task:

- **WiFi Command Task**: Asynchronous mode switching and command processing
- **Task-Based Queue**: Non-blocking command execution
- **Proper Sequencing**: Stop WiFi → Start Mode → Connect flow
- **Core 1 Pinned**: Dedicated processing core for WiFi operations
- **Priority 2**: Higher priority than display, lower than network stack
- **8KB Stack**: Sufficient memory for WiFi operations
- **Background Processing**: Commands execute without blocking main loop

### 🌐 **Web Server Improvements**

Enhanced responsiveness and broader compatibility:

- **10x Faster**: Main loop reduced from 100ms to 10ms delay
- **100 Hz handleClient()**: Up from 10 Hz for instant web response
- **CPU Sharing**: yield() calls for better task cooperation
- **No More Pending**: Eliminated "HTTP requests stuck at pending" issue
- **All Board Support**: Web server works on all variants including TFT-only boards
- **Station Mode Fix**: Web server properly starts in Station mode
- **Enhanced Monitoring**: Proactive state monitoring with 2-second throttling

### 🔧 **Bug Fixes & Refinements**

Critical fixes for reliability and consistency:

- **Connection Timing**: Fixed false "Connection Failed" with valid credentials
- **Proper Delays**: Added delay(500) in connection loop for WiFi negotiation
- **TFT Updates**: Fixed display not updating during Station mode connection
- **Mode Switching**: Proper startStationMode() call before connectToNetwork()
- **Conditional Compilation**: Fixed USE_NEOPIXEL → USE_WEBSERVER logic
- **QR Code Clearing**: Automatic removal when WiFi disconnects

---

## 🎉 What's New in v4.3.1

Version 4.3.1 is a **documentation-focused patch release** providing comprehensive Wiki documentation for all v4.3.0 features.

### 📚 **Comprehensive Wiki Documentation**

**New Wiki Pages** (2,500+ lines of documentation):

- **Port Scanner Documentation** (1,400+ lines)
  - Port scanning fundamentals and TCP connection concepts
  - Four scan types: Common (16 ports), Well-Known (1-1024), Custom Range, All Ports (65,535)
  - 25+ service identifications (HTTP, SSH, MySQL, RDP, VNC, etc.)
  - Web interface guide with detailed UI documentation
  - RESTful API endpoints with JavaScript integration examples
  - Real-world use cases: router security, web server audit, database checks, IoT discovery
  - Legal and ethical considerations, security best practices
  - Troubleshooting guide and technical implementation details

- **Signal Strength Monitor Documentation** (1,100+ lines)
  - RSSI fundamentals: dBm explained, quality ratings, signal interpretation
  - Serial commands: `signal show/scan/monitor` with detailed examples
  - Web interface with auto-refresh, color-coded meters, graphical bars
  - Use cases: troubleshooting, positioning, site surveys, stability monitoring
  - JSON API integration with JavaScript examples
  - Best practices for accurate measurements and signal optimization
  - Technical specifications, performance metrics, memory usage

**Updated Wiki Pages**:

- [Home](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Home) - Added navigation to new tools
- [Web Configuration Interface](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Web-Configuration-Interface) - Updated to 10 pages, added API endpoints
- [Command Reference](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Command-Reference) - Added signal commands
- [Quick Start Guide](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Quick-Start-Guide) - Updated feature list
- [Sidebar Navigation](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/_Sidebar) - Enhanced navigation

---

## 🎉 What's New in v4.3.0

Version 4.3.0 introduces **advanced network analysis tools**, **TFT display support**, and **visual spectrum analysis** for professional WiFi diagnostics and security auditing.

### 📶 **Signal Strength Monitor**

Real-time WiFi signal analysis with auto-scanning capabilities:

- **Live Signal Monitoring**: Current connection signal strength with 3-second auto-refresh
- **Nearby Networks Scanner**: Comprehensive scan of all networks with signal quality
- **Auto-Scan Mode**: Configurable intervals (5, 10, 15, 30, 60 seconds)
- **Hidden Network Detection**: Identifies and displays hidden SSIDs as `<Hidden Network>`
- **Visual Signal Quality**: Color-coded circles (Green/Yellow/Orange/Red)
- **Non-Blocking Scans**: Page content remains visible during scan updates
- **Real-Time Countdown**: Shows time until next automatic scan
- **Web Interface**: Professional UI at `/signal` with responsive design
- **RSSI to Quality**: Automatic conversion to percentage (0-100%)
- **Quality Text**: Excellent/Good/Fair/Weak/Very Weak classifications
- **Integration**: Added to Analysis Dashboard for quick access

### 🔒 **Port Scanner**

Network security auditing and open port detection:

- **Multiple Scan Types**:
  - Common Ports (16 most-used ports - fastest)
  - Well-Known Ports (1-1024)
  - Custom Range (user-defined start/end)
  - All Ports (1-65535 - comprehensive)
- **Service Identification**: Maps 25+ common services (HTTP, SSH, MySQL, RDP, etc.)
- **Real-Time Progress**: Animated progress bar with percentage
- **Background Scanning**: Non-blocking TCP connection tests
- **Results Display**: Tabular format with Port/Service/Status
- **Target Selection**: Auto-populated with gateway IP
- **Security Warnings**: Clear legal disclaimers and authorization reminders
- **Web Interface**: Professional purple-themed UI at `/portscan`
- **API Endpoints**: RESTful API for scan control and status
- **Integration**: Added to Analysis Dashboard with dedicated card

### 📊 **Channel Graph Visualization**

Visual WiFi spectrum analysis with interactive graph:

- **HTML5 Canvas Graph**: Professional bar chart showing all 2.4GHz channels
- **Color-Coded Visualization**:
  - 🟢 Green: Low congestion (0-40%)
  - 🟡 Yellow: Medium congestion (40-70%)
  - 🔴 Red: High congestion (70-100%)
- **Network Count Overlays**: Shows number of networks on each channel
- **Recommended Channels**: Starred (⭐) channels with lowest interference
- **Best Channel Detection**: Automatically identifies optimal channel
- **Interactive Legend**: Clear explanation of colors and metrics
- **Grid and Axes**: Professional graph layout with labeled axes
- **Non-Overlapping Indicators**: Highlights channels 1, 6, and 11
- **Responsive Design**: Adapts to screen size with window resize support
- **Web Interface**: Accessible at `/channel/graph`
- **Educational Content**: Detailed explanation of channel overlap

### 📱 **TFT Display Support**

Built-in screen support for Adafruit Feather ESP32-S3 TFT boards with comprehensive mode-specific displays and real-time signal strength visualization.

---

## 🎉 What's New in v4.2.0

Version 4.2.0 represents a **major architectural simplification** with removal of RTOS complexity and introduction of comprehensive configuration persistence, web-based configuration management, responsive UI improvements, and instant mode switching capabilities.

### **Architecture Simplification & Stability**

**Streamlined codebase** for better maintainability and reliability:

- **RTOS Removed**: Eliminated FreeRTOS complexity for easier development and improved stability
- **Simplified Architecture**: Direct loop-based processing for clearer code flow
- **Better Memory Management**: Improved heap management and reduced fragmentation
- **Enhanced Stability**: More predictable behavior with synchronous operations
- **Same Features**: All user-facing functionality retained and enhanced

### 💾 **Configuration Persistence System**

Complete NVS-based storage for Access Point and Station configurations with base64 password encryption.

### 🌐 **Web Configuration Interface**

Professional web-based configuration and management system with instant mode switching capabilities.

### 📱 **Responsive Web Interface Enhancements**

Mobile-optimized interface with adaptive navigation and professional gradient UI.

### 🔒 **Security Enhancements**

Base64 password encoding, password masking, and secure storage.

---

## 📋 What Was in v4.1.0

Version 4.1.0 represented a **major architectural transformation** with the introduction of FreeRTOS (later removed in v4.2.0 for simplification).

**Note**: FreeRTOS architecture was introduced in v4.1.0 but removed in v4.2.0 to improve stability and maintainability.

---

## 📋 What Was New in v4.0.0

### 🌐 Web Server on Both Boards

Web interface available on **both ESP32 Development Board and Feather ESP32-S3 TFT**

### 📱 Clickable Network Details (Issue #10)

Interactive WiFi scanning with detailed network information pages:

- 8-level signal quality scale
- Channel congestion analysis
- Security assessment with recommendations
- Smart caching (50 networks, 5 minutes)

### ⚡ Memory Optimization

Flash and RAM savings through compiler optimizations:

- ESP32dev: -27.7 KB flash saved
- Feather ESP32-S3: -25.4 KB flash saved
- PROGMEM storage and F() macros

### 🧪 Test Coverage

19 automated test cases for v4.0.0 features

---

For complete version history, see [CHANGELOG.md](../CHANGELOG.md).
