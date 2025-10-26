# ESP32 WiFi Utility Suite - Release Notes v4.3.0

**Release Date:** October 25, 2025  
**Version:** 4.3.0  
**Type:** Major Feature Release

---

## 🎉 Release Highlights

Version 4.3.0 introduces **four major network analysis tools** that transform the ESP32 WiFi Utility into a comprehensive network diagnostics platform:

1. **📶 Signal Strength Monitor** - Real-time WiFi signal monitoring with auto-scan
2. **🔒 Port Scanner** - Network security auditing and service discovery
3. **📊 Channel Graph** - Visual spectrum analysis with interactive charts
4. **📱 TFT Display Support** - Built-in screen integration for Feather boards

---

## 🚀 New Features

### 1. Signal Strength Monitor

**Real-time WiFi signal analysis with automated scanning**

#### Core Features
- **Live Signal Monitoring**: Current connection strength with 3-second auto-refresh
- **Nearby Networks Scanner**: Comprehensive scan with signal quality indicators
- **Auto-Scan Mode**: Configurable intervals (5, 10, 15, 30, 60 seconds)
- **Hidden Network Detection**: Identifies hidden SSIDs as `<Hidden Network>`
- **Visual Quality Indicators**: Color-coded circles (🟢🟡🟠🔴)
- **RSSI to Quality Conversion**: Automatic percentage calculation (0-100%)
- **Quality Classifications**: Excellent/Good/Fair/Weak/Very Weak
- **Non-Blocking Updates**: Page content remains visible during scans
- **Real-Time Countdown**: Shows time until next automatic scan

#### Web Interface
- **URL**: `http://<device-ip>/signal`
- **Professional UI**: Cyan gradient theme
- **Mobile Responsive**: Optimized for all screen sizes
- **API Endpoints**: 
  - `/signal` - Main monitoring page
  - `/signal/api?current=1` - Get current signal (JSON)
  - `/signal/api?scan=1` - Scan networks (JSON)

#### Integration
- Added to Analysis Dashboard with dedicated card
- Accessible from navigation dropdown menu
- Quick Actions button for one-click access

#### Use Cases
- Monitor connection quality in real-time
- Track signal changes during troubleshooting
- Identify best network in range
- Detect hidden networks
- Automated network monitoring

---

### 2. Port Scanner

**Comprehensive network security auditing tool**

#### Scan Types

**1. Common Ports (Fast) - ~20-30 seconds**
- 16 most commonly used ports
- Quick security assessment
- Ideal for routine checks
- Ports: FTP, SSH, HTTP, HTTPS, SMTP, MySQL, RDP, etc.

**2. Well-Known Ports - ~10-30 minutes**
- Ports 1-1024
- Standard IANA services
- Comprehensive audit
- Enterprise security scanning

**3. Custom Range - Variable duration**
- User-defined port range
- Targeted analysis
- Flexible scanning
- Example: Scan ports 8000-9000

**4. All Ports (Comprehensive) - ~8-12 hours**
- Ports 1-65535
- Complete security audit
- Deep analysis
- Full port coverage

#### Service Identification

Automatically identifies **25+ common services**:
- **Web**: HTTP (80), HTTPS (443), HTTP-Alt (8080)
- **Remote Access**: SSH (22), Telnet (23), RDP (3389), VNC (5900)
- **Email**: SMTP (25), POP3 (110), IMAP (143)
- **Databases**: MySQL (3306), PostgreSQL (5432), MongoDB (27017), Redis (6379)
- **File Transfer**: FTP (21), SMB (445)
- **And more**: DNS, Oracle, MSSQL, Printer services

#### Features
- **Real-Time Progress**: Animated progress bar with percentage
- **Background Scanning**: Non-blocking TCP connection tests
- **Results Display**: Tabular format (Port/Service/Status)
- **Gateway Detection**: Auto-populated target IP
- **Security Warnings**: Legal disclaimers and authorization reminders
- **Professional UI**: Purple gradient theme

#### Web Interface
- **URL**: `http://<device-ip>/portscan`
- **API Endpoints**:
  - `/portscan/start` - Start scan with parameters
  - `/portscan/stop` - Stop active scan
  - `/portscan/status` - Get scan progress (JSON)
  - `/portscan/api` - Helper endpoints

#### Integration
- Added to Analysis Dashboard with purple card
- Navigation dropdown access
- Quick Actions integration
- Last scan results display

#### Security Features
- Clear legal warnings
- Authorization requirements
- Network impact notices
- Responsible use guidelines
- Ethical scanning practices

---

### 3. Channel Graph Visualization

**Interactive WiFi spectrum analysis with HTML5 Canvas**

#### Visual Features

**Bar Chart Display**:
- 14 vertical bars for channels 1-14
- Bar height = congestion level (0-100%)
- Color-coded visualization
- Network count overlays

**Color-Coded Congestion**:
- 🟢 **Green (0-40%)**: Low congestion - Excellent choice
- 🟡 **Yellow (40-70%)**: Medium congestion - Acceptable
- 🔴 **Red (70-100%)**: High congestion - Avoid

**Professional Graph Elements**:
- Grid lines for easy reading
- Labeled axes (X: Channels, Y: Congestion %)
- Axis titles and descriptions
- Interactive legend
- Recommended channel markers (⭐)

#### Features
- **Best Channel Detection**: Automatic identification
- **Non-Overlapping Indicators**: Highlights channels 1, 6, 11
- **Network Count Display**: Shows networks per channel
- **Responsive Design**: Adapts to screen size
- **Window Resize Support**: Redraws on window change
- **Educational Content**: Explains channel overlap

#### Technical Implementation
- **Canvas Size**: 1000x400px (responsive)
- **Client-Side Rendering**: Pure JavaScript
- **Real-Time Data**: From channel analyzer
- **No External Dependencies**: Self-contained

#### Web Interface
- **URL**: `http://<device-ip>/channel/graph`
- **Access**: Button on Channel Analysis page
- **Integration**: Analysis Dashboard link

#### Use Cases
- Identify optimal WiFi channel
- Visual spectrum analysis
- Interference detection
- Router channel selection
- Multi-AP deployment planning

#### Educational Value
- Understanding channel overlap
- 2.4GHz spectrum visualization
- Non-overlapping channels (1, 6, 11)
- Congestion impact on performance
- Best practices for channel selection

---

### 4. TFT Display Support

**Built-in screen integration for Adafruit Feather ESP32-S3 boards**

#### Hardware Support
- **Adafruit Feather ESP32-S3 TFT**
- **Adafruit Feather ESP32-S3 Reverse TFT**
- **Display**: 135x240 pixels, ST7789 driver
- **Auto-Detection**: Automatically uses TFT when available

#### AP Mode Display

**Visual Elements**:
- 🟢 Green "AP Mode" text indicator
- QR code for instant WiFi connection
- SSID display (truncated if long)
- Password display
- AP IP address

**Features**:
- Scan QR with mobile device to connect
- Compact layout optimized for small screen
- Clear, readable information
- Professional styling

#### Station Mode Display

**Visual Elements**:
- 🔵 Blue "Station Mode" text indicator
- Connection status with visual feedback
- Connected network SSID
- Local IP address
- Real-time signal strength (RSSI in dBm)
- Color-coded signal quality indicator
- Visual signal quality bar graph

**Signal Quality Colors**:
- 🟢 **Green**: Strong signal (-60 dBm or better)
- 🟡 **Yellow**: Medium signal (-60 to -70 dBm)
- 🔴 **Red**: Weak signal (below -70 dBm)

**Signal Quality Bar**:
- Horizontal progress bar (0-100%)
- Calculated from RSSI value
- Visual representation of connection quality
- Updates in real-time

#### Features
- **Dynamic Mode Detection**: Uses `esp_wifi_get_mode()`
- **Automatic Updates**: Displays appropriate mode info
- **QR Code Generation**: Instant connection via scan
- **Signal Visualization**: Live signal strength tracking
- **Compact Layout**: Optimized for 135x240 screen

#### Libraries Added
- **Adafruit ST7735 and ST7789 Library** @^1.10.0
- **Adafruit GFX Library** @^1.11.0

---

## 🌐 Web Interface Enhancements

### Analysis Dashboard Updates

**New Cards Added**:

1. **Signal Monitor Card** (Cyan gradient):
   - Current signal strength display
   - Last scan timestamp
   - Quality indicator
   - Quick access button

2. **Port Scanner Card** (Purple gradient):
   - Last scan results summary
   - Open ports count
   - Security status
   - Quick launch button

**Enhanced Features**:
- Channel Graph button on Channel Analysis page
- Updated navigation dropdown with all new tools
- Quick Actions section with one-click access
- Tips section with tool guidance
- Consistent gradient styling throughout

### Navigation Improvements

**Analysis Dropdown Menu** now includes:
- 📊 Dashboard
- 📶 Signal (NEW)
- 🔒 Port Scanner (NEW)
- ⚡ iPerf
- 📉 Latency
- 📡 Channel (with Graph option)

**Quick Actions Section**:
- One-click access to all analysis tools
- Gradient-styled buttons
- Icon indicators
- Mobile-optimized

---

## 📚 Documentation

### New Documentation Files

1. **`docs/SIGNAL_STRENGTH_MONITOR.md`** (~400 lines)
   - Complete signal monitor guide
   - Features and usage
   - API endpoints
   - Configuration options
   - Troubleshooting

2. **`docs/PORT_SCANNER.md`** (~476 lines)
   - Comprehensive port scanner documentation
   - Scan types and service identification
   - Security considerations
   - Usage examples
   - Legal and ethical guidelines

3. **`docs/CHANNEL_GRAPH.md`** (~595 lines)
   - Visual spectrum analysis guide
   - Graph interpretation
   - Channel selection best practices
   - Technical implementation details
   - Educational content on WiFi spectrum

### Updated Documentation

- **README.md**: Added v4.3.0 features section (~150 lines)
- **CHANGELOG.md**: Complete v4.3.0 release notes (~200 lines)
- **Version badges**: Updated to 4.3.0

---

## 🔧 Technical Details

### New Source Files

1. **Signal Monitor**:
   - `include/signal_monitor.h`
   - `src/signal_monitor.cpp`

2. **Port Scanner**:
   - `include/port_scanner.h`
   - `src/port_scanner.cpp`

3. **TFT Display**:
   - `include/tft_display.h`
   - `src/tft_display.cpp`

### Modified Files

- `include/web_server.h` - Added new handler declarations
- `src/web_server.cpp` - New routes and pages (~800 lines added)
- `src/main.cpp` - TFT and signal monitor initialization
- `src/wifi_manager.cpp` - TFT display integration

### Build Configuration

**Flash Usage**:
- **ESP32dev**: 90.7% (1,188,777 bytes) - increased by ~10KB
- **Feather ESP32-S3 TFT**: 80.0% (1,153,357 bytes)
- **All 6 environments**: Build successfully

**Library Dependencies Added**:
- Adafruit ST7735 and ST7789 Library @^1.10.0
- Adafruit GFX Library @^1.11.0

---

## 📊 Statistics

### Code Additions

| Component | Lines Added |
|-----------|-------------|
| Signal Monitor | ~400 |
| Port Scanner | ~650 |
| Channel Graph | ~260 |
| TFT Display | ~320 |
| Web Pages | ~800 |
| Documentation | ~1,500 |
| **Total** | **~3,930** |

### Feature Counts

- **New Web Pages**: 3 (`/signal`, `/portscan`, `/channel/graph`)
- **New API Endpoints**: 8
- **New Documentation Files**: 3
- **Service Identifications**: 25+
- **Scan Configurations**: 4 types
- **Auto-Scan Intervals**: 5 options
- **TFT Display Modes**: 2 (AP/Station)

---

## 🎯 Use Cases

### For Home Users

1. **WiFi Optimization**:
   - Use Signal Monitor to find best network
   - Use Channel Graph to select optimal channel
   - Monitor connection quality over time

2. **Security**:
   - Scan router for unexpected open ports
   - Verify only required services are exposed
   - Regular security audits

3. **Troubleshooting**:
   - Identify signal strength issues
   - Detect channel congestion
   - Monitor network stability

### For IT Professionals

1. **Network Deployment**:
   - Channel planning with visual spectrum
   - Multi-AP deployment optimization
   - Interference mitigation

2. **Security Auditing**:
   - Comprehensive port scanning
   - Service discovery
   - Vulnerability assessment

3. **Performance Optimization**:
   - Signal strength monitoring
   - Channel congestion analysis
   - Network quality tracking

### For Network Administrators

1. **Site Surveys**:
   - Visual spectrum analysis
   - Signal coverage mapping
   - Interference identification

2. **Maintenance**:
   - Regular security scans
   - Performance monitoring
   - Capacity planning

3. **Documentation**:
   - Network topology mapping
   - Service inventory
   - Security compliance reporting

---

## ⚠️ Important Notes

### Port Scanner Disclaimer

**Legal and Ethical Use**:
- Only scan devices you own or have explicit permission to scan
- Unauthorized port scanning may be illegal in your jurisdiction
- Comply with organizational security policies
- Check local laws before scanning
- Use responsibly and ethically

**Network Impact**:
- Port scanning generates network traffic
- May trigger IDS/IPS alerts
- Can affect network performance
- Should be scheduled during maintenance windows

### TFT Display Requirements

**Hardware**:
- Only works on Adafruit Feather ESP32-S3 TFT boards
- Regular ESP32 boards do not have built-in display
- Reverse TFT variant also supported

**Libraries**:
- Automatically included for TFT board builds
- No manual installation required
- Included in platformio.ini configuration

### Channel Graph Considerations

**Browser Compatibility**:
- Requires HTML5 Canvas support
- Works on modern browsers (Chrome, Firefox, Safari, Edge)
- Mobile browsers supported
- Internet Explorer not supported

---

## 🔄 Upgrade Instructions

### From v4.2.0 to v4.3.0

**No configuration changes required!**

All existing features remain fully compatible. New features are additions only.

**Steps**:
1. Flash new firmware to device
2. No configuration reset needed
3. Access new features via web interface
4. Documentation available in `/docs` folder

**What's Preserved**:
- ✅ AP configuration
- ✅ Station configuration
- ✅ All existing settings
- ✅ All existing commands
- ✅ Web interface compatibility

**What's New**:
- ✅ Signal Monitor at `/signal`
- ✅ Port Scanner at `/portscan`
- ✅ Channel Graph at `/channel/graph`
- ✅ TFT display support (if hardware available)
- ✅ Enhanced Analysis Dashboard

---

## 🐛 Bug Fixes

This release focuses on new features. No critical bugs fixed.

**Minor Improvements**:
- Navigation menu consistency
- Button styling improvements
- Mobile responsiveness enhancements
- Page layout optimizations

---

## 🔮 Future Roadmap

### Planned for v4.4.0

- 5GHz spectrum analysis
- Network device discovery
- Bandwidth usage monitoring
- Enhanced security scanning
- Export functionality (CSV/JSON)
- Historical data tracking

---

## 📞 Support

### Getting Help

**Documentation**:
- README.md - Quick start guide
- CHANGELOG.md - Detailed change history
- docs/ - Feature-specific guides

**Online Resources**:
- GitHub Issues - Bug reports and feature requests
- GitHub Wiki - Extended documentation
- GitHub Discussions - Community support

### Reporting Issues

Please report bugs and feature requests on GitHub Issues with:
- Clear description
- Steps to reproduce
- Expected vs actual behavior
- Hardware platform (ESP32dev/Feather)
- Firmware version

---

## 👥 Contributors

This release was made possible by:
- Core development team
- Community feedback
- Feature requests
- Bug reports
- Documentation contributions

Thank you to everyone who contributed!

---

## 📜 License

ESP32 WiFi Utility Suite is released under the MIT License.

See LICENSE file for full details.

---

## 🎉 Conclusion

Version 4.3.0 represents a **major milestone** in the ESP32 WiFi Utility Suite evolution, transforming it from a WiFi management tool into a **comprehensive network analysis platform**.

**Key Achievements**:
- ✅ 4 major features added
- ✅ ~4,000 lines of new code
- ✅ Professional documentation
- ✅ Enhanced user experience
- ✅ Expanded use cases

**Download**: Get the latest release from GitHub

**Enjoy your enhanced WiFi analysis capabilities!** 🚀

---

**Release Date**: October 25, 2025  
**Version**: 4.3.0  
**Build**: All 6 environments pass  
**Status**: Production Ready ✅
