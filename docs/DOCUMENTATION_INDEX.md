# ESP32 WiFi Utility - Complete Documentation Index

**Current Version:** 5.0.0 (Unreleased)  
**Last Updated:** November 2, 2025

---

## 📚 Table of Contents

1. [Getting Started](#getting-started)
2. [Latest Updates](#latest-updates)
3. [User Documentation](#user-documentation)
4. [Technical Documentation](#technical-documentation)
5. [Quick Reference Guides](#quick-reference-guides)
6. [Migration Guides](#migration-guides)
7. [Archived Documentation](#archived-documentation)

---

##  Getting Started

**Essential reading for new users:**

| Document                                                | Description                           | Audience  |
| ------------------------------------------------------- | ------------------------------------- | --------- |
| **[Main README](../README.md)**                         | Project overview, features, and setup | Everyone  |
| **[Quick Start](../README.md#quick-start)**             | Build and deploy in 5 minutes         | Beginners |
| **[Command Reference](../README.md#command-reference)** | Complete command list                 | All users |
| **[Web Configuration Guide](configuration/WEB_CONFIGURATION.md)**     | Browser-based configuration           | All users |

**Recommended reading order:**

1. Main README → Overview and capabilities
2. Quick Start → Get device running
3. Command Reference → Learn available commands
4. Web Configuration → Configure via browser

---

## 🆕 Latest Updates

**Major enhancements focusing on TFT display UI redesign, modular architecture, and professional branding:**

### Release Documentation

| Document                                            | Description                         |
| --------------------------------------------------- | ----------------------------------- |
| **[Release Notes v5.0.0](RELEASE_NOTES_V5.0.0.md)** | Comprehensive release documentation |
| **[Changelog](../CHANGELOG.md)**                    | Complete version history            |

### New Features (v5.2.0)

| Feature                     | Documentation                                                                                |
| --------------------------- | -------------------------------------------------------------------------------------------- |
| **TFT UI Redesign** 🎨       | **[TFT UI Redesign Guide](features/TFT_UI_REDESIGN.md)** - Complete UI overhaul                      |
| **Welcome Screen**          | [TFT UI Redesign - Welcome Screen](TFT_UI_REDESIGN.md#welcome-screen)                       |
| **Color-Coded Status**      | [TFT UI Redesign - Status System](TFT_UI_REDESIGN.md#color-coded-status-system)             |
| **Modular Architecture**    | [TFT UI Redesign - Architecture](TFT_UI_REDESIGN.md#modular-architecture)                   |
| **UI Design Assets**        | [Lopaka Editor Project](https://lopaka.app/editor/23371/50884)                              |

### Features (v5.0.0)

| Feature                     | Documentation                                                                                |
| --------------------------- | -------------------------------------------------------------------------------------------- |
| **QR Code Support**         | [Release Notes - QR Code Section](RELEASE_NOTES_V5.0.0.md#qr-code-support-for-station-mode)  |
| **Non-Blocking WiFi**       | [Release Notes - Non-Blocking Section](RELEASE_NOTES_V5.0.0.md#non-blocking-wifi-connection) |
| **Structured Logging**      | [Release Notes - Logging Section](RELEASE_NOTES_V5.0.0.md#structured-logging-system)         |
| **Enhanced Status Command** | [Release Notes - Status Section](RELEASE_NOTES_V5.0.0.md#enhanced-status-command)            |
| **WiFi Command Task**       | [WiFi Task Architecture](technical/WIFI_TASK_ARCHITECTURE.md)                                |
| **TFT Display Task**        | [TFT Task Architecture](technical/TFT_TASK_ARCHITECTURE.md)                                  |

### Previous Release (v4.3.1)

| Feature                     | Documentation                                      |
| --------------------------- | -------------------------------------------------- |
| **Port Scanner**            | [Port Scanner Guide](features/PORT_SCANNER.md)              |
| **Signal Strength Monitor** | [Signal Monitor Guide](features/SIGNAL_STRENGTH_MONITOR.md) |
| **Channel Graph**           | [Channel Graph Guide](features/CHANNEL_GRAPH.md)            |

---

## 📖 User Documentation

### Web Interface & Configuration

| Document                                                | Description                          | Level        |
| ------------------------------------------------------- | ------------------------------------ | ------------ |
| **[Web Configuration Guide](configuration/WEB_CONFIGURATION.md)**     | Complete web interface documentation | Beginner     |
| **[Web Interface Guide](user-guides/WEB_INTERFACE.md)** | Dashboard and analysis features      | Beginner     |
| **[Configuration System](configuration/CONFIGURATION_SYSTEM.md)**     | Overview of persistent configuration | Intermediate |
| **[Responsive Menu Guide](features/RESPONSIVE_MENU.md)**         | Mobile interface usage               | Beginner     |

### Network Analysis & Testing

| Document                                                   | Description                           | Level        |
| ---------------------------------------------------------- | ------------------------------------- | ------------ |
| **[Port Scanner](features/PORT_SCANNER.md)**                        | Port scanning and service detection   | Intermediate |
| **[Signal Strength Monitor](features/SIGNAL_STRENGTH_MONITOR.md)**  | RSSI monitoring and analysis          | Beginner     |
| **[Channel Graph](features/CHANNEL_GRAPH.md)**                      | Visual spectrum analysis              | Intermediate |
| **[Channel Analysis Guide](user-guides/CHANNEL_GUIDE.md)** | Spectrum analysis and recommendations | Intermediate |
| **[Latency Testing Guide](user-guides/LATENCY_GUIDE.md)**  | Network performance testing           | Intermediate |
| **[iPerf Guide](user-guides/IPERF_GUIDE.md)**              | Throughput measurement                | Advanced     |
| **[Enhanced Scanning](user-guides/ENHANCED_SCANNING.md)**  | WiFi network discovery                | Beginner     |

### Security Configuration

| Document                                                      | Description                             | Level        |
| ------------------------------------------------------------- | --------------------------------------- | ------------ |
| **[Security Configuration Guide](security/SECURITY_CONFIGURATION.md)** | Complete WiFi security configuration    | Beginner     |
| **[Security Best Practices](security/SECURITY_BEST_PRACTICES.md)**     | Security recommendations and compliance | Intermediate |
| **[Security Testing Suite](security/SECURITY_TESTING.md)**             | Comprehensive testing procedures        | Advanced     |
| **[Security Test Results](security/SECURITY_TEST_RESULTS.md)**         | Test execution results and validation   | Advanced     |

### Hardware-Specific

| Document                                            | Description                              | Level    |
| --------------------------------------------------- | ---------------------------------------- | -------- |
| **[NeoPixel Guide](user-guides/NEOPIXEL_GUIDE.md)** | RGB LED status indicators (Feather only) | Beginner |

---

## 🔧 Technical Documentation

### FreeRTOS Task Architecture (NEW)

| Document                                                     | Description                      | Audience   |
| ------------------------------------------------------------ | -------------------------------- | ---------- |
| **[WiFi Command Task](technical/WIFI_TASK_ARCHITECTURE.md)** | Asynchronous WiFi mode switching | Developers |
| **[TFT Display Task](technical/TFT_TASK_ARCHITECTURE.md)**   | Event-driven display updates     | Developers |
| **[RTOS Architecture](technical/RTOS_ARCHITECTURE.md)**      | Overall task management          | Developers |
| **[RTOS API Reference](technical/RTOS_API_REFERENCE.md)**    | Task and queue APIs              | Developers |

### FreeRTOS Tutorials

| Document                                                      | Description                | Audience   |
| ------------------------------------------------------------- | -------------------------- | ---------- |
| **[Creating New Tasks](technical/RTOS_TUTORIAL_NEW_TASK.md)** | Step-by-step task creation | Developers |
| **[Queue Communication](technical/RTOS_TUTORIAL_QUEUES.md)**  | Inter-task messaging       | Developers |
| **[Debugging RTOS](technical/RTOS_TUTORIAL_DEBUGGING.md)**    | Task debugging techniques  | Developers |

### Configuration System

| Document                                                        | Description                     | Audience   |
| --------------------------------------------------------------- | ------------------------------- | ---------- |
| **[AP Config Persistence](configuration/AP_CONFIG_PERSISTENCE.md)**           | AP configuration deep dive      | Developers |
| **[Station Config Persistence](configuration/STATION_CONFIG_PERSISTENCE.md)** | Station configuration deep dive | Developers |
| **[Configuration System](configuration/CONFIGURATION_SYSTEM.md)**             | NVS-based persistence           | Developers |

### Implementation & Architecture

| Document                                                                  | Description                   | Audience   |
| ------------------------------------------------------------------------- | ----------------------------- | ---------- |
| **[Library Structure](architecture/LIBRARY_STRUCTURE.md)**                             | Modular architecture overview | Developers |
| **[Implementation Summary](technical/IMPLEMENTATION_SUMMARY.md)**         | System architecture overview  | Developers |
| **[Channel Implementation](technical/CHANNEL_IMPLEMENTATION.md)**         | Spectrum analysis algorithms  | Developers |
| **[Project Completion Summary](technical/PROJECT_COMPLETION_SUMMARY.md)** | Project achievements          | All        |

### Build & Testing

| Document                                                    | Description                       | Audience     |
| ----------------------------------------------------------- | --------------------------------- | ------------ |
| **[Security Testing Suite](security/SECURITY_TESTING.md)**           | WiFi security testing procedures  | QA/Testers   |
| **[Security Test Results](security/SECURITY_TEST_RESULTS.md)**       | Test results and validation       | QA/Testers   |
| **[Test Infrastructure](technical/TEST_INFRASTRUCTURE.md)** | Testing framework                 | Developers   |
| **[Automated Releases](technical/AUTOMATED_RELEASES.md)**   | CI/CD pipeline                    | Contributors |
| **[Workflow Summary](technical/WORKFLOW_SUMMARY.md)**       | Complete automation overview      | Contributors |

---

## 📋 Quick Reference Guides

**Command cheat sheets for rapid configuration:**

| Guide                                                       | Commands                   | Use Case                    |
| ----------------------------------------------------------- | -------------------------- | --------------------------- |
| **[AP Config Quick Ref](reference/AP_CONFIG_QUICK_REF.md)**           | Access Point configuration | Setting up WiFi hotspot     |
| **[Station Config Quick Ref](reference/STATION_CONFIG_QUICK_REF.md)** | Station configuration      | Connecting to WiFi networks |

**Web Interface Quick Access:**

- Configuration: `http://<device-ip>/config`
- Dashboard: `http://<device-ip>/`
- Network Scan: `http://<device-ip>/scan`
- Analysis: `http://<device-ip>/analysis`

---

## 🔄 Migration Guides

**Upgrading between versions:**

| From Version  | To Version    | Guide                                                           | Breaking Changes       |
| ------------- | ------------- | --------------------------------------------------------------- | ---------------------- |
| v4.1.0 (RTOS) | v4.2.0        | **[Migration Guide v4.2.0](MIGRATION_GUIDE_V4.2.0.md)**         | ⚠️ RTOS removed        |
| v3.x          | v4.1.0 (RTOS) | **[RTOS Migration Guide](user-guides/RTOS_MIGRATION_GUIDE.md)** | ⚠️ Architecture change |

**Migration Checklist:**

- [ ] Read release notes for target version
- [ ] Review breaking changes section
- [ ] Update configuration files if needed
- [ ] Test core functionality after upgrade
- [ ] Re-save configurations if migrating to v4.2.0

---

## 📦 Archived Documentation

**Historical documentation for removed features:**

| Directory                                      | Contents                    | Reason                            |
| ---------------------------------------------- | --------------------------- | --------------------------------- |
| **[technical/archived/](technical/archived/)** | RTOS documentation (v4.1.0) | RTOS removed in v4.2.0            |
| **[archive/](archive/)**                       | Old feature documentation   | Superseded by new implementations |

**Archived Topics:**

- RTOS Architecture (v4.1.0)
- RTOS API Reference (v4.1.0)
- RTOS Tutorials (v4.1.0)
- RTOS FAQ (v4.1.0)
- Old web server features

**Why archived?**

- Features removed for simplification
- Superseded by better implementations
- Historical reference only

---

## 🎯 Documentation by Use Case

### Use Case: First-Time Setup

1. [Main README](../README.md) - Understand capabilities
2. [Quick Start](../README.md#quick-start) - Build and deploy
3. [Command Reference](../README.md#command-reference) - Learn commands
4. [Web Configuration](configuration/WEB_CONFIGURATION.md) - Configure via browser

### Use Case: Configure WiFi Hotspot

1. [AP Config Quick Ref](reference/AP_CONFIG_QUICK_REF.md) - Command syntax
2. [AP Config Persistence](configuration/AP_CONFIG_PERSISTENCE.md) - Detailed guide
3. [Web Configuration](configuration/WEB_CONFIGURATION.md) - Web interface method

### Use Case: Connect to WiFi Network

1. [Station Config Quick Ref](reference/STATION_CONFIG_QUICK_REF.md) - Command syntax
2. [Station Config Persistence](configuration/STATION_CONFIG_PERSISTENCE.md) - Detailed guide
3. [Web Configuration](configuration/WEB_CONFIGURATION.md) - Web interface method

### Use Case: Network Analysis

1. [Enhanced Scanning](user-guides/ENHANCED_SCANNING.md) - WiFi scanning
2. [Channel Analysis Guide](user-guides/CHANNEL_GUIDE.md) - Spectrum analysis
3. [Latency Testing Guide](user-guides/LATENCY_GUIDE.md) - Performance testing
4. [iPerf Guide](user-guides/IPERF_GUIDE.md) - Throughput measurement

### Use Case: Security Configuration

1. [Security Configuration Guide](security/SECURITY_CONFIGURATION.md) - Complete security setup
2. [Security Best Practices](security/SECURITY_BEST_PRACTICES.md) - Security recommendations
3. [AP Config Quick Ref](reference/AP_CONFIG_QUICK_REF.md) - AP security commands
4. [Station Config Quick Ref](reference/STATION_CONFIG_QUICK_REF.md) - Station security preferences
5. [Web Configuration](configuration/WEB_CONFIGURATION.md) - Security via web interface

### Use Case: Testing WiFi Security

1. [Security Testing Suite](security/SECURITY_TESTING.md) - Test procedures
2. [Security Test Results](security/SECURITY_TEST_RESULTS.md) - Results documentation
3. Test all security types: Open, WPA2, WPA3, Mixed
4. Validate QR code generation and scanning

### Use Case: Mobile Configuration

1. [Web Configuration](configuration/WEB_CONFIGURATION.md) - Mobile interface
2. [Responsive Menu Guide](features/RESPONSIVE_MENU.md) - Mobile navigation
3. [Web Interface Guide](user-guides/WEB_INTERFACE.md) - Touch-friendly features

### Use Case: Contributing

1. [Technical Documentation](#technical-documentation) - Architecture
2. [Test Infrastructure](technical/TEST_INFRASTRUCTURE.md) - Testing
3. [Workflow Summary](technical/WORKFLOW_SUMMARY.md) - CI/CD

---

## 📊 Documentation Statistics

| Category         | Count  | Total Pages (est.) |
| ---------------- | ------ | ------------------ |
| User Guides      | 13     | ~250               |
| Technical Docs   | 10     | ~150               |
| Quick References | 2      | ~20                |
| Migration Guides | 2      | ~40                |
| Release Notes    | 1      | ~20                |
| **Total**        | **28** | **~480**           |

---

## 🔗 External Resources

**Related Links:**

- [GitHub Repository](https://github.com/arunkumar-mourougappane/esp32-wifi-utility)
- [Issue Tracker](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/issues)
- [Changelog](../CHANGELOG.md)
- [License](../LICENSE)

**ESP32 Resources:**

- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Arduino Core for ESP32](https://github.com/espressif/arduino-esp32)
- [PlatformIO Documentation](https://docs.platformio.org/)

---

## 📝 Documentation Contributions

**Improving Documentation:**

- Report errors via [Issues](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/issues)
- Suggest improvements in pull requests
- Add examples and use cases
- Clarify confusing sections

**Documentation Standards:**

- Clear, concise language
- Step-by-step instructions
- Code examples with explanations
- Screenshots where helpful
- Proper markdown formatting

---

## ✅ Document Status Legend

| Symbol | Meaning                                |
| ------ | -------------------------------------- |
| 🆕     | New in current version                 |
| ⚠️     | Breaking changes or important warnings |
| 🔄     | Updated recently                       |
| 📦     | Archived/historical                    |

---

**Last Updated:** October 25, 2025  
**Version:** 4.2.0  
**Maintained By:** [Arunkumar Mourougappane](https://github.com/arunkumar-mourougappane)
