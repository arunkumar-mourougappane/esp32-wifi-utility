# ESP32 WiFi Utility Suite

![Build Status](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/actions/workflows/build.yml/badge.svg)
![Version](https://img.shields.io/badge/version-5.0.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-green.svg)
![PlatformIO](https://img.shields.io/badge/build-PlatformIO-orange.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Boards](https://img.shields.io/badge/boards-ESP32dev%20|%20Feather%20ESP32--S3%20|%20Reverse%20TFT-brightgreen.svg)
[![Changelog](https://img.shields.io/badge/changelog-available-brightgreen.svg)](CHANGELOG.md)

A professional-grade ESP32 WiFi analysis and management suite featuring comprehensive network scanning, signal strength monitoring, port scanning, visual spectrum analysis, TFT display support with QR codes, performance testing, tri-board support, persistent configuration storage, instant mode switching via web interface, non-blocking operations, structured logging, and FreeRTOS task-based architecture.

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

###  **FreeRTOS Task Architecture**

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

### 🔗 **Quick Links**

- **GitHub Wiki**: [Complete Documentation](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki)
- **Release Notes**: [v5.0.0 Details](docs/RELEASE_NOTES_V5.0.0.md)
- **Changelog**: [Full Version History](CHANGELOG.md)

---

## 🎉 What's New in v4.3.1

Version 4.3.1 is a **documentation-focused patch release** providing comprehensive Wiki documentation for all v4.3.0 features. This release significantly enhances user experience with detailed guides, API references, and best practices.

### 📚 **Comprehensive Wiki Documentation**

**New Wiki Pages** (2,500+ lines of documentation):

- **[Port Scanner Documentation](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Port-Scanner)** (1,400+ lines)

  - Port scanning fundamentals and TCP connection concepts
  - Four scan types: Common (16 ports), Well-Known (1-1024), Custom Range, All Ports (65,535)
  - 25+ service identifications (HTTP, SSH, MySQL, RDP, VNC, etc.)
  - Web interface guide with detailed UI documentation
  - RESTful API endpoints with JavaScript integration examples
  - Real-world use cases: router security, web server audit, database checks, IoT discovery
  - Legal and ethical considerations, security best practices
  - Troubleshooting guide and technical implementation details

- **[Signal Strength Monitor Documentation](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Signal-Strength-Monitor)** (1,100+ lines)
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

### 📊 **Documentation Highlights**

- **15+ API Integration Examples**: JavaScript code samples for all endpoints
- **10+ Use Cases**: Real-world scenarios with step-by-step guides
- **25+ Service Identifications**: Complete port-to-service mapping
- **8+ New Commands**: Serial interface command documentation
- **Complete Troubleshooting**: Common issues and solutions for all features
- **Security Guidelines**: Legal considerations and responsible disclosure

### 🔗 **Quick Links**

- **GitHub Wiki**: [Complete Documentation](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki)
- **Release Notes**: [v4.3.1 Details](docs/RELEASE_NOTES_V4.3.1.md)
- **Changelog**: [Full Version History](CHANGELOG.md)

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

Built-in screen support for Adafruit Feather ESP32-S3 TFT boards:

- **Mode-Specific Displays**:
  - **AP Mode**:
    - Green "AP Mode" indicator
    - QR code for easy device connection
    - SSID and password display
    - AP IP address
  - **Station Mode**:
    - Blue "Station Mode" indicator
    - Connection status
    - Connected network SSID
    - Local IP address
    - Real-time signal strength (RSSI in dBm)
    - Color-coded signal quality (Green/Yellow/Red)
    - Visual signal quality bar graph
- **Dynamic Updates**: Automatically updates based on WiFi mode
- **QR Code Generation**: Instant WiFi connection via QR scan
- **Signal Visualization**: Live signal strength with quality bar
- **Compact Layout**: Optimized for 135x240 pixel display
- **Board Support**:
  - Adafruit Feather ESP32-S3 TFT
  - Adafruit Feather ESP32-S3 Reverse TFT
- **Auto-Detection**: Automatically displays appropriate mode info

### 🌐 **Web Interface Enhancements**

Expanded Analysis Dashboard with new tools:

- **Signal Monitor Card**: Cyan-themed card with current signal info
- **Port Scanner Card**: Purple-themed card with last scan results
- **Channel Graph Button**: Quick access to visual spectrum
- **Quick Actions**: All tools accessible from one location
- **Updated Navigation**: Analysis dropdown includes all new tools
- **Responsive Design**: Mobile-optimized for all screen sizes
- **Professional Styling**: Consistent gradient themes throughout

### 🎯 **Integration & Accessibility**

- **Unified Dashboard**: All analysis tools in one place at `/analysis`
- **Navigation Menu**: Analysis dropdown includes Signal, Port Scanner, and Channel tools
- **Quick Actions**: One-click access to all diagnostic features
- **Tips Section**: Educational content for each tool
- **Status Cards**: Real-time status display for each tool
- **Color Themes**:
  - Cyan: Signal Monitor
  - Purple: Port Scanner
  - Green: Channel Graph
  - Purple/Blue: Other analysis tools

---

## 🎉 What's New in v4.2.0

Version 4.2.0 represents a **major architectural simplification** with removal of RTOS complexity and introduction of comprehensive configuration persistence, web-based configuration management, responsive UI improvements, and instant mode switching capabilities.

###  **Architecture Simplification & Stability**

**Streamlined codebase** for better maintainability and reliability:

- **RTOS Removed**: Eliminated FreeRTOS complexity for easier development and improved stability
- **Simplified Architecture**: Direct loop-based processing for clearer code flow
- **Better Memory Management**: Improved heap management and reduced fragmentation
- **Enhanced Stability**: More predictable behavior with synchronous operations
- **Same Features**: All user-facing functionality retained and enhanced

### 💾 **Configuration Persistence System**

Complete NVS-based storage for Access Point and Station configurations with base64 password encryption:

#### Access Point Configuration

- **Persistent Settings**: SSID, password (base64 encoded), channel, auto-start
- **Survives Reboots**: Configuration stored securely in non-volatile memory
- **Default Fallback**: Automatic defaults if no config saved
- **Serial Commands**: `ap config <ssid> <password> [channel] [auto]`
- **Web Interface**: Full configuration via `/config` page
- **Secure Storage**: Passwords encoded in base64 before NVS storage

#### Station Configuration

- **WiFi Credentials**: SSID and password (base64 encoded) storage
- **Auto-Connect**: Automatic connection on boot when enabled
- **Secure Storage**: Passwords never exposed in plain text, encoded in base64
- **Serial Commands**: `station config <ssid> <password> [auto]`
- **Web Interface**: Easy credential management with masked password fields
- **Password Privacy**: Saved passwords never displayed on web interface

#### Boot Behavior (Priority Order)

1. Check for saved AP config with auto-start enabled → Start AP mode
2. Check for saved Station config with auto-connect enabled → Connect to WiFi
3. No saved config or auto-start/connect disabled → Start in IDLE mode

### 🌐 **Web Configuration Interface**

Professional web-based configuration and management system:

#### Configuration Page Features (`/config`)

- **AP Configuration Section**:
  - SSID, password, channel (1-13), auto-start toggle
  - Save configuration that persists across reboots
  - Clear saved configuration option
- **Station Configuration Section**:
  - WiFi network credentials (SSID/password)
  - Auto-connect on boot toggle
  - Save configuration for automatic connection
  - Clear saved configuration option
- **Quick Mode Switch**:
  - Toggle between AP and Station modes without rebooting
  - Instant activation using saved configurations
  - Status display showing current mode
- **Reboot Modal**:
  - Countdown timer (10 seconds) with visual progress
  - Confirm/Cancel options for user control
  - Automatic page reload after reboot
- **Responsive Design**:
  - Works seamlessly on desktop, tablet, and mobile devices
  - Touch-friendly buttons and inputs
- **Password Security**:
  - Passwords masked in input fields
  - Saved passwords never displayed (shown as asterisks)
  - Base64 encoding for storage security
- **Real-time Validation**:
  - Immediate feedback on input errors
  - Form validation before submission
  - Clear error messages

#### API Endpoints

```
GET  /config              # Configuration page UI
POST /config/ap           # Save AP configuration
POST /config/station      # Save Station configuration
POST /config/clear        # Clear saved configurations
POST /reboot              # Reboot device with countdown
POST /mode/switch         # Instant mode switching (AP/Station)
```

### ⚡ **Instant Mode Switching**

Switch between Access Point and Station modes without device reboot:

- **One-Click Operation**: Toggle modes from web interface `/config` page
- **2-Second Switch**: Instant activation vs 10+ second reboot time
- **Smart Configuration**: Automatically uses saved settings if available
- **Visual Feedback**: Real-time status updates and confirmation messages
- **No Interruption**: Web interface remains accessible after switch
- **Fallback Handling**: Clear messaging if no saved configuration exists

**Quick Mode Toggle Benefits:**

- 📡 **Switch to Access Point** - Activates AP mode with saved config immediately
- 📶 **Switch to Station** - Connects to saved WiFi network instantly
- Current mode prominently displayed
- Status messages confirm successful operations
- Error handling for missing configurations

### 📱 **Responsive Web Interface Enhancements**

Mobile-optimized interface with adaptive navigation:

- **Desktop View**: Horizontal navigation bar with full menu visible
- **Mobile View**: Hamburger menu (☰) with collapsible navigation
- **Touch-Friendly**: 44px minimum touch targets for easy interaction
- **Flexible Layout**: Adapts seamlessly to all screen sizes (320px to 4K)
- **Professional Design**: Modern gradient UI with intuitive icons
- **Smooth Animations**: CSS transitions for polished user experience
- **Accessible**: Clear labeling and keyboard navigation support

**Responsive Navigation Features:**

- Hamburger icon automatically shown on screens < 768px width
- Click/tap to toggle mobile menu visibility
- Smooth slide-in/out animations
- Dropdown menus adapt to mobile context
- All pages fully responsive and mobile-optimized

### 🔒 **Security Enhancements**

- **Base64 Password Encoding**: All passwords encoded before NVS storage
- **Password Masking**: Web UI never displays saved passwords in plain text
- **Secure Decoding**: Passwords decoded only when needed for WiFi operations
- **Input Validation**: Server-side validation of all configuration parameters

### 🐛 **Critical Bug Fixes**

- **Heap Corruption**: Fixed WiFi operation crashes with better memory management
- **NVS Operations**: Enhanced error checking and recovery for configuration storage
- **Web Server Stability**: Improved handling of concurrent requests
- **Mode Switching**: Fixed race conditions during WiFi mode transitions

---

## 📋 What Was in v4.1.0

Version 4.1.0 represented a **major architectural transformation** with the introduction of FreeRTOS (now removed in v4.2.0 for simplification).

---

## 📋 What Was in v4.1.0

Version 4.1.0 represents a **major architectural transformation** with the introduction of FreeRTOS, delivering unprecedented
performance, responsiveness, and reliability.

###  **MAJOR: FreeRTOS-Based Architecture** (Issues #12-#20)

Complete reimplementation with **professional FreeRTOS task-based architecture**:

#### Core RTOS Infrastructure (#13)

- **Dual-Core Task Distribution**: Core 0 for network (WiFi, Web Server), Core 1 for application (Commands, LED, Analysis)
- **5-Level Priority System**: HIGHEST (Commands) → HIGH (WiFi) → MEDIUM (Web) → LOW (LED) → VERY_LOW (Analysis)
- **Queue-Based Communication**: Type-safe, thread-safe messaging with 4 system queues (Command, WiFi Event, Status, Analysis)
- **Mutex Protection**: Automatic resource protection with deadlock prevention (WiFi, WebServer, ScanResults, Serial mutexes)
- **Task Monitoring**: Real-time statistics on stack usage, loop times, CPU utilization
- **Graceful Error Recovery**: Tasks restart independently without system reboot

#### Performance Improvements

| Metric                    | v3.x (Synchronous) | v4.1.0 (RTOS)     | Improvement           |
| ------------------------- | ------------------ | ----------------- | --------------------- |
| **Command Response**      | Blocking (3-5s)    | **<10ms**         | **500× faster**       |
| **WiFi + Web Server**     | Sequential         | **Concurrent**    | **2× throughput**     |
| **LED Updates**           | Stuttering         | **60 FPS**        | **Smooth animation**  |
| **CPU Utilization**       | 80% single-core    | **40% dual-core** | **50% reduction**     |
| **System Responsiveness** | Poor (blocked)     | **Excellent**     | **Always responsive** |

#### Task Implementation

- **Command Task** (#14): Asynchronous command processing on Core 1, Priority HIGHEST
- **WiFi Task** (#15): Network operations on Core 0, Priority HIGH, event-driven architecture
- **Web Server Task** (#16): Concurrent HTTP handling on Core 0, Priority MEDIUM
- **Analysis Task** (#17): Background diagnostics on Core 1, Priority VERY_LOW
- **LED Task** (#18): Smooth 60 FPS animations on Core 1, Priority LOW

### 💡 **LED Controller Task Enhancement** (#18)

Professional LED control with smooth animations and multiple states:

- **60 FPS Smooth Animations**: Dedicated task ensures consistent, jitter-free updates
- **10 LED States**: Idle, Scanning, Connecting, Connected, Disconnected, AP Mode, Error, Analysis, Custom, Off
- **5 Animation Patterns**: Solid, Pulse (sine wave), Blink, Fade, Flash
- **Priority-Based State Changes**: Errors override normal states, smooth transitions
- **Multi-Hardware Support**: Standard LED (ESP32dev) and NeoPixel RGB (Feather ESP32-S3 TFT/Reverse TFT)
- **Queue-Based Updates**: Non-blocking state changes from any task
- **Customizable Colors**: Full RGB control for NeoPixel boards

### 🧪 **RTOS Testing Suite** (#19)

Comprehensive test coverage ensuring reliability and performance:

- **59 Automated Tests**: Unit, integration, performance, and stress tests
- **~90% Code Coverage**: 650/720 lines of RTOS code tested
- **6 Test Categories**:
  - Queue Operations (12 tests): Creation, send/receive, overflow, FIFO, timeout, concurrent access
  - Task Management (11 tests): Lifecycle, priorities, core affinity, stack monitoring
  - Mutex Operations (11 tests): Lock/unlock, timeout, contention, fairness
  - Integration Tests (9 tests): Inter-task communication, queue chaining, error handling
  - Performance Tests (8 tests): Latency, throughput, timing benchmarks
  - Stress Tests (8 tests): Queue flooding, high load, memory pressure, long-running stability
- **All Performance Targets Met**:
  - Queue latency: <1ms ✅
  - Mutex operations: <100μs ✅
  - Command throughput: >100/sec ✅
  - Task switching: <1ms ✅
  - End-to-end latency: <10ms ✅
- **Memory Stable**: No leaks detected, <5KB variation under load
- **Test Documentation**: Complete results in `test/RTOS_TEST_RESULTS.md`

### 📚 **Comprehensive RTOS Documentation** (#20)

Professional documentation suite (~5000 lines) for developers:

- **[RTOS Architecture Guide](docs/technical/RTOS_ARCHITECTURE.md)** (~800 lines): Complete system design, task structure,
  queue flows with diagrams, synchronization primitives, CPU core assignments, memory management
- **[RTOS API Reference](docs/technical/RTOS_API_REFERENCE.md)** (~1400 lines): 100% API coverage with examples for
  RTOSManager, QueueManager, MutexManager, TaskBase, and all task classes
- **[RTOS Migration Guide](docs/user-guides/RTOS_MIGRATION_GUIDE.md)** (~850 lines): v3.x to v4.x upgrade guide with
  behavioral changes, API changes, step-by-step migration, troubleshooting
- **[Tutorial: Creating a New Task](docs/technical/RTOS_TUTORIAL_NEW_TASK.md)** (~500 lines): Step-by-step
  task implementation with complete example
- **[Tutorial: Using Queues](docs/technical/RTOS_TUTORIAL_QUEUES.md)** (~450 lines): Inter-task communication patterns
  and best practices
- **[Tutorial: Debugging RTOS](docs/technical/RTOS_TUTORIAL_DEBUGGING.md)** (~400 lines): Tools, techniques,
  and common issue resolution
- **[RTOS FAQ](docs/user-guides/RTOS_FAQ.md)** (~550 lines): 40+ common questions answered

### ✅ **Backward Compatibility**

All existing features work identically - **no changes required** for basic usage:

- ✅ All serial commands unchanged
- ✅ Web interface operates the same
- ✅ Configuration format compatible
- ✅ Saved credentials preserved

**What changed internally**:

- Commands execute asynchronously (non-blocking)
- Multiple operations run concurrently
- Event-driven architecture replaces polling
- Better error recovery and system stability

### 🎯 Build Results

**All configurations build successfully**:

- **ESP32dev (RTOS)**: RAM 16.5%, Flash 86.2% ✅
- **Adafruit Feather (RTOS)**: RAM 16.1%, Flash 74.8% ✅
- **ESP32dev (Legacy)**: RAM 15.8%, Flash 83.1% ✅

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

## 🎯 **NEW in v4.1.0: FreeRTOS Architecture**

Version 4.1.0 introduces a comprehensive **FreeRTOS-based architecture**, transforming the ESP32 WiFi Utility from synchronous
blocking operations to **asynchronous, concurrent task-based processing**.

###  Key RTOS Features

- **Asynchronous Command Execution**: Commands return immediately, operations run in background
- **Concurrent Operations**: WiFi scanning, web server, and LED animations all run simultaneously
- **Dual-Core Task Distribution**: Core 0 for network operations, Core 1 for application logic
- **Responsive User Interface**: No blocking - system always responsive to commands
- **Priority-Based Scheduling**: 5-level priority system ensures critical tasks execute first
- **Queue-Based Communication**: Type-safe, thread-safe messaging between tasks
- **Mutex Protection**: Automatic resource protection with deadlock prevention
- **Task Monitoring**: Real-time statistics on stack usage, loop times, and performance
- **Graceful Error Recovery**: Tasks can restart independently without system reboot
- **Smooth LED Animations**: Dedicated LED task provides 60 FPS animations

### 📊 Performance Improvements

| Metric                | v3.x (Synchronous) | v4.1.0 (RTOS) | Improvement           |
| --------------------- | ------------------ | ------------- | --------------------- |
| Command Response      | Blocking (3-5s)    | <10ms         | **500x faster**       |
| WiFi + Web Server     | Sequential         | Concurrent    | **2x throughput**     |
| LED Updates           | Stuttering         | 60 FPS        | **Smooth**            |
| CPU Utilization       | 80% single-core    | 40% dual-core | **50% reduction**     |
| System Responsiveness | Poor (blocked)     | Excellent     | **Always responsive** |

### 🏗️ RTOS Architecture

```
┌─────────── Core 0 (Protocol CPU) ───────────┐    ┌────────── Core 1 (Application CPU) ──────────┐
│                                              │    │                                              │
│  WiFi Task        (Priority: HIGH)          │    │  Command Task     (Priority: HIGHEST)        │
│  Web Server Task  (Priority: MEDIUM)        │    │  LED Task         (Priority: LOW)            │
│                                              │    │  Analysis Task    (Priority: VERY_LOW)       │
│                                              │    │                                              │
└──────────────────┬───────────────────────────┘    └─────────────────┬─────────────────────────┘
                   │                                                   │
                   └───────────────► Queues & Mutexes ◄───────────────┘
                                   (Thread-Safe Communication)
```

### 📚 Comprehensive RTOS Documentation

- **[RTOS Architecture Guide](docs/technical/RTOS_ARCHITECTURE.md)** - Complete system design, task structure, queue flows
- **[RTOS API Reference](docs/technical/RTOS_API_REFERENCE.md)** - Full API documentation with examples
- **[RTOS Migration Guide](docs/user-guides/RTOS_MIGRATION_GUIDE.md)** - Upgrade from v3.x to v4.x
- **[Tutorial: Creating a New Task](docs/technical/RTOS_TUTORIAL_NEW_TASK.md)** - Step-by-step task implementation
- **[Tutorial: Using Queues](docs/technical/RTOS_TUTORIAL_QUEUES.md)** - Inter-task communication patterns
- **[Tutorial: Debugging RTOS](docs/technical/RTOS_TUTORIAL_DEBUGGING.md)** - Debugging tools and techniques
- **[RTOS FAQ](docs/user-guides/RTOS_FAQ.md)** - Common questions and solutions
- **[Test Results](test/RTOS_TEST_RESULTS.md)** - 59 tests, ~90% coverage, all performance targets met

### ✅ RTOS Quality Metrics

- **59 Automated Tests**: Comprehensive coverage of all RTOS components
- **~90% Code Coverage**: 650/720 lines of RTOS code tested
- **All Performance Targets Met**:
  - Queue latency: <1ms ✅
  - Mutex operations: <100μs ✅
  - Command throughput: >100/sec ✅
  - Task switching: <1ms ✅
  - End-to-end latency: <10ms ✅
- **Memory Stable**: No leaks detected, <5KB variation under load
- **Production Ready**: Tested on ESP32dev and Adafruit Feather ESP32-S3

### 🔄 Backward Compatibility

**All existing features work identically** - no changes required for basic usage:

- ✅ All serial commands unchanged
- ✅ Web interface operates the same
- ✅ Configuration format compatible
- ✅ Credentials preserved

**What changed internally**:

- Commands execute asynchronously (non-blocking)
- Multiple operations run concurrently
- Event-driven architecture replaces polling
- Better error recovery and system stability

## � Features

### 🎯 **NEW: Professional Channel Analysis System**

- **Real-time Spectrum Scanning**: Complete 2.4GHz band analysis (channels 1-14)
- **Advanced Congestion Analysis**: AI-powered 0-100% congestion scoring
- **Channel Overlap Detection**: Mathematical interference analysis and optimization
- **Smart Recommendations**: Automated optimal channel selection with rationale
- **Interference Classification**: Microwave, Bluetooth, and continuous wave detection
- **Background Monitoring**: Configurable 1-60 second monitoring intervals
- **Professional Reporting**: JSON export and detailed optimization reports
- **Visual Feedback**: Real-time LED status indication during analysis

### 📊 **NEW: Network Performance & Latency Analysis**

- **Advanced Latency Testing**: Comprehensive ping-style analysis with jitter calculations
- **Statistical Analysis**: Mean, median, min/max latency with standard deviation
- **Packet Loss Detection**: Real-time packet loss monitoring and reporting
- **Network Quality Assessment**: Professional network performance scoring
- **Historical Tracking**: Trend analysis and performance monitoring over time

### 🔧 **NEW: Dual-Board Hardware Support**

- **ESP32 Development Board**: Full feature set with built-in LED control
- **Adafruit Feather ESP32-S3 TFT**: Enhanced with NeoPixel RGB LED integration
- **Conditional Compilation**: Hardware-specific optimizations and features
- **Memory Optimized**: Efficient resource usage across both platforms
- **Visual Feedback**: Board-appropriate status indication (LED/NeoPixel)

### 📡 Multiple WiFi Modes

- **Idle Mode**: Standby state - ready but not consuming WiFi resources
- **Station Mode**: Scan for nearby WiFi networks with comprehensive analysis
- **Access Point Mode**: Create a WiFi hotspot for other devices to connect
- **Off Mode**: Completely disable WiFi functionality
- **Seamless switching** between all modes via serial commands

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
- **Advanced Channel Commands**:
  - `channel scan` - Quick spectrum analysis (<5 seconds)
  - `channel scan detailed` - Comprehensive analysis (<30 seconds)
  - `channel monitor start/stop` - Background monitoring
  - `channel recommend` - AI-powered optimal channel suggestions
  - `channel export` - JSON data export for analysis
- **Network Statistics**: Summary of security types, signal strengths, and channel usage
- **Interactive Commands**: `scan now` for immediate analysis, `scan info <id>` for detailed examination
- Real-time scanning control (on/off) with configurable intervals

### 📱 Access Point Features

- Creates WiFi hotspot with configurable SSID and password
- **QR Code Generation**: Automatic QR code display for instant mobile connection
- Real-time connected client monitoring with MAC addresses and signal strength
- Client management: list, disconnect individual clients or all clients
- Deauthentication capability for access control and testing
- Displays comprehensive AP network information
- Easy connection for mobile devices and computers

### 🌐 **Web Server Interface** (Available on Both Boards!)

- **Browser-Based Control**: Access device features via any web browser on ESP32dev or Feather
- **Mobile-Responsive Design**: Beautiful interface optimized for phones, tablets, and desktops
- **Hierarchical Navigation**: Professional dropdown menu system for organized access
- **Progress Indicators**: Visual feedback with backdrop overlay during scan operations
- **Real-Time Monitoring**: Live system status, WiFi statistics, and client information
- **Network Visualization**: Interactive WiFi scanning with clickable network details
- **Clickable Network Details**: Click any scanned network to view comprehensive information
  - 8-level signal quality assessment with recommendations
  - Channel congestion analysis (Clear to Severe ratings)
  - Security evaluation with encryption type details
  - Connection recommendations based on signal and channel conditions
  - Visual indicators with emoji icons and color-coded ratings
- **Professional Dashboard Pages**:
  - 🏠 **Home** - Quick stats and system overview with unified metrics
  - 📊 **Status** - Detailed system information and configuration
  - 🔍 **Scan Networks** - Interactive WiFi scanning with clickable network details (v4.0.0)
    - Click any network to view comprehensive information
    - Signal quality assessment with 8-level scale
    - Channel congestion analysis and recommendations
    - Security evaluation with encryption details
  - 🔬 **Analysis Dashboard** - Unified testing hub with quick actions
    - ⚡ iPerf Testing - Bandwidth measurement and throughput analysis
    - 📉 Latency Testing - Network quality and jitter analysis
    - 📡 Channel Analysis - Spectrum analysis and congestion monitoring
- **Modern UI/UX Features**:
  - Gradient design with professional styling and smooth animations
  - Dropdown menus with hover states and mobile-responsive behavior
  - Full-screen progress overlays with animated spinners
  - Smart status badges for connection state and mode
  - Responsive grid layouts and card-based design
- **Advanced Functionality**:
  - Zero Configuration: Automatic IP detection and URL generation
  - Multi-Mode Support: Works in both AP mode and Station mode
  - Interactive Forms: Configure and launch tests from web interface
  - Real-time Statistics: Live bandwidth, latency, and channel metrics
  - Button-Triggered Scans: Prevent duplicate operations with progress feedback
- **Access Control**:
  - In AP mode: Accessible to connected clients at AP IP
  - In Station mode: Accessible on local network at device IP
- **Simple Commands**:
  - `webserver start` - Launch web interface
  - `webserver stop` - Stop web server
  - `webserver status` - Get access URL and server state

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

## 📚 Comprehensive Documentation Suite

This project includes professional-grade documentation covering all features and advanced capabilities:

### 📖 Core Feature Documentation

| Document                                                             | Description                                             | Key Topics                                                         |
| -------------------------------------------------------------------- | ------------------------------------------------------- | ------------------------------------------------------------------ |
| **[📚 Documentation Index](docs/README.md)**                         | **Complete documentation portal**                       | **All guides and technical documentation**                         |
| **[Channel Analysis Guide](docs/user-guides/CHANNEL_GUIDE.md)**      | Complete guide to professional spectrum analysis        | AI recommendations, congestion analysis, interference detection    |
| **[Latency Testing Guide](docs/user-guides/LATENCY_GUIDE.md)**       | Advanced network latency and jitter analysis            | Performance metrics, statistical analysis, quality assessment      |
| **[Enhanced WiFi Scanning](docs/user-guides/ENHANCED_SCANNING.md)**  | Advanced network scanning features and analysis         | Visual indicators, security analysis, network statistics           |
| **[iPerf Network Testing](docs/user-guides/IPERF_GUIDE.md)**         | Network performance measurement and throughput analysis | TCP/UDP testing, bandwidth measurement, diagnostics                |
| **[Automated Release System](docs/technical/AUTOMATED_RELEASES.md)** | GitHub Actions automated release pipeline               | Version-based releases, dual-board builds, professional automation |

### 🔧 Technical Implementation Guides

| Document                                                                       | Description                                            | Key Topics                                                     |
| ------------------------------------------------------------------------------ | ------------------------------------------------------ | -------------------------------------------------------------- |
| **[Test Infrastructure Guide](docs/technical/TEST_INFRASTRUCTURE.md)**         | Comprehensive testing framework for dual-board support | Unity testing, CI/CD, hardware validation                      |
| **[Channel Implementation Details](docs/technical/CHANNEL_IMPLEMENTATION.md)** | Technical deep-dive into spectrum analysis algorithms  | Mathematical models, interference patterns, optimization       |
| **[Implementation Summary](docs/technical/IMPLEMENTATION_SUMMARY.md)**         | Overview of all system enhancements and architecture   | Module structure, performance metrics, technical specs         |
| **[Project Completion Summary](docs/technical/PROJECT_COMPLETION_SUMMARY.md)** | Comprehensive project status and feature completion    | Achievement tracking, performance benchmarks, deployment guide |

### 🎯 Quick Start Guides

- **[📚 Documentation Portal](docs/README.md)** - **Central hub for all documentation**
- **[Channel Analysis Quick Start](docs/user-guides/CHANNEL_GUIDE.md#quick-start)** - `channel scan`, `channel recommend`
  with AI-powered analysis
- **[Latency Testing Quick Start](docs/user-guides/LATENCY_GUIDE.md#basic-usage)** - Network performance and jitter measurement
- **[Dual-Board Setup](docs/technical/TEST_INFRASTRUCTURE.md#dual-board-support)** - ESP32dev and Feather ESP32-S3 TFT configuration
- **[Test Suite Execution](docs/technical/TEST_INFRASTRUCTURE.md#running-tests)** - Automated testing and validation procedures

### 📊 Feature Highlights

- **🎯 Professional Spectrum Analysis**: AI-powered channel recommendations with 0-100% congestion scoring
- **� Advanced Performance Testing**: Comprehensive latency analysis with jitter calculations and statistical reporting
- **🔧 Tri-Board Support**: ESP32dev, Adafruit Feather ESP32-S3 TFT, and Reverse TFT with hardware-specific optimizations
- **🧪 Enterprise Testing**: Unity framework with comprehensive test coverage for both platforms
- **📱 Visual Feedback**: Smart LED/NeoPixel status indication with real-time analysis progress

## 🏗️ Professional Modular Architecture

This project features a **enterprise-grade, modular codebase** designed for maintainability, scalability, and extensibility:

### 📦 Core Modules

- **`config.h`** - Centralized configuration (pins, timings, credentials, board detection)
- **`wifi_manager`** - WiFi state management and enhanced network scanning
- **`ap_manager`** - Access Point client management and deauthentication
- **`iperf_manager`** - Network performance testing with TCP/UDP support
- **`led_controller`** - Visual status indication via LED/NeoPixel (board-aware)
- **`command_interface`** - Interactive serial command processing with full integration
- **`main.cpp`** - Clean entry point (setup/loop only)

### 🆕 Advanced Analysis Modules

- **`channel_analyzer`** - **NEW**: Professional spectrum analysis engine (570+ lines)
  - Real-time 2.4GHz band scanning and congestion analysis
  - AI-powered channel recommendation algorithms
  - Interference pattern detection and classification
  - JSON export and comprehensive reporting capabilities
- **`latency_analyzer`** - **NEW**: Network performance and jitter analysis
  - Advanced ping-style latency measurement with statistical analysis
  - Packet loss detection and network quality assessment
  - Historical tracking and trend analysis capabilities

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

## 🛠️ Hardware Requirements & Tri-Board Support

### Supported Boards

#### Option 1: ESP32 Development Board (Standard)

- **Board**: Any ESP32dev variant (DevKitC, NodeMCU-32S, etc.)
- **LED**: Built-in LED on GPIO 2 (standard on most ESP32 boards)
- **Memory**: 4MB Flash, 320KB RAM minimum
- **Features**: Full functionality with standard LED status indication

#### Option 2: Adafruit Feather ESP32-S3 TFT (Enhanced) 🆕

- **Board**: Adafruit Feather ESP32-S3 TFT with 4MB Flash
- **LED**: NeoPixel RGB LED on GPIO 48 (enhanced color status indication)
- **Display**: 1.14" Color TFT Display (240x135) - _Future enhancement planned_
- **Features**: Enhanced visual feedback with RGB color coding for analysis status
- **Memory**: 8MB PSRAM, enhanced performance for complex analysis

#### Option 3: Adafruit Feather ESP32-S3 Reverse TFT (Enhanced) 🆕

- **Board**: Adafruit Feather ESP32-S3 Reverse TFT with 4MB Flash
- **LED**: NeoPixel RGB LED on GPIO 48 (enhanced color status indication)
- **Display**: 1.14" Color TFT Display (240x135) with reverse orientation - _Future enhancement planned_
- **Features**: Enhanced visual feedback with RGB color coding for analysis status, optimized for reverse TFT layout
- **Memory**: 8MB PSRAM, enhanced performance for complex analysis

### Universal Requirements

- **USB Cable** for programming and serial communication
- **Computer** with PlatformIO or Arduino IDE
- **Serial Terminal** for command interface (115200 baud)

## 📦 Pre-Built Firmware Downloads

**🎯 Want to use it immediately?** Download pre-built firmware from our [latest release](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/releases/latest):

- **ESP32dev**: `esp32-wifi-utility-esp32dev-v{version}.bin` - Standard ESP32 development board
- **Feather ESP32-S3 TFT**: `esp32-wifi-utility-feather-s3-tft-v{version}.bin` - Enhanced with NeoPixel support
- **Feather ESP32-S3 Reverse TFT**: `esp32-wifi-utility-feather-s3-reversetft-v{version}.bin` - Enhanced with NeoPixel  
  support and reverse TFT layout

### Flash Pre-Built Firmware

```bash
# Using esptool (install with: pip install esptool)
esptool.py --port /dev/ttyUSB0 write_flash 0x10000 firmware.bin

# Or use Arduino IDE / PlatformIO for uploading
```

### 🤖 Automated Releases

Every version bump in `platformio.ini` automatically triggers a new release with fresh firmware builds for all boards.
See our [Automated Release Documentation](docs/technical/AUTOMATED_RELEASES.md) for details.

## ⚡ Quick Start

### 📚 Documentation

For comprehensive guides and API references, visit the [GitHub Wiki](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki):

- **[Quick Start Guide](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Quick-Start-Guide)** - Get started in minutes
- **[Port Scanner](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Port-Scanner)** - Network security auditing
- **[Signal Strength Monitor](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Signal-Strength-Monitor)** - WiFi optimization
- **[Command Reference](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Command-Reference)** - Complete command list
- **[Web Configuration](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki/Web-Configuration-Interface)** - Browser interface guide

### 1. Hardware Setup

#### ESP32 Development Board Setup:

1. Connect your ESP32dev to your computer via USB
2. Ensure the built-in LED is available (GPIO 2)

#### Adafruit Feather ESP32-S3 TFT Setup:

1. Connect your Feather board to your computer via USB
2. The NeoPixel LED will provide enhanced RGB status indication

### 2. Software Setup

1. Install [PlatformIO](https://platformio.org/) or [Arduino IDE](https://www.arduino.cc/en/software)
2. Clone or download this project
3. Open the project in your IDE

### 3. Build and Upload (Tri-Board Support)

#### Building for ESP32 Development Board:

```bash
# Using PlatformIO (recommended)
pio run -e esp32dev -t upload

# Build only to check compilation
pio run -e esp32dev

# Monitor serial output
pio device monitor
```

#### For Adafruit Feather ESP32-S3 TFT:

```bash
# Using PlatformIO with NeoPixel support
pio run -e adafruit_feather_esp32s3_tft -t upload

# Build only to check compilation
pio run -e adafruit_feather_esp32s3_tft

# Monitor serial output
pio device monitor
```

#### For Adafruit Feather ESP32-S3 Reverse TFT:

```bash
# Using PlatformIO with NeoPixel support
pio run -e adafruit_feather_esp32s3_reversetft -t upload

# Build only to check compilation
pio run -e adafruit_feather_esp32s3_reversetft

# Monitor serial output
pio device monitor
```

#### Build All Configurations:

```bash
# Build all board configurations
pio run

# Run comprehensive test suite
./run_tests_build_only.sh
```

#### Using Arduino IDE:

**File → Open → select main.cpp** (ESP32dev configuration only)

**Note**: The project uses a professional modular structure with multiple source files.
PlatformIO automatically handles the compilation of all `.cpp` files in the `src/` directory and applies board-specific configurations.

### 4. Running Tests (Optional)

#### Run Automated Test Suite

```bash
# Test on ESP32 Development Board
pio test --environment test

# Test on Feather ESP32-S3 TFT
pio test --environment test_feather

# Build tests without uploading (no hardware required)
pio test --environment test --without-uploading
pio test --environment test_feather --without-uploading
```

**Test Coverage**: 19 test cases covering:

- Cache management and validation
- Network details and signal quality
- WiFi fundamentals (RSSI, channels, encryption)
- System integration and memory management

**Test Results**: All tests pass on both boards (~14 seconds on ESP32, ~5.5 seconds on Feather)

See [Test Documentation](test/TEST_DOCUMENTATION.md) for detailed information.

### 5. Connect to Serial Monitor

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

### 🆕 Access Point Configuration Commands (v4.2.0)

| Command                                        | Description                                       |
| ---------------------------------------------- | ------------------------------------------------- |
| `ap config <ssid> <password> [channel] [auto]` | Save AP configuration (persistent across reboots) |
| `ap config load`                               | Display current saved AP configuration            |
| `ap config clear`                              | Clear saved AP configuration                      |
| `ap start`                                     | Start AP with saved config (or defaults if none)  |

**Parameters:**

- `ssid`: Network name (1-32 characters)
- `password`: WPA2 password (8-63 characters)
- `channel`: WiFi channel 1-13 (optional, default: 1)
- `auto`: Enable auto-start on boot (optional, default: false)

**Examples:**

```bash
ap config "MyHotspot" "SecurePass123" 6 auto  # Save with auto-start
ap start                                       # Start with saved config
ap config load                                 # Show saved config
ap config clear                                # Clear configuration
```

### 🆕 Station Configuration Commands (v4.2.0)

| Command                                   | Description                                     |
| ----------------------------------------- | ----------------------------------------------- |
| `station config <ssid> <password> [auto]` | Save station config (persistent across reboots) |
| `station config load`                     | Display current saved station configuration     |
| `station config clear`                    | Clear saved station configuration               |
| `station connect`                         | Connect using saved config (or prompt if none)  |

**Parameters:**

- `ssid`: Network name to connect to
- `password`: Network password
- `auto`: Enable auto-connect on boot (optional, default: false)

**Examples:**

```bash
station config "HomeNetwork" "WiFiPass123" auto  # Save with auto-connect
station connect                                   # Connect with saved config
station config load                               # Show saved config
station config clear                              # Clear configuration
```

### Scanning Commands (Station Mode)

| Command          | Description                                     |
| ---------------- | ----------------------------------------------- |
| `scan on`        | Start automatic WiFi network scanning           |
| `scan off`       | Stop WiFi scanning                              |
| `scan now`       | Immediate detailed scan with enhanced analysis  |
| `scan info <id>` | Show comprehensive details for specific network |

### 🆕 Professional Channel Analysis Commands

| Command                 | Description                                                 |
| ----------------------- | ----------------------------------------------------------- |
| `channel scan`          | Quick spectrum analysis of all 2.4GHz channels (<5 seconds) |
| `channel scan quick`    | Fast 5-second channel congestion scan                       |
| `channel scan detailed` | Comprehensive 30-second spectrum analysis                   |
| `channel monitor start` | Begin continuous background channel monitoring              |
| `channel monitor stop`  | End background monitoring                                   |
| `channel recommend`     | AI-powered optimal channel recommendations with rationale   |
| `channel export`        | Export scan data to JSON format for external analysis       |
| `channel status`        | Show current monitoring status and recent scan results      |
| `channel help`          | Display comprehensive channel analysis command reference    |

### 🆕 Network Performance & Latency Analysis

| Command                  | Description                                     |
| ------------------------ | ----------------------------------------------- |
| `latency test <host>`    | Comprehensive latency test with jitter analysis |
| `latency monitor <host>` | Continuous latency monitoring with statistics   |
| `latency report`         | Generate detailed network performance report    |
| `latency help`           | Display latency analysis command reference      |

💡 **Professional Analysis**: The new channel and latency analysis systems provide enterprise-grade
network diagnostics with AI-powered recommendations. See [Channel Analysis Guide](CHANNEL_GUIDE.md) and
[Latency Testing Guide](LATENCY_GUIDE.md) for complete details.

### Network Connection Commands (Station Mode)

| Command                     | Description                                          |
| --------------------------- | ---------------------------------------------------- |
| `connect <ssid> <password>` | Connect to a WiFi network with specified credentials |
| `disconnect`                | Disconnect from current WiFi network                 |

### Network Performance Testing Commands

| Command        | Description                      |
| -------------- | -------------------------------- |
| `iperf`        | Show iPerf performance test help |
| `iperf status` | Show current iPerf test status   |

💡 **Network Performance Testing**: The iPerf functionality provides comprehensive network bandwidth and performance
analysis with both TCP and UDP protocols. See [iPerf Testing Guide](IPERF_GUIDE.md) for detailed usage, configuration
options, and examples.

### Information Commands

| Command      | Description                                                  |
| ------------ | ------------------------------------------------------------ |
| `status`     | Show current device status and statistics                    |
| `ap info`    | Show detailed Access Point information (AP mode only)        |
| `ap clients` | List all connected clients with MAC addresses (AP mode only) |
| `qr`         | Display QR code for easy mobile connection (AP mode only)    |
| `clear`      | Clear console screen and redisplay header                    |
| `help`       | Display all available commands                               |

### 🆕 Web Server Commands (Available on Both Boards!)

| Command            | Description                                     |
| ------------------ | ----------------------------------------------- |
| `webserver start`  | Launch web server for browser-based interface   |
| `webserver stop`   | Stop the web server                             |
| `webserver status` | Check server status and get access URL          |
| `webserver`        | Display web server help and feature information |

💡 **Web Interface Access**: Once started, access the web interface at the displayed IP address
(e.g., `http://192.168.4.1` in AP mode or your device IP in Station mode). The web interface
provides real-time monitoring, interactive WiFi scanning with clickable network details, and comprehensive
channel analysis through a beautiful mobile-responsive dashboard. **Now available on both ESP32dev and
Feather ESP32-S3 TFT boards!** (v3.0.0+)

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
│  Est. Speed:   50-150 Mbps (802.11n)               │
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

## 📁 Professional Project Structure

```text
esp32-wifi-utility/
├── platformio.ini                          # Multi-environment build configuration
├── README.md                               # Comprehensive project documentation
├── 📚 docs/                                # Professional documentation suite
│   ├── README.md                          # 🆕 Documentation index and navigation
│   ├── user-guides/                       # 🆕 End-user documentation
│   │   ├── CHANNEL_GUIDE.md              # 🆕 Channel analysis guide (340+ lines)
│   │   ├── LATENCY_GUIDE.md              # 🆕 Latency & jitter analysis guide
│   │   ├── ENHANCED_SCANNING.md          # Enhanced WiFi scanning features
│   │   └── IPERF_GUIDE.md                # iPerf network testing documentation
│   └── technical/                         # 🆕 Developer & technical documentation
│       ├── CHANNEL_IMPLEMENTATION.md     # 🆕 Technical implementation details
│       ├── TEST_INFRASTRUCTURE.md        # 🆕 Comprehensive testing framework
│       ├── AUTOMATED_RELEASES.md         # 🆕 Automated GitHub release system
│       ├── IMPLEMENTATION_SUMMARY.md     # 🆕 System enhancement overview
│       └── PROJECT_COMPLETION_SUMMARY.md # 🆕 Complete project status
├── scripts/                               # 🆕 Development tools and utilities
│   └── version-manager.sh                # 🆕 Version bumping and release management
├── 🧪 Test Infrastructure/                 # Professional testing suite
│   ├── run_tests.sh                       # 🆕 Comprehensive test runner
│   ├── run_tests_build_only.sh           # 🆕 Build verification script
│   └── test/                              # Unity test framework
│       ├── test_config.h                  # 🆕 Enhanced test configuration
│       ├── test_simple_validation.cpp     # 🆕 Working test suite
│       └── *.cpp.bak                      # Backup comprehensive test files
├── src/                                    # Source implementation files
│   ├── main.cpp                          # Main application with channel integration
│   ├── channel_analyzer.cpp              # 🆕 Professional spectrum analysis (570+ lines)
│   ├── latency_analyzer.cpp              # 🆕 Network performance analysis
│   ├── wifi_manager.cpp                  # WiFi state management and scanning
│   ├── ap_manager.cpp                    # Access Point client management
│   ├── iperf_manager.cpp                 # Network performance testing
│   ├── led_controller.cpp                # LED/NeoPixel status indication
│   └── command_interface.cpp             # Enhanced command processing
├── include/                               # Header files and API definitions
│   ├── config.h                         # Hardware and dual-board configuration
│   ├── channel_analyzer.h               # 🆕 Spectrum analysis API (190+ declarations)
│   ├── latency_analyzer.h               # 🆕 Network performance API
│   ├── wifi_manager.h                   # WiFi management declarations
│   ├── ap_manager.h                     # AP management declarations
│   ├── iperf_manager.h                  # iPerf testing declarations
│   ├── led_controller.h                 # LED control declarations
│   └── command_interface.h              # Enhanced command interface
├── lib/                                  # Custom libraries (automatically managed)
└── LICENSE                               # MIT License
```

## 🔧 Technical Details & Performance Metrics

### Memory Usage (Optimized for Dual-Board Support)

#### ESP32 Development Board:

- **Flash**: ~1018KB total (77.6% of ESP32 capacity with all features)
  - Core WiFi functionality: ~200KB
  - **NEW**: Channel analyzer engine: ~45KB
  - **NEW**: Latency analyzer: ~15KB
  - Enhanced scanning features: ~8KB
  - iPerf implementation: ~25KB
  - QR Code generation: ~12KB
  - Command interface and utilities: ~25KB
- **RAM**: ~52KB baseline usage (15.8% of available RAM)
- **Free Heap**: Typically 275KB+ available during operation

#### Adafruit Feather ESP32-S3 TFT:

- **Flash**: ~991KB total (68.7% of ESP32-S3 capacity with enhanced features)
  - All standard features plus NeoPixel integration: ~15KB
  - Enhanced visual feedback system: ~8KB
- **RAM**: ~62KB baseline usage (19.1% of available RAM)
- **Free Heap**: Typically 265KB+ available during operation
- **PSRAM**: 8MB additional high-speed memory available for future enhancements

### WiFi Specifications

- **Protocols**: 802.11 b/g/n
- **Frequency**: 2.4 GHz
- **Security**: Open, WEP, WPA/WPA2/WPA3 PSK, WPA2 Enterprise
- **Range**: Typical ESP32 WiFi range (varies by antenna and environment)

### Performance (Enhanced Capabilities)

#### Core Performance:

- **Scan Speed**: ~3-5 seconds per enhanced scan with full analysis
- **AP Capacity**: Up to 4 simultaneous clients (ESP32 limitation)
- **Response Time**: <100ms for serial commands
- **Connection Time**: Typical WiFi connection in 5-10 seconds

#### 🆕 Professional Channel Analysis Performance:

- **Quick Scan**: <5 seconds for basic channel congestion analysis
- **Detailed Scan**: <30 seconds for comprehensive spectrum analysis
- **Background Monitoring**: Configurable 1-60 second intervals
- **AI Recommendations**: <2 seconds for optimal channel calculation
- **Memory Efficiency**: <8KB RAM usage for full channel analysis functionality
- **Memory Optimization**: Flash usage reduced by ~27KB through compiler optimizations (v4.0.0)
- **Web Server Caching**: 50 networks cached for 5 minutes with minimal RAM impact
- **Accuracy**: ±1 dBm RSSI measurement precision
- **Reliability**: 99.5%+ successful scan completion rate

#### 🆕 Network Performance & Latency Analysis:

- **Latency Testing**: Comprehensive ping-style analysis with jitter calculations
- **Statistical Processing**: Real-time mean, median, standard deviation calculation
- **Packet Loss Detection**: Accurate network quality assessment
- **Response Time**: <50ms for latency command processing

#### Traditional Performance:

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

### Code Organization (Professional Architecture)

- **Total Lines**: ~4500+ lines organized across enterprise-grade modular architecture
- **Main Loop**: Clean entry point in `main.cpp` with minimal logic
- **Modular Design**: 8 specialized modules with clear separation of concerns
- **Enhanced Features**:
  - Professional spectrum analysis engine (570+ lines)
  - Advanced latency analysis with statistical processing
  - Comprehensive WiFi scanning with AI recommendations
  - Interactive web interface with clickable network details (336+ lines added in v4.0.0)
  - iPerf performance testing suite
  - Dual-board hardware abstraction layer
- **Memory Efficient**:
  - Optimized global state management with proper extern declarations
  - PROGMEM storage for HTML/CSS (4KB+ moved to flash)
  - F() macro for 200+ string literals (~3KB RAM saved)
  - String pre-allocation to reduce heap fragmentation
  - Compiler optimizations: -Os, -ffunction-sections, -fdata-sections, -Wl,--gc-sections
- **Quality Assurance**:
  - Unity test framework integration
  - 19 comprehensive test cases (100% pass rate)
  - Test coverage for cache management, network details, WiFi fundamentals
  - Automated build verification system
  - Professional CI/CD pipeline ready
- **Compilation**: Zero warnings with optimized builds for both platforms
- **Documentation**:
  - 6 comprehensive documentation files (1000+ pages total)
  - Professional API documentation
  - Complete user guides and technical implementation details

## 🚧 Future Enhancements

### 🔧 Core Features

- [ ] **Power Management**: Sleep mode for battery operation
- [x] **Web Interface**: ✅ Implemented! Browser-based control available on both boards (v3.0.0)
- [x] **Clickable Network Details**: ✅ Implemented! Interactive scan results with detailed analysis (v4.0.0)
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

This project is open source under the MIT License. Feel free to modify and distribute according to your needs.

## 👤 Maintainer

**Maintained by:** [Arunkumar Mourougappane](https://github.com/arunkumar-mourougappane)

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
