# ESP32 WiFi Utility - Documentation Index

## 📖 User Guides

Comprehensive guides for using all features of the ESP32 WiFi Utility:

### Core Features
- **[Web Configuration Guide](WEB_CONFIGURATION.md)** - Complete web-based configuration system
- **[Configuration System](CONFIGURATION_SYSTEM.md)** - Persistent AP and Station configuration
- **[Port Scanner](PORT_SCANNER.md)** - 🆕 v4.3.0: Port scanning and service detection
- **[Signal Strength Monitor](SIGNAL_STRENGTH_MONITOR.md)** - 🆕 v4.3.0: RSSI monitoring and analysis
- **[Channel Graph](CHANNEL_GRAPH.md)** - 🆕 v4.3.0: Visual spectrum analysis
- **[Channel Analysis Guide](user-guides/CHANNEL_GUIDE.md)** - Professional spectrum analysis and AI recommendations
- **[Latency Testing Guide](user-guides/LATENCY_GUIDE.md)** - Network performance and jitter analysis
- **[Enhanced WiFi Scanning](user-guides/ENHANCED_SCANNING.md)** - Advanced network discovery and analysis
- **[iPerf Network Testing](user-guides/IPERF_GUIDE.md)** - Comprehensive throughput and performance testing
- **[Web Interface Guide](user-guides/WEB_INTERFACE.md)** - Complete web-based dashboard documentation (Available on both boards!)
- **[NeoPixel Status Display](user-guides/NEOPIXEL_GUIDE.md)** - RGB LED status indicators (Feather ESP32-S3 TFT only)

### Quick References
- **[AP Config Quick Reference](AP_CONFIG_QUICK_REF.md)** - Access Point configuration commands
- **[Station Config Quick Reference](STATION_CONFIG_QUICK_REF.md)** - Station configuration commands
- **[Responsive Menu Guide](RESPONSIVE_MENU.md)** - Mobile-optimized web interface

## 🔧 Technical Documentation

In-depth technical information for developers and advanced users:

### FreeRTOS Task Architecture
- **[WiFi Command Task](technical/WIFI_TASK_ARCHITECTURE.md)** - 🆕 Asynchronous WiFi mode switching
- **[TFT Display Task](technical/TFT_TASK_ARCHITECTURE.md)** - 🆕 Event-driven display updates
- **[RTOS Architecture](technical/RTOS_ARCHITECTURE.md)** - Overall task management system
- **[RTOS API Reference](technical/RTOS_API_REFERENCE.md)** - Task and queue APIs
- **[Creating New Tasks](technical/RTOS_TUTORIAL_NEW_TASK.md)** - Tutorial: Step-by-step task creation
- **[Queue Communication](technical/RTOS_TUTORIAL_QUEUES.md)** - Tutorial: Inter-task messaging
- **[Debugging RTOS](technical/RTOS_TUTORIAL_DEBUGGING.md)** - Tutorial: Task debugging techniques

### Implementation Details
- **[Library Structure](LIBRARY_STRUCTURE.md)** - Modular architecture overview
- **[Channel Implementation](technical/CHANNEL_IMPLEMENTATION.md)** - Technical deep-dive into spectrum analysis algorithms
- **[Test Infrastructure](technical/TEST_INFRASTRUCTURE.md)** - Comprehensive testing framework for dual-board support
- **[Automated Releases](technical/AUTOMATED_RELEASES.md)** - GitHub Actions automated release system
- **[Workflow Summary](technical/WORKFLOW_SUMMARY.md)** - Complete CI/CD pipeline and release automation overview
- **[Implementation Summary](technical/IMPLEMENTATION_SUMMARY.md)** - Complete system architecture overview
- **[Project Completion Summary](technical/PROJECT_COMPLETION_SUMMARY.md)** - Final project status and achievements

### Configuration Deep Dives
- **[AP Configuration Persistence](AP_CONFIG_PERSISTENCE.md)** - Access Point configuration system details
- **[Station Configuration Persistence](STATION_CONFIG_PERSISTENCE.md)** - Station configuration system details

##  Quick Start

New to the ESP32 WiFi Utility? Start here:

1. **[Main README](../README.md)** - Project overview and setup instructions
2. **[Hardware Setup](../README.md#hardware-requirements--dual-board-support)** - Choose and configure your ESP32 board
3. **[Quick Start Guide](../README.md#quick-start)** - Build and deploy in minutes
4. **[Command Reference](../README.md#command-reference)** - Complete command documentation
5. **[Web Configuration](WEB_CONFIGURATION.md)** - 🆕 v4.2.0: Configure via browser

## 🎯 Feature Highlights

### 🆕 v5.0.0: TFT Display Enhancements & System Responsiveness
- **QR Code Support**: Automatic WiFi credential sharing via scannable QR codes (Station mode)
- **Non-Blocking WiFi**: Asynchronous connection operations - device stays responsive during connections
- **Structured Logging**: Professional LOG_* framework with severity levels and component tags
- **Enhanced Status Command**: Comprehensive network info (signal, uptime, channel, quality)
- **FreeRTOS Task Architecture**: Dedicated WiFi command task and TFT display task
- **10x Web Server Speed**: Main loop optimized from 100ms to 10ms for instant response
- **Critical Bug Fixes**: Connection timing, TFT updates, mode switching, web server compatibility

### 🆕 v4.3.1: Comprehensive Documentation
- **Port Scanner Documentation**: 1,400+ lines covering fundamentals, use cases, security
- **Signal Monitor Documentation**: 1,100+ lines with RSSI guide, API integration, best practices
- **Wiki Enhancements**: 2,500+ lines total, 15+ API examples, 10+ use cases

### 🆕 v4.3.0: Advanced Network Analysis & TFT Display
- **Port Scanner**: Network security auditing with service identification (25+ services)
- **Signal Strength Monitor**: Real-time RSSI monitoring with auto-scan capabilities
- **Channel Graph**: Visual spectrum analysis with interactive web interface
- **TFT Display Support**: QR codes, mode indicators, network information on both TFT variants

### v4.2.0: Configuration Persistence & Instant Mode Switching
- **Persistent Configuration**: Save AP and Station settings that survive reboots
- **Auto-Start/Connect**: Device boots into configured mode automatically
- **Web Configuration Interface**: Complete configuration via browser
- **Instant Mode Switching**: Toggle between AP and Station without rebooting
- **Responsive Design**: Mobile-optimized interface with hamburger menu
- **Password Security**: Saved passwords never exposed in web interface
- **Reboot Modal**: Countdown timer for controlled device restarts
- **Quick References**: Command cheat sheets for rapid configuration

### v4.0.0: Interactive Network Details & Multi-Platform Web Server
- **Clickable Network Details**: Click any scanned network to view comprehensive information
  - 8-level signal quality scale with visual indicators
  - Channel congestion analysis (Clear to Severe ratings)
  - Security assessment for all 9 WiFi encryption types
  - Connection recommendations based on signal and channel conditions
- **Web Server on Both Boards**: Now available on ESP32dev and Feather ESP32-S3 TFT
- **Memory Optimization**: 27KB flash savings through compiler optimizations
- **Comprehensive Testing**: 19 automated test cases with 100% pass rate

### 🌐 Web-Based Interface
- Browser-accessible dashboard for all features (Both boards supported!)
- Hierarchical dropdown navigation with professional UI/UX
- Real-time progress indicators with backdrop overlays
- Mobile-responsive design with touch-friendly controls
- Unified Analysis Dashboard for network testing tools
- Interactive scan results with clickable network details
- Configuration management with instant mode switching

### 🆕 Professional Channel Analysis
- Real-time 2.4GHz spectrum scanning with AI-powered recommendations
- Advanced congestion detection and interference analysis
- Professional reporting with JSON export capabilities

### 📊 Advanced Performance Testing  
- Comprehensive latency analysis with jitter calculations
- Network quality assessment and statistical reporting
- iPerf integration for throughput measurement

### 🔧 Tri-Board Support
- ESP32 Development Board with standard LED indication
- Adafruit Feather ESP32-S3 TFT with enhanced NeoPixel feedback and web server
- Adafruit Feather ESP32-S3 Reverse TFT (added v4.1.0)
- Hardware-specific optimizations and conditional compilation

### 🧪 Professional Testing Framework
- Unity test framework integration for all platforms
- Automated build verification and CI/CD ready
- Comprehensive test coverage with hardware validation

## 📚 Documentation Organization

```
docs/
├── README.md                           # This index file
├── DOCUMENTATION_INDEX.md              # Complete documentation catalog
├── RELEASE_NOTES_V5.0.0.md            # 🆕 Latest release notes
├── RELEASE_NOTES_V4.3.1.md            # v4.3.1 release notes
├── WEB_CONFIGURATION.md                # Web-based configuration guide
├── CONFIGURATION_SYSTEM.md             # Configuration overview
├── PORT_SCANNER.md                     # 🆕 v4.3.0 Port scanning guide
├── SIGNAL_STRENGTH_MONITOR.md         # 🆕 v4.3.0 Signal monitor guide
├── CHANNEL_GRAPH.md                    # 🆕 v4.3.0 Channel graph guide
├── AP_CONFIG_PERSISTENCE.md            # AP configuration details
├── STATION_CONFIG_PERSISTENCE.md       # Station configuration details
├── AP_CONFIG_QUICK_REF.md             # AP commands cheat sheet
├── STATION_CONFIG_QUICK_REF.md        # Station commands cheat sheet
├── RESPONSIVE_MENU.md                  # Mobile menu guide
├── LIBRARY_STRUCTURE.md                # Modular architecture
├── IMPROVEMENTS_SUMMARY.md             # Feature improvements
├── user-guides/                        # End-user documentation
│   ├── CHANNEL_GUIDE.md               # Channel analysis user guide
│   ├── LATENCY_GUIDE.md               # Latency testing guide
│   ├── ENHANCED_SCANNING.md           # WiFi scanning features
│   ├── IPERF_GUIDE.md                 # Performance testing guide
│   ├── WEB_INTERFACE.md               # Web dashboard documentation
│   └── NEOPIXEL_GUIDE.md              # NeoPixel LED guide (Feather only)
└── technical/                          # Developer documentation
    ├── WIFI_TASK_ARCHITECTURE.md      # 🆕 WiFi command task
    ├── TFT_TASK_ARCHITECTURE.md       # 🆕 TFT display task
    ├── RTOS_ARCHITECTURE.md            # RTOS system overview
    ├── RTOS_API_REFERENCE.md           # Task and queue APIs
    ├── RTOS_TUTORIAL_NEW_TASK.md      # Tutorial: Creating tasks
    ├── RTOS_TUTORIAL_QUEUES.md        # Tutorial: Queue messaging
    ├── RTOS_TUTORIAL_DEBUGGING.md     # Tutorial: Debugging tasks
    ├── CHANNEL_IMPLEMENTATION.md       # Algorithm implementation
    ├── TEST_INFRASTRUCTURE.md          # Testing framework
    ├── IMPLEMENTATION_SUMMARY.md       # Architecture overview
    ├── PROJECT_COMPLETION_SUMMARY.md   # Project achievements
    ├── AUTOMATED_RELEASES.md           # CI/CD pipeline
    └── WORKFLOW_SUMMARY.md             # Release automation
```

## 🌐 Quick Access - v4.2.0 Features

**Configuration via Serial:**
```bash
# Access Point
ap config "MyHotspot" "SecurePass123" 6 auto
ap start

# Station  
station config "HomeNetwork" "WiFiPass123" auto
station connect
```

**Configuration via Web:**
1. Connect to device
2. Navigate to `http://<device-ip>/config`
3. Fill in AP or Station configuration
4. Click Save
5. Use Quick Mode Switch to toggle modes instantly

**Benefits:**
- ✅ Persistent across reboots
- ✅ No retyping credentials
- ✅ Auto-start on boot
- ✅ Instant mode switching
- ✅ Mobile-friendly interface
- New in v4.0.0: Clickable network details with comprehensive information
- Guide: [Complete Web Interface Documentation](user-guides/WEB_INTERFACE.md)

## 🤝 Contributing

Want to contribute to the documentation? See the main [README](../README.md) for contribution guidelines.

## 📞 Support

For questions or issues:
1. Check the relevant user guide above
2. Review the [Command Reference](../README.md#command-reference)
3. Try the `help` command in the serial interface
4. For web interface: See [Web Interface Guide](user-guides/WEB_INTERFACE.md#troubleshooting)
5. Open an issue on the project repository

---

**Happy WiFi Analysis! 📡**