# 📡 Channel Congestion Scanner Guide

This guide covers the comprehensive WiFi channel congestion analysis features of the ESP32 WiFi Utility.

## 🎯 Overview

The Channel Congestion Scanner provides advanced spectrum analysis capabilities including:

- **Comprehensive Channel Analysis**: Detailed congestion scoring for all 2.4GHz channels (1-13)
- **Signal Strength Mapping**: RSSI analysis and network overlap detection
- **Intelligent Recommendations**: Smart channel selection based on congestion patterns
- **Interference Detection**: Non-WiFi interference pattern recognition
- **Continuous Monitoring**: Background spectrum monitoring with periodic updates
- **Visual Feedback**: NeoPixel status indicators during scanning (Feather board only)
- **Export Capabilities**: JSON data export and optimization reports

## 📋 Available Commands

### Core Commands

| Command | Description |
|---------|-------------|
| `channel` | Show channel congestion analysis help |
| `channel scan` | Comprehensive channel congestion analysis |
| `channel quick` | Quick channel congestion check |
| `congestion` | Quick congestion summary |
| `spectrum` | Full spectrum analysis with recommendations |

### Advanced Commands

| Command | Description |
|---------|-------------|
| `channel monitor start` | Start continuous channel monitoring |
| `channel monitor stop` | Stop channel monitoring |
| `channel monitor status` | Show monitoring status |
| `channel recommendations` | Show optimal channel recommendations |
| `channel report` | Generate optimization report |
| `channel export` | Export analysis data in JSON format |

### Usage Examples

```bash
# Quick congestion check
ESP32> congestion

# Comprehensive channel analysis
ESP32> channel scan

# Full spectrum analysis with recommendations
ESP32> spectrum

# Start continuous monitoring (30-second intervals)
ESP32> channel monitor start

# Get specific recommendations
ESP32> channel recommendations

# Export data for external analysis
ESP32> channel export
```

## 🔧 Analysis Features

### 1. Congestion Scoring (0-100%)
The system calculates a comprehensive congestion score for each channel based on:
- **Network Count**: Number of detected networks (0-50 points)
- **Signal Strength**: Strongest signal impact (0-30 points)
- **Average RSSI**: Overall signal level (0-20 points)

**Scoring Guidelines**:
- **0-30%**: Low congestion (excellent for AP use)
- **30-70%**: Medium congestion (acceptable with monitoring)
- **70-100%**: High congestion (avoid for new AP deployment)

### 2. Channel Overlap Analysis
WiFi channels in the 2.4GHz band overlap significantly:
- **Channel 1**: Overlaps with channels 1-5
- **Channel 6**: Overlaps with channels 4-8  
- **Channel 11**: Overlaps with channels 9-13

The system analyzes overlap patterns to provide accurate interference predictions.

### 3. Interference Detection
Advanced pattern recognition identifies:
- **Non-WiFi Interference**: Microwave ovens, Bluetooth devices, etc.
- **Unusual Signal Patterns**: Large RSSI gaps indicating external interference
- **Non-Standard Channel Usage**: Heavy traffic on overlapping channels

### 4. Smart Recommendations
The recommendation engine prioritizes:
1. **Standard Non-Overlapping Channels**: 1, 6, 11 (preferred)
2. **Low Congestion Scores**: Channels with minimal traffic
3. **Signal Isolation**: Channels with reduced interference
4. **Future-Proofing**: Considerations for network growth

## 📊 Sample Output

### Quick Congestion Summary
```
📊 === Channel Congestion Summary ===
🟢 Low congestion channels: 3
🟡 Medium congestion channels: 6  
🔴 High congestion channels: 4
📊 Overall spectrum utilization: 64.2%

📈 Busiest Channels:
  1. Channel 6: 8 networks
  2. Channel 1: 5 networks
  3. Channel 11: 4 networks
```

### Comprehensive Analysis
```
📡 === Comprehensive Channel Congestion Analysis ===
⏰ Scan Time: 3240 ms | Networks Found: 23 | Overall Congestion: 64.2%
┌────┬─────────┬──────────┬───────┬─────────────────────────┬────────────┐
│ CH │ Networks│ Congestion│  RSSI │      Dominant Network   │ Overlap │ R │
├────┼─────────┼──────────┼───────┼─────────────────────────┼────────────┤
│  1 │       5 │██████░░░░│ -42dBm│ MyHomeNetwork          │         12 │ Y │
│  2 │       3 │████░░░░░░│ -55dBm│ Neighbor_WiFi          │         18 │ N │
│  3 │       2 │███░░░░░░░│ -61dBm│ CoffeeShop_Guest       │         15 │ N │
│  4 │       1 │██░░░░░░░░│ -68dBm│ Office_Network         │          8 │ N │
│  5 │       0 │░░░░░░░░░░│   N/A │                        │          6 │ N │
│  6 │       8 │████████░░│ -38dBm│ StrongSignal_5G        │         20 │ Y │
│  7 │       2 │███░░░░░░░│ -59dBm│ Mobile_Hotspot         │         11 │ N │
│  8 │       1 │██░░░░░░░░│ -72dBm│ IoT_Devices            │          9 │ N │
│  9 │       0 │░░░░░░░░░░│   N/A │                        │          3 │ N │
│ 10 │       1 │██░░░░░░░░│ -74dBm│ SmartHome_Hub          │          4 │ N │
│ 11 │       4 │██████░░░░│ -45dBm│ Enterprise_AP          │          5 │ Y │
│ 12 │       0 │░░░░░░░░░░│   N/A │                        │          4 │ N │
│ 13 │       0 │░░░░░░░░░░│   N/A │                        │          0 │ N │
└────┴─────────┴──────────┴───────┴─────────────────────────┴────────────┘
📊 Best Channel: 5 (0.0% congestion)
⚠️  Worst Channel: 6 (78.5% congestion)
R = Recommended for AP use
```

### Channel Recommendations
```
💡 === Channel Recommendations ===
🎯 Recommended channels for AP deployment (in order of preference):
  1. Channel 5 - 0.0% congestion, 0 networks, 2432 MHz
  2. Channel 13 - 0.0% congestion, 0 networks, 2472 MHz  
  3. Channel 1 - 45.2% congestion, 5 networks, 2412 MHz
  4. Channel 11 - 52.8% congestion, 4 networks, 2462 MHz
  5. Channel 6 - 78.5% congestion, 8 networks, 2437 MHz

📋 Recommendation factors:
  • Channels 1, 6, 11 are preferred (non-overlapping)
  • Lower congestion scores are better
  • Fewer networks mean less competition
  • Consider signal strength and overlap
```

## 🔄 Continuous Monitoring

### Background Monitoring
```bash
# Start monitoring with 30-second intervals
ESP32> channel monitor start

# Check monitoring status
ESP32> channel monitor status
Channel Monitoring: ACTIVE (interval: 30s, last update: 15s ago) | Last scan: 45s ago

# Stop monitoring
ESP32> channel monitor stop
```

### Monitoring Benefits
- **Trend Analysis**: Track congestion changes over time
- **Interference Detection**: Identify temporary interference sources
- **Performance Correlation**: Link network performance to spectrum conditions
- **Automatic Updates**: Stay informed of spectrum changes

## 🎨 Visual Status Indicators (Feather Board)

When using the Adafruit Feather ESP32-S3 TFT board with NeoPixel:

| Color | Status |
|-------|---------|
| 🔵 **Blue** | Channel scanning in progress |
| 🟢 **Green** | Scan completed successfully |
| 🔴 **Red** | Scan error or failure |
| 🟡 **Yellow** | High congestion detected |

## 📊 Technical Specifications

### Channel Coverage
- **2.4GHz Band**: Channels 1-13 (2412-2472 MHz)
- **Channel Spacing**: 5 MHz between adjacent channels
- **Channel Width**: 20 MHz (±10 MHz from center frequency)
- **Overlap Pattern**: Each channel overlaps with ±2 adjacent channels

### Measurement Accuracy
- **RSSI Precision**: ±2 dBm typical
- **Frequency Coverage**: Complete 2.4 GHz ISM band
- **Scan Duration**: 1-5 seconds depending on detail level
- **Update Rate**: Configurable from 10 seconds to 10 minutes

### Memory Usage
- **Flash Impact**: ~15KB additional code
- **RAM Usage**: ~4KB during active scanning
- **Data Storage**: Up to 100 network records per scan
- **Export Size**: ~2KB JSON per scan result

## 🔧 Configuration Options

### Scan Configuration
```cpp
typedef struct {
    bool include_hidden_networks;       // Include hidden SSIDs (default: true)
    uint16_t scan_duration_ms;          // Scan duration (default: 3000ms)
    bool detailed_analysis;             // Enable interference detection (default: true)
    bool continuous_monitoring;         // Background monitoring (default: false)
    uint8_t monitoring_interval_sec;    // Monitor interval (default: 30s)
} ChannelScanConfig;
```

### Performance Tuning
- **Quick Scan**: 1-second duration for basic congestion check
- **Standard Scan**: 3-second duration for comprehensive analysis
- **Detailed Scan**: 5-second duration with interference detection
- **Monitoring Mode**: Configurable intervals from 10 seconds to 10 minutes

## 📈 Optimization Strategies

### Best Practices
1. **Initial Assessment**: Run `spectrum` command for baseline analysis
2. **Regular Monitoring**: Enable continuous monitoring in busy environments
3. **Trend Analysis**: Export data periodically for long-term analysis
4. **Multi-Point Testing**: Test from different physical locations
5. **Time-Based Analysis**: Consider peak vs. off-peak usage patterns

### Channel Selection Guidelines
- **New Deployment**: Choose channels with <30% congestion
- **Existing Network**: Monitor performance and switch if congestion >70%
- **High-Density Areas**: Prefer channels 1, 6, 11 despite congestion
- **Rural Areas**: Consider non-standard channels if significantly clearer
- **Enterprise**: Coordinate channel assignments across multiple APs

## 🔍 Troubleshooting

### Common Issues

**"Channel analysis requires station mode"**
- Solution: Switch to station mode: `mode station`

**High congestion on all channels**
- Dense urban environment is normal
- Consider 5GHz band if available
- Use monitoring to find quieter time periods

**Inconsistent results between scans**
- Network conditions are dynamic
- Run multiple scans for average conditions
- Enable continuous monitoring for trends

**No interference detected despite issues**
- Some interference types are difficult to detect
- Consider external spectrum analyzer for validation
- Check for non-WiFi devices in 2.4GHz band

### Debugging Commands
```bash
# Check WiFi mode and status
ESP32> status

# Verify station mode is active
ESP32> mode station

# Quick congestion check
ESP32> congestion

# Detailed analysis with all features
ESP32> spectrum

# Monitor real-time changes
ESP32> channel monitor start
```

## 📊 Data Export and Integration

### JSON Export Format
The `channel export` command produces structured data suitable for:
- **External Analysis Tools**: Import into spreadsheets or databases
- **Network Management Systems**: Automated spectrum management
- **Historical Tracking**: Long-term congestion trend analysis
- **Report Generation**: Professional network optimization reports

### Sample JSON Structure
```json
{
  "timestamp": 1634567890000,
  "scan_duration_ms": 3240,
  "total_networks": 23,
  "overall_congestion": 64.2,
  "best_channel": 5,
  "worst_channel": 6,
  "interference_detected": false,
  "channels": [
    {
      "channel": 1,
      "network_count": 5,
      "congestion_score": 45.2,
      "strongest_rssi": -42,
      "average_rssi": -48,
      "overlapping_networks": 12,
      "is_recommended": true,
      "dominant_network": "MyHomeNetwork"
    }
    // ... additional channels
  ]
}
```

##  Advanced Features

### Interference Pattern Recognition
- **Microwave Interference**: Periodic signal disruption patterns
- **Bluetooth Coexistence**: Frequency hopping interference detection
- **Cordless Phones**: Fixed-frequency interference identification
- **Industrial Equipment**: Broadband noise source recognition

### Predictive Analysis
- **Trend Identification**: Congestion pattern analysis over time
- **Peak Usage Prediction**: Identify optimal deployment windows
- **Growth Planning**: Capacity planning based on congestion trends
- **Quality Correlation**: Link network performance to spectrum conditions

### Integration Ready
- **Network Management**: SNMP-compatible monitoring integration
- **Alert Systems**: Threshold-based congestion alerting
- **Automated Optimization**: Dynamic channel switching triggers
- **Performance Metrics**: KPI correlation with spectrum data

---

This comprehensive channel congestion analysis system transforms your ESP32 into a professional spectrum
analyzer, providing enterprise-grade WiFi optimization capabilities for network planning, troubleshooting,
and performance optimization.