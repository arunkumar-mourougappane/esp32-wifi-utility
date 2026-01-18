# ESP32 WiFi Utility Suite

![Build Status](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/actions/workflows/build.yml/badge.svg)
![Version](https://img.shields.io/badge/version-5.2.1-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-green.svg)
![PlatformIO](https://img.shields.io/badge/build-PlatformIO-orange.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Boards](https://img.shields.io/badge/boards-ESP32dev%20|%20Feather%20ESP32--S3%20|%20Reverse%20TFT-brightgreen.svg)
[![Changelog](https://img.shields.io/badge/changelog-available-brightgreen.svg)](CHANGELOG.md)

A professional-grade ESP32 WiFi analysis and management suite featuring comprehensive network scanning, signal strength monitoring, port scanning, visual spectrum analysis, TFT display support with QR codes, performance testing, tri-board support, persistent configuration storage, instant mode switching via web interface, and structured logging.

## 📚 Documentation

- **[Quick Start Guide](#quick-start)** - Get up and running in minutes
- **[Features Overview](docs/features/FEATURES.md)** - Complete feature descriptions
- **[Command Reference](docs/reference/COMMAND_REFERENCE.md)** - All available commands
- **[Usage Examples](docs/reference/USAGE_EXAMPLES.md)** - Practical examples
- **[Version History](docs/releases/VERSION_HISTORY.md)** - Release notes and changelog
- **[Architecture Guide](docs/architecture/ARCHITECTURE.md)** - Technical implementation
- **[Security Configuration](docs/security/SECURITY_CONFIGURATION.md)** - WiFi security setup
- **[GitHub Wiki](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki)** - Comprehensive guides

## 🎉 Latest Updates

**Version 5.2.1** - Documentation patch release with corrected TFT UI screenshot paths.

**Version 5.2.0** - Complete TFT display UI redesign with modular architecture, color-coded status screens, and branded welcome experience.

**Version 5.1.0** - Real-time latency testing UI improvements with live progress bars and statistics.

See [Version History](docs/releases/VERSION_HISTORY.md) for complete release notes and [CHANGELOG.md](CHANGELOG.md) for detailed changes.

## ✨ Key Features

### 🔧 Core Capabilities

- **Multiple WiFi Modes**: Station, Access Point, Idle, and Off modes with seamless switching
- **Advanced Network Scanning**: Enhanced WiFi scanner with visual indicators and security assessment
- **Professional Channel Analysis**: AI-powered spectrum analysis with congestion scoring
- **Network Performance Testing**: Comprehensive latency analysis and iPerf bandwidth testing
- **Port Scanning**: Network security auditing with service identification
- **Signal Strength Monitor**: Real-time signal analysis with auto-scanning

### 🌐 User Interfaces

- **Web Interface**: Professional browser-based control on all boards
  - Mobile-responsive design with hamburger menu
  - Interactive WiFi scanning with clickable network details
  - Real-time monitoring and status dashboard
  - Configuration management with instant mode switching
- **TFT Display**: Built-in screen support for Feather ESP32-S3 boards
  - Color-coded status screens (blue/green/red/yellow)
  - Branded welcome screen
  - QR codes for instant connection
  - Real-time signal strength visualization
- **Serial Interface**: Interactive command-line with smart prompts

### 💾 Configuration & Management

- **Persistent Storage**: NVS-based configuration for AP and Station settings
- **Security Options**: WPA2, WPA3, Mixed mode, Open networks
- **QR Code Generation**: Automatic QR codes for easy mobile connection
- **Client Management**: Monitor and control connected devices in AP mode
- **Auto-Connect**: Automatic connection to saved networks on boot

### 🔧 Hardware Support

- **ESP32 Development Board**: Full features with standard LED
- **Adafruit Feather ESP32-S3 TFT**: Enhanced with NeoPixel RGB and TFT display
- **Adafruit Feather ESP32-S3 Reverse TFT**: Same features with reversed display orientation

See [Features Guide](docs/features/FEATURES.md) for complete feature descriptions.

## 🛠️ Hardware Requirements

### Supported Boards

| Board | LED | Display | Memory | Features |
|-------|-----|---------|--------|----------|
| ESP32 Development Board | Built-in LED (GPIO 2) | None | 4MB Flash, 320KB RAM | Full functionality |
| Feather ESP32-S3 TFT | NeoPixel RGB (GPIO 48) | 1.14" TFT (240x135) | 4MB Flash, 8MB PSRAM | Enhanced visual feedback |
| Feather ESP32-S3 Reverse TFT | NeoPixel RGB (GPIO 48) | 1.14" TFT (reversed) | 4MB Flash, 8MB PSRAM | Enhanced visual feedback |

### Universal Requirements

- USB cable for programming
- Computer with PlatformIO or Arduino IDE
- Serial terminal (115200 baud)

## 📦 Pre-Built Firmware Downloads

**🎯 Want to use it immediately?** Download pre-built firmware from our [latest release](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/releases/latest):

- **ESP32dev**: `esp32-wifi-utility-esp32dev-v{version}.bin`
- **Feather ESP32-S3 TFT**: `esp32-wifi-utility-feather-s3-tft-v{version}.bin`
- **Feather ESP32-S3 Reverse TFT**: `esp32-wifi-utility-feather-s3-reversetft-v{version}.bin`

### Flash Pre-Built Firmware

```bash
# Using esptool (install with: pip install esptool)
esptool.py --port /dev/ttyUSB0 write_flash 0x10000 firmware.bin

# Or use Arduino IDE / PlatformIO for uploading
```

## ⚡ Quick Start

### 1. Hardware Setup

**ESP32 Development Board:**
1. Connect your ESP32dev to your computer via USB
2. Ensure the built-in LED is available (GPIO 2)

**Adafruit Feather ESP32-S3 TFT:**
1. Connect your Feather board to your computer via USB
2. The NeoPixel LED will provide enhanced RGB status indication

### 2. Software Setup

1. Install [PlatformIO](https://platformio.org/) or [Arduino IDE](https://www.arduino.cc/en/software)
2. Clone or download this project
3. Open the project in your IDE

### 3. Build and Upload

#### For ESP32 Development Board:

```bash
# Using PlatformIO (recommended)
pio run -e esp32dev -t upload

# Monitor serial output
pio device monitor
```

#### For Adafruit Feather ESP32-S3 TFT:

```bash
# Using PlatformIO with NeoPixel support
pio run -e adafruit_feather_esp32s3_tft -t upload

# Monitor serial output
pio device monitor
```

#### For Adafruit Feather ESP32-S3 Reverse TFT:

```bash
# Using PlatformIO with NeoPixel support
pio run -e adafruit_feather_esp32s3_reversetft -t upload

# Monitor serial output
pio device monitor
```

### 4. Connect to Serial Monitor

- **Baud Rate**: 115200
- **Line Ending**: Newline (NL)

### 5. Basic Commands

```bash
mode ap               # Start Access Point
ap info               # Show AP details
qr                    # Display QR code

mode station          # Switch to station mode
scan now              # Scan for networks
connect "MyWiFi" "password"  # Connect to network

webserver start       # Launch web interface
status                # Show device status
help                  # List all commands
```

See [Command Reference](docs/reference/COMMAND_REFERENCE.md) for complete command documentation and [Usage Examples](docs/reference/USAGE_EXAMPLES.md) for practical examples.

## 🔧 Configuration

### WiFi Access Point Settings

Edit in `include/config.h` for default AP configuration:

```cpp
#define AP_SSID "ESP32-WiFiScanner"     // Default AP network name
#define AP_PASSWORD "123456789"         // Default AP password (min 8 chars)
```

Or use runtime commands:

```bash
ap save MyHotspot SecurePass123 6 wpa2 yes  # Save custom AP config
station save HomeNetwork WiFiPass123 auto yes  # Save station config
```

See [Security Configuration](docs/security/SECURITY_CONFIGURATION.md) for detailed security setup.

## 🐛 Troubleshooting

### Common Issues

**Compilation Errors**
- Ensure you're using ESP32 board package
- Verify PlatformIO configuration
- Check all dependencies are installed

**Serial Commands Not Working**
- Check baud rate is 115200
- Ensure line ending is "Newline" (NL)
- Verify USB connection

**WiFi Not Scanning**
- Switch to station mode: `mode station`
- Enable scanning: `scan on`
- Check WiFi is available

**Access Point Not Visible**
- Verify AP mode: `mode ap`
- Check SSID in nearby networks
- Ensure password is 8+ characters

**Web Server Not Accessible**
- Start web server: `webserver start`
- Check displayed IP address
- Verify device is in AP or Station mode
- Check firewall settings

### Debug Commands

```bash
status          # Check current mode and settings
help            # Review available commands
mode idle       # Return to standby state
clear           # Clear screen
```

For more troubleshooting, see the [GitHub Wiki](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki).

## 🧪 Running Tests

```bash
# Test on ESP32 Development Board
pio test --environment test

# Test on Feather ESP32-S3 TFT
pio test --environment test_feather

# Build tests without uploading
pio test --environment test --without-uploading
```

**Test Coverage**: 19 test cases covering cache management, network details, WiFi fundamentals, and system integration. All tests pass on both boards.

## 📊 Technical Specifications

### Memory Usage

- **ESP32dev**: 77.6% flash, 15.8% RAM, ~275KB free heap
- **Feather ESP32-S3**: 68.7% flash, 19.1% RAM, ~265KB free heap

### Performance

- **Scan Speed**: 3-5 seconds per comprehensive scan
- **Response Time**: <100ms for commands
- **Channel Analysis**: <5s quick, <30s detailed
- **WiFi Protocols**: 802.11 b/g/n (2.4 GHz)
- **iPerf Throughput**: 10-20 Mbps TCP

### Code Statistics

- **Total Lines**: ~4,500+ lines of organized code
- **Zero Warnings**: Clean compilation on all platforms
- **Test Coverage**: 19 comprehensive test cases
- **Documentation**: 1,000+ pages across multiple guides

See [Architecture Guide](docs/architecture/ARCHITECTURE.md) for detailed technical information.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

## 📄 License

This project is open source under the MIT License. See [LICENSE](LICENSE) for details.

## 👤 Maintainer

**Maintained by:** [Arunkumar Mourougappane](https://github.com/arunkumar-mourougappane)

## 📞 Support

If you encounter any issues or have questions:

1. Check the [troubleshooting section](#troubleshooting) above
2. Review the [command reference](docs/reference/COMMAND_REFERENCE.md)
3. Check the [GitHub Wiki](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/wiki)
4. Open an issue on the project repository

---

**Happy WiFi Scanning! 📡**
