# Signal Strength Monitor

Real-time WiFi signal strength monitoring and analysis feature for ESP32 WiFi Utility.

## Overview

The Signal Strength Monitor provides comprehensive real-time monitoring of WiFi signal strength (RSSI) for both the connected network and nearby networks. It includes both Serial command interface and web-based graphical interface.

## Features

- **Real-time Signal Monitoring**: Continuous monitoring of connected network signal strength
- **Nearby Network Scanning**: Scan and display signal strength of all nearby WiFi networks
- **Multiple Interfaces**: Serial commands and web-based GUI
- **Signal Quality Metrics**: RSSI (dBm), Quality percentage, Quality rating
- **Visual Signal Meters**: ASCII art meters in Serial, graphical bars in web interface
- **Auto-refresh**: Web interface automatically updates every 3 seconds

## Signal Strength Reference

### RSSI (Received Signal Strength Indicator)

| RSSI Range (dBm) | Quality | Percentage | Description |
|------------------|---------|------------|-------------|
| -30 to -50 | Excellent | 100% | Optimal signal, maximum performance |
| -50 to -60 | Good | 80-100% | Very good signal, reliable connection |
| -60 to -70 | Fair | 60-80% | Adequate signal, may experience slowdowns |
| -70 to -80 | Weak | 40-60% | Marginal signal, connection issues likely |
| -80 to -90 | Very Weak | 20-40% | Poor signal, frequent disconnections |
| Below -90 | Unusable | <20% | Connection not reliable |

## Serial Commands

### View Current Signal Strength

```bash
> signal show
# or
> signal status
```

Displays detailed information about the currently connected network:
- Network SSID
- RSSI in dBm
- Quality percentage and rating
- Visual signal strength meter

**Example Output:**
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Signal Strength Information
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Network:  HomeWiFi [CONNECTED]
RSSI:     -65 dBm
Quality:  70% (Fair)
Signal:   [████████████████████████████░░░░░░░░░░░░]
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### Scan Nearby Networks

```bash
> signal scan
```

Scans and displays signal strength of all nearby WiFi networks with:
- Network SSID
- RSSI in dBm
- Quality percentage
- Quality rating
- Connection status indicator
- Visual signal meter for each network

**Example Output:**
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Nearby Networks Signal Strength
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

 1. HomeWiFi                      -65 dBm   70%  Fair [CONNECTED]
    [████████████████████████████░░░░░░░░░░░░]
 2. NeighborNetwork               -72 dBm   56%  Weak
    [██████████████████████░░░░░░░░░░░░░░░░░░]
 3. CoffeeShop_Guest              -80 dBm   40%  Very Weak
    [████████████████░░░░░░░░░░░░░░░░░░░░░░░░]
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### Continuous Monitoring

```bash
> signal monitor
# or
> signal monitor start
```

Starts continuous signal strength monitoring with updates every 5 seconds. Shows:
- Timestamp
- Network SSID
- Current RSSI
- Quality percentage and rating
- Real-time signal meter

Press any key to stop monitoring.

**Example Output:**
```
Signal monitoring active. Press any key to stop.

[00:01:25] HomeWiFi: -65 dBm | 70% | Fair
          Signal: [███████░░░]
[00:01:30] HomeWiFi: -64 dBm | 72% | Fair
          Signal: [███████░░░]
[00:01:35] HomeWiFi: -66 dBm | 68% | Fair
          Signal: [███████░░░]
```

**Stop Monitoring:**
```bash
> signal monitor stop
```

### Get Help

```bash
> signal
```

Displays comprehensive help information about signal strength commands.

## Web Interface

### Access Signal Monitor Page

Navigate to: `http://[device-ip]/signal`

The web interface provides:

1. **Current Connection Panel**
   - Network SSID with connection status
   - Large RSSI display with color coding
   - Quality percentage and rating
   - Graphical signal strength bars
   - Auto-refreshes every 3 seconds

2. **Nearby Networks Panel**
   - "Scan Networks" button to trigger scan
   - Table view of all nearby networks
   - Sortable columns:
     - Network name (with connection indicator)
     - Graphical signal bars
     - RSSI value (color-coded)
     - Quality percentage and rating
   - Real-time visualization

### Color Coding

Signal strength is color-coded for quick assessment:

- **Green (#4CAF50)**: Excellent/Good (-30 to -60 dBm)
- **Light Green (#8BC34A)**: Fair (-60 to -70 dBm)  
- **Amber (#FFC107)**: Weak (-70 to -80 dBm)
- **Orange (#FF9800)**: Very Weak (-80 to -90 dBm)
- **Red (#F44336)**: Unusable (below -90 dBm)

### Signal Bars

10-bar graphical indicator with:
- Height increases left to right
- Filled bars in signal quality color
- Empty bars in gray
- Provides visual at-a-glance assessment

## API Endpoints

### Get Current Signal Strength

**Endpoint:** `GET /signal/api?current=1`

**Response:**
```json
{
  "connected": true,
  "ssid": "HomeWiFi",
  "rssi": -65,
  "quality": 70,
  "qualityText": "Fair",
  "timestamp": 12345678
}
```

### Scan Nearby Networks

**Endpoint:** `GET /signal/api?scan=1`

**Response:**
```json
{
  "count": 3,
  "networks": [
    {
      "ssid": "HomeWiFi",
      "rssi": -65,
      "quality": 70,
      "qualityText": "Fair",
      "connected": true
    },
    {
      "ssid": "NeighborNetwork",
      "rssi": -72,
      "quality": 56,
      "qualityText": "Weak",
      "connected": false
    }
  ]
}
```

## Use Cases

### 1. Troubleshooting Connection Issues

Monitor signal strength to identify:
- Weak signal causing disconnections
- Interference from other networks
- Need to relocate router or device
- Optimal placement for access points

### 2. Site Survey

Use signal scanning to:
- Map WiFi coverage in a building
- Identify dead zones
- Plan AP placement
- Assess interference levels

### 3. Performance Optimization

Compare signal strengths to:
- Choose best network to connect to
- Verify signal improvements after changes
- Monitor signal stability over time
- Identify intermittent issues

### 4. Network Planning

Use data to:
- Determine AP coverage area
- Identify channel interference
- Plan network expansion
- Optimize existing infrastructure

## Implementation Details

### Files Added

- **include/signal_monitor.h**: Header file with function declarations and structures
- **src/signal_monitor.cpp**: Implementation of signal monitoring functionality

### Files Modified

- **include/web_server.h**: Added signal monitor handlers
- **src/web_server.cpp**: Implemented web interface and API endpoints
- **src/command_interface.cpp**: Added signal commands
- **src/main.cpp**: Integrated signal monitoring update loop

### Key Functions

**Signal Retrieval:**
- `getCurrentSignalStrength()`: Get current connection signal
- `getNearbySignalStrengths()`: Scan and return nearby networks
- `rssiToQuality()`: Convert RSSI to percentage
- `rssiToQualityText()`: Convert RSSI to descriptive text

**Display:**
- `printSignalInfo()`: Display signal info to Serial
- `displaySignalMeter()`: Show ASCII signal meter
- `updateSignalMonitoring()`: Continuous monitoring update

**Web Handlers:**
- `handleSignalMonitor()`: Serve web interface page
- `handleSignalStrengthAPI()`: JSON API for AJAX requests

## Technical Specifications

### Scan Parameters

- **Maximum Networks**: 20 networks per scan (configurable)
- **Scan Type**: Non-blocking WiFi scan
- **Duplicate Handling**: Shows all instances of networks
- **Hidden Networks**: Not shown in scan

### Update Intervals

- **Serial Monitoring**: 5 seconds (default)
- **Web Auto-refresh**: 3 seconds
- **API Response Time**: ~50-200ms for current signal, 1-3 seconds for scan

### Memory Usage

- **SignalInfo Structure**: ~100 bytes per network
- **Cached Results**: Minimal (immediately processed)
- **Web Page**: ~8KB HTML/JavaScript
- **Impact**: Low (<10KB heap during operation)

## Tips & Best Practices

### For Best Results

1. **Position device properly**: Avoid metal enclosures and concrete walls
2. **Use external antenna**: If available, improves signal reception
3. **Regular monitoring**: Track signal changes over time
4. **Compare locations**: Test different placements
5. **Check interference**: Use channel analysis alongside signal monitoring

### Interpreting Results

- **-50 dBm or better**: Excellent, full speed possible
- **-60 to -70 dBm**: Normal operating range for most devices
- **-70 to -80 dBm**: Marginal, may experience performance degradation
- **Below -80 dBm**: Connection issues likely, consider improvements

### Common Issues

**Signal fluctuates rapidly:**
- Normal for WiFi, averaging over time provides better picture
- Use continuous monitoring to see trends
- Environmental interference (microwave, other devices)

**Cannot see nearby networks:**
- Device may be out of range
- Networks on different frequency (5GHz vs 2.4GHz)
- Hidden SSIDs not displayed
- Channel overlap may affect visibility

**Signal weaker than expected:**
- Check antenna connection
- Verify no physical obstructions
- Consider metal cases or shielding
- Distance from router/AP

## Future Enhancements

Potential improvements for future versions:

- **Signal history graphing**: Plot signal strength over time
- **Alert thresholds**: Notify when signal drops below threshold
- **Data export**: Save signal measurements to CSV/JSON
- **Heatmap generation**: Create coverage heatmaps
- **Multiple connection tracking**: Monitor multiple networks simultaneously
- **Predictive analysis**: Identify patterns and predict issues

## Related Features

- [WiFi Scanning](WiFi-Scanning-Analysis) - Network discovery and information
- [Channel Analysis](Channel-Analysis) - Spectrum analysis and optimization
- [Performance Testing](Performance-Testing) - Latency and throughput testing
- [Web Configuration](Web-Configuration-Interface) - Web-based control

---

**Feature Added:** v4.2.1  
**Dependencies:** WiFi.h (ESP32 core)  
**Stability:** Stable  
**Performance Impact:** Low

---

[← Back to Home](Home) | [View All Features](Home#features)