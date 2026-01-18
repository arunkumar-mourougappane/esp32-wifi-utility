# ESP32 WiFi Utility - Architecture

Technical documentation covering project structure, modular architecture, and implementation details.

## Table of Contents

- [Project Structure](#project-structure)
- [Modular Architecture](#modular-architecture)
- [Hardware Support](#hardware-support)
- [Memory Management](#memory-management)
- [Build System](#build-system)
- [Testing Framework](#testing-framework)

---

## Project Structure

```text
esp32-wifi-utility/
├── platformio.ini                          # Multi-environment build configuration
├── README.md                               # Project overview and quick start
├── CHANGELOG.md                            # Complete version history
├── LICENSE                                 # MIT License
├── 📚 docs/                                # Professional documentation suite
│   ├── README.md                          # Documentation index
│   ├── VERSION_HISTORY.md                 # Detailed release notes
│   ├── COMMAND_REFERENCE.md               # Complete command guide
│   ├── USAGE_EXAMPLES.md                  # Practical examples
│   ├── FEATURES.md                        # Feature descriptions
│   ├── ARCHITECTURE.md                    # This file
│   ├── SECURITY_CONFIGURATION.md          # Security setup
│   ├── SECURITY_TESTING.md                # Test procedures
│   ├── user-guides/                       # End-user documentation
│   │   ├── CHANNEL_GUIDE.md              # Channel analysis guide
│   │   ├── LATENCY_GUIDE.md              # Latency testing guide
│   │   ├── ENHANCED_SCANNING.md          # WiFi scanning features
│   │   ├── IPERF_GUIDE.md                # iPerf testing guide
│   │   └── NEOPIXEL_GUIDE.md             # NeoPixel LED guide
│   ├── technical/                         # Technical documentation
│   │   ├── CHANNEL_IMPLEMENTATION.md     # Spectrum analysis details
│   │   ├── TEST_INFRASTRUCTURE.md        # Testing framework
│   │   ├── AUTOMATED_RELEASES.md         # Release automation
│   │   ├── IMPLEMENTATION_SUMMARY.md     # System overview
│   │   ├── TFT_TASK_ARCHITECTURE.md      # TFT display architecture
│   │   └── WIFI_TASK_ARCHITECTURE.md     # WiFi task design
│   ├── images/                            # Screenshots and diagrams
│   └── archive/                           # Historical documentation
├── 🧪 Test Infrastructure/                 # Professional testing suite
│   ├── run_tests.sh                       # Comprehensive test runner
│   ├── run_tests_build_only.sh           # Build verification
│   └── test/                              # Unity test framework
│       ├── test_config.h                  # Test configuration
│       ├── test_simple_validation.cpp     # Working test suite
│       └── *.cpp.bak                      # Backup test files
├── src/                                    # Source implementation
│   ├── main.cpp                          # Main application
│   ├── channel_analyzer.cpp              # Spectrum analysis engine
│   ├── latency_analyzer.cpp              # Network performance
│   ├── wifi_manager.cpp                  # WiFi operations
│   ├── ap_manager.cpp                    # Access Point management
│   ├── iperf_manager.cpp                 # Performance testing
│   ├── led_controller.cpp                # LED/NeoPixel control
│   └── command_interface.cpp             # Command processing
├── include/                               # Header files
│   ├── config.h                         # Hardware configuration
│   ├── channel_analyzer.h               # Spectrum analysis API
│   ├── latency_analyzer.h               # Network performance API
│   ├── wifi_manager.h                   # WiFi declarations
│   ├── ap_manager.h                     # AP declarations
│   ├── iperf_manager.h                  # iPerf declarations
│   ├── led_controller.h                 # LED declarations
│   └── command_interface.h              # Command interface
├── lib/                                  # Custom libraries
│   ├── APConfig/                        # AP configuration persistence
│   ├── APManager/                       # AP client management
│   ├── BatteryMonitor/                  # Battery status
│   ├── CommandInterface/                # Command processing
│   ├── Config/                          # Configuration system
│   ├── LEDController/                   # LED control
│   ├── NetworkAnalyzer/                 # Network analysis
│   ├── NetworkTools/                    # Network utilities
│   ├── StationConfig/                   # Station configuration
│   ├── TFTDisplay/                      # TFT display support
│   ├── Utils/                           # Utility functions
│   ├── WebServer/                       # Web interface
│   └── WiFiManager/                     # WiFi management
├── scripts/                              # Development tools
│   ├── version-manager.sh               # Version management
│   └── udp_echo_server.py               # UDP testing server
└── pc_test_apps/                         # Test applications
    ├── Makefile                         # Build configuration
    └── udp_echo_server.cpp              # C++ UDP server
```

---

## Modular Architecture

### 🏗️ Professional Design Principles

This project features an **enterprise-grade, modular codebase** designed for:

- **Maintainability**: Each module handles specific functionality
- **Scalability**: Easy to add new features or modify existing ones
- **Code Reusability**: Modules can be used independently
- **Clean Dependencies**: Well-defined interfaces between components
- **Separation of Concerns**: Clear boundaries between modules

### 📦 Core Modules

#### Configuration Module (`config.h`)

- Centralized hardware configuration
- Board detection and pin assignments
- Timing and behavior constants
- Default WiFi credentials
- Conditional compilation flags

#### WiFi Manager (`wifi_manager`)

- WiFi state management
- Enhanced network scanning
- Connection handling
- Network information retrieval
- Signal strength analysis

#### Access Point Manager (`ap_manager`)

- Access Point creation and management
- Client monitoring and tracking
- Deauthentication functionality
- QR code generation
- Client list management

#### iPerf Manager (`iperf_manager`)

- Network performance testing
- TCP and UDP support
- Client and server modes
- Real-time statistics
- Bandwidth measurement

#### LED Controller (`led_controller`)

- Visual status indication
- Standard LED support (ESP32dev)
- NeoPixel RGB support (Feather)
- Multiple animation patterns
- Board-aware implementation

#### Command Interface (`command_interface`)

- Interactive serial command processing
- Smart command prompt
- Command parsing and execution
- Error handling and feedback
- Full integration with all modules

#### Main Application (`main.cpp`)

- Clean entry point
- Module initialization
- Main loop coordination
- Minimal application logic

### 🆕 Advanced Analysis Modules

#### Channel Analyzer (`channel_analyzer`)

Professional spectrum analysis engine (570+ lines):

- Real-time 2.4GHz band scanning
- AI-powered channel recommendations
- Congestion analysis (0-100% scoring)
- Interference pattern detection
- JSON export capabilities
- Comprehensive reporting

#### Latency Analyzer (`latency_analyzer`)

Network performance and jitter analysis:

- Advanced ping-style measurement
- Statistical analysis (mean, median, stddev)
- Packet loss detection
- Network quality assessment
- Historical tracking

### 🎯 Module Interactions

```text
main.cpp
    ├── config.h (hardware & timing constants)
    ├── wifi_manager (network operations & scanning)
    ├── ap_manager (client management)
    ├── iperf_manager (performance testing)
    ├── channel_analyzer (spectrum analysis)
    ├── latency_analyzer (network performance)
    ├── led_controller (status indication)
    └── command_interface (user interaction)
```

### Library Structure

The `lib/` directory contains modular libraries with clear responsibilities:

- **APConfig**: Persistent AP configuration storage
- **StationConfig**: Persistent station configuration
- **WebServer**: Complete web interface implementation
- **TFTDisplay**: Display management and rendering
- **NetworkAnalyzer**: Network analysis tools
- **NetworkTools**: Utility functions
- **BatteryMonitor**: Power management
- **Utils**: Common utilities

---

## Hardware Support

### 🔧 Supported Boards

#### ESP32 Development Board (Standard)

- **Processor**: ESP32 (dual-core Xtensa LX6)
- **Flash**: 4MB
- **RAM**: 320KB
- **LED**: Built-in LED on GPIO 2
- **Features**: Full functionality with standard LED

#### Adafruit Feather ESP32-S3 TFT

- **Processor**: ESP32-S3 (dual-core Xtensa LX7)
- **Flash**: 4MB
- **RAM**: 512KB
- **PSRAM**: 8MB
- **LED**: NeoPixel RGB on GPIO 48
- **Display**: 1.14" TFT (240x135 pixels)
- **Features**: Enhanced RGB feedback, TFT display support

#### Adafruit Feather ESP32-S3 Reverse TFT

- **Processor**: ESP32-S3 (dual-core Xtensa LX7)
- **Flash**: 4MB
- **RAM**: 512KB
- **PSRAM**: 8MB
- **LED**: NeoPixel RGB on GPIO 48
- **Display**: 1.14" TFT (240x135 pixels, reversed orientation)
- **Features**: Enhanced RGB feedback, reverse TFT layout

### Hardware Detection

The build system automatically detects the target board and applies appropriate configurations:

```cpp
#ifdef ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVERSETFT
    // Reverse TFT configuration
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT)
    // Standard TFT configuration
#elif defined(ARDUINO_FEATHER_ESP32)
    // Feather ESP32 configuration
#else
    // ESP32dev configuration
#endif
```

---

## Memory Management

### Memory Usage Optimization

Several techniques are used to optimize memory usage:

#### Flash Memory Optimization

- **PROGMEM Storage**: Large HTML/CSS strings stored in flash
- **F() Macro**: String literals moved to flash (~3KB RAM saved)
- **Compiler Optimizations**: `-Os`, `-ffunction-sections`, `-fdata-sections`
- **Link-Time Optimization**: `-Wl,--gc-sections` removes unused code
- **Result**: ~27KB flash saved compared to unoptimized builds

#### RAM Optimization

- **String Pre-allocation**: `.reserve()` calls to reduce fragmentation
- **Smart Caching**: 50 networks cached for 5 minutes with minimal impact
- **Efficient Data Structures**: Optimized structures for scan results
- **Stack Management**: Careful stack usage in recursive functions
- **Result**: Consistent free heap of 265-275KB during operation

### Memory Benchmarks

#### ESP32 Development Board

- **Flash**: 1,018KB / 1,310KB (77.6%)
- **RAM**: 52KB / 327KB (15.8%)
- **Free Heap**: ~275KB during operation
- **Stack**: Adequate for all operations

#### Feather ESP32-S3 TFT

- **Flash**: 991KB / 1,441KB (68.7%)
- **RAM**: 62KB / 327KB (19.1%)
- **Free Heap**: ~265KB during operation
- **PSRAM**: 8MB available for future enhancements

---

## Build System

### PlatformIO Configuration

Multi-environment build system supporting all boards:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

[env:adafruit_feather_esp32s3_tft]
platform = espressif32
board = adafruit_feather_esp32s3_tft
framework = arduino

[env:adafruit_feather_esp32s3_reversetft]
platform = espressif32
board = adafruit_feather_esp32s3_reversetft
framework = arduino
```

### Build Commands

```bash
# Build for specific board
pio run -e esp32dev
pio run -e adafruit_feather_esp32s3_tft
pio run -e adafruit_feather_esp32s3_reversetft

# Build all configurations
pio run

# Upload to device
pio run -e esp32dev -t upload

# Monitor serial output
pio device monitor
```

### Conditional Compilation

Board-specific features controlled via preprocessor directives:

```cpp
#ifdef USE_NEOPIXEL
    // NeoPixel-specific code
#endif

#ifdef USE_TFT
    // TFT display code
#endif

#ifdef USE_WEBSERVER
    // Web server code
#endif
```

---

## Testing Framework

### 🧪 Comprehensive Test Suite

Professional testing infrastructure with Unity framework:

- **19 Test Cases**: Comprehensive coverage
- **100% Pass Rate**: All tests passing on both boards
- **Test Categories**:
  - Cache management and validation
  - Network details and signal quality
  - WiFi fundamentals (RSSI, channels, encryption)
  - System integration and memory management

### Test Environments

```ini
[env:test]
platform = espressif32
board = esp32dev
test_framework = unity

[env:test_feather]
platform = espressif32
board = adafruit_feather_esp32s3_tft
test_framework = unity
```

### Running Tests

```bash
# Test on ESP32 Development Board
pio test --environment test

# Test on Feather ESP32-S3 TFT
pio test --environment test_feather

# Build tests without uploading
pio test --environment test --without-uploading

# Run test scripts
./run_tests.sh
./run_tests_build_only.sh
```

### Test Results

- **ESP32dev**: ~14 seconds, all tests pass
- **Feather ESP32-S3**: ~5.5 seconds, all tests pass
- **Coverage**: Cache, network details, WiFi fundamentals, system integration

See [Test Infrastructure Guide](technical/TEST_INFRASTRUCTURE.md) for details.

---

## Code Organization

### Total Lines of Code

- **~4,500+ lines** organized across enterprise-grade modular architecture
- **Main Application**: Clean entry point with minimal logic
- **Core Modules**: ~3,000 lines
- **Advanced Analysis**: ~800 lines (channel + latency)
- **Web Interface**: ~1,200 lines
- **Test Suite**: ~500 lines

### Code Quality

- **Zero Warnings**: Clean compilation on all platforms
- **Consistent Style**: Professional coding standards
- **Documentation**: Comprehensive inline documentation
- **Modular Design**: Clean interfaces between components
- **Error Handling**: Robust error handling throughout

### Compilation Statistics

- **Compilation Time**: ~14 seconds (ESP32dev), ~10 seconds (Feather)
- **Optimization Level**: `-Os` (optimize for size)
- **Standards**: C++11 with Arduino framework
- **Dependencies**: Minimal external dependencies

---

## Performance Characteristics

### Execution Performance

- **Command Response**: <100ms for most commands
- **WiFi Scan**: 3-5 seconds for comprehensive scan
- **Channel Analysis**: <5 seconds quick, <30 seconds detailed
- **Connection Time**: 5-10 seconds typical
- **Web Server Response**: <50ms for most requests

### Network Performance

- **WiFi Protocols**: 802.11 b/g/n
- **Frequency**: 2.4 GHz
- **Range**: Typical ESP32 WiFi range
- **Throughput**: 10-20 Mbps (TCP iPerf)
- **Latency**: 1-50ms typical (depends on network)
- **Accuracy**: ±1 dBm RSSI measurement

### Resource Utilization

- **CPU Usage**: ~40% dual-core during operation
- **Memory Fragmentation**: Minimal with pre-allocation
- **Power Consumption**: Typical for ESP32 WiFi operations
- **Thermal**: Normal ESP32 operating temperature

---

## Development Guidelines

### Adding New Features

1. Create module in `lib/` or add to `src/`
2. Add header to `include/`
3. Update `config.h` if needed
4. Add commands to `command_interface`
5. Add tests to `test/`
6. Update documentation
7. Test on all supported boards

### Code Style

- Use meaningful variable and function names
- Add comments for complex logic
- Follow existing code style
- Keep functions focused and small
- Use header guards in all headers
- Document public APIs

### Testing Requirements

- All new features must have tests
- Tests must pass on all supported boards
- Build tests must complete without errors
- Memory usage must remain reasonable
- Performance benchmarks must be maintained

---

## Related Documentation

- [Features Guide](FEATURES.md) - Complete feature descriptions
- [Command Reference](COMMAND_REFERENCE.md) - All commands
- [Test Infrastructure](technical/TEST_INFRASTRUCTURE.md) - Testing details
- [Implementation Summary](technical/IMPLEMENTATION_SUMMARY.md) - System overview
- [Quick Start Guide](../README.md#quick-start) - Getting started
