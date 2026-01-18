# ESP32 WiFi Utility - Documentation Index

Welcome to the ESP32 WiFi Utility documentation portal. Find guides, references, and technical documentation organized by topic.

## 📖 Getting Started

New to the ESP32 WiFi Utility? Start here:

1. **[Main README](../README.md)** - Project overview and quick start
2. **[Version History](VERSION_HISTORY.md)** - Release notes and what's new
3. **[Features Guide](FEATURES.md)** - Complete feature descriptions
4. **[Command Reference](COMMAND_REFERENCE.md)** - All available commands
5. **[Usage Examples](USAGE_EXAMPLES.md)** - Practical command examples

## 📚 Core Documentation

### Essential Guides

- **[Features Guide](FEATURES.md)** - Comprehensive feature descriptions
- **[Command Reference](COMMAND_REFERENCE.md)** - Complete command documentation
- **[Usage Examples](USAGE_EXAMPLES.md)** - Practical usage scenarios
- **[Version History](VERSION_HISTORY.md)** - Release notes and changelog
- **[Architecture Guide](ARCHITECTURE.md)** - Technical implementation details

### Configuration & Setup

- **[Security Configuration](SECURITY_CONFIGURATION.md)** - WiFi security setup guide
- **[Security Testing](SECURITY_TESTING.md)** - Testing procedures and QA
- **[Configuration System](CONFIGURATION_SYSTEM.md)** - Persistent settings
- **[AP Config Quick Reference](AP_CONFIG_QUICK_REF.md)** - Access Point commands
- **[Station Config Quick Reference](STATION_CONFIG_QUICK_REF.md)** - Station commands

### Web & Display Interfaces

- **[Web Configuration](WEB_CONFIGURATION.md)** - Browser-based configuration
- **[Responsive Menu](RESPONSIVE_MENU.md)** - Mobile-optimized interface
- **[TFT UI Redesign](TFT_UI_REDESIGN.md)** - v5.2.0 display enhancements

## 🎯 Feature-Specific Guides

### Network Analysis & Testing

- **[Channel Analysis Guide](user-guides/CHANNEL_GUIDE.md)** - Professional spectrum analysis
- **[Latency Testing Guide](user-guides/LATENCY_GUIDE.md)** - Network performance testing
- **[Enhanced WiFi Scanning](user-guides/ENHANCED_SCANNING.md)** - Advanced network discovery
- **[iPerf Network Testing](user-guides/IPERF_GUIDE.md)** - Bandwidth measurement
- **[Port Scanner](PORT_SCANNER.md)** - Network security auditing
- **[Signal Strength Monitor](SIGNAL_STRENGTH_MONITOR.md)** - RSSI monitoring
- **[Channel Graph](CHANNEL_GRAPH.md)** - Visual spectrum visualization

### Hardware & Display

- **[NeoPixel Guide](user-guides/NEOPIXEL_GUIDE.md)** - RGB LED status indicators
- **[Web Interface Guide](user-guides/WEB_INTERFACE.md)** - Complete dashboard documentation

## 🔧 Technical Documentation

### Architecture & Implementation

- **[Architecture Guide](ARCHITECTURE.md)** - System design and structure
- **[Library Structure](LIBRARY_STRUCTURE.md)** - Modular architecture overview
- **[Implementation Summary](technical/IMPLEMENTATION_SUMMARY.md)** - System overview
- **[Project Completion Summary](technical/PROJECT_COMPLETION_SUMMARY.md)** - Final status

### Advanced Topics

- **[Channel Implementation](technical/CHANNEL_IMPLEMENTATION.md)** - Spectrum analysis algorithms
- **[Test Infrastructure](technical/TEST_INFRASTRUCTURE.md)** - Testing framework
- **[Automated Releases](technical/AUTOMATED_RELEASES.md)** - CI/CD and release automation
- **[Workflow Summary](technical/WORKFLOW_SUMMARY.md)** - Complete pipeline
- **[WiFi Task Architecture](technical/WIFI_TASK_ARCHITECTURE.md)** - Asynchronous WiFi operations
- **[TFT Display Task](technical/TFT_TASK_ARCHITECTURE.md)** - Display management

### Configuration Deep Dives

- **[AP Configuration Persistence](AP_CONFIG_PERSISTENCE.md)** - Access Point storage
- **[Station Configuration Persistence](STATION_CONFIG_PERSISTENCE.md)** - Station storage

## 📊 Release Information

### Latest Releases

- **[v5.2.1 Release Notes](RELEASE_NOTES_V5.2.1.md)** - Documentation fixes
- **[v5.2.0 Release Notes](RELEASE_NOTES_V5.2.0.md)** - TFT UI redesign
- **[Version History](VERSION_HISTORY.md)** - All releases
- **[CHANGELOG](../CHANGELOG.md)** - Detailed change log
- **[Improvements Summary](IMPROVEMENTS_SUMMARY.md)** - System enhancements

### Key Features by Version

#### v5.2.1 (Current)
- Documentation patch release
- Corrected TFT UI screenshot paths
- Enhanced visual documentation

#### v5.2.0
- Complete TFT display UI redesign
- Color-coded status screens
- Branded welcome experience
- Modular architecture improvements

#### v5.1.0
- Real-time latency testing UI
- Live progress bars and statistics
- Status page redesign
- External testing tools

#### v5.0.0
- QR code support for Station mode
- Non-blocking WiFi operations
- Structured logging system
- Enhanced status command

See [Version History](VERSION_HISTORY.md) for complete release notes.
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
├── RELEASE_NOTES_V5.2.1.md            # 🆕 Latest release notes (v5.2.1)
├── RELEASE_NOTES_V5.2.0.md            # v5.2.0 release notes
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