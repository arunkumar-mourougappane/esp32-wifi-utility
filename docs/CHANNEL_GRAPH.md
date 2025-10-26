# Channel Graph Visualization - WiFi Spectrum Analysis

## Overview

The Channel Graph Visualization provides a professional, interactive visual representation of the 2.4GHz WiFi spectrum. Using HTML5 Canvas technology, it displays network congestion across all 14 WiFi channels, helping users identify the optimal channel for their wireless network and understand spectrum interference.

## Features

### Visual Spectrum Display

The Channel Graph presents WiFi spectrum data in an intuitive, color-coded bar chart format:

#### **Graph Components**

1. **Bar Chart Visualization**
   - 14 vertical bars representing channels 1-14
   - Bar height indicates congestion level (0-100%)
   - Color-coded for quick assessment
   - Network count overlays on bars

2. **Color-Coded Congestion Levels**
   - 🟢 **Green (0-40%)**: Low congestion - Excellent choice
   - 🟡 **Yellow (40-70%)**: Medium congestion - Acceptable
   - 🔴 **Red (70-100%)**: High congestion - Avoid

3. **Network Count Indicators**
   - Displays number of networks per channel
   - Shown as text overlay on bars
   - Format: "X net" or "X nets"
   - White text for visibility

4. **Recommended Channel Markers**
   - Gold star (⭐) above optimal channels
   - Indicates lowest interference
   - Based on congestion analysis
   - Multiple recommendations possible

5. **Professional Graph Elements**
   - Grid lines for easy reading
   - Labeled X-axis (Channel 1-14)
   - Labeled Y-axis (0-100% congestion)
   - Axis titles and descriptions
   - Interactive legend

### Interactive Features

#### **Responsive Design**
- Canvas size: 1000x400px (scales to container)
- Adapts to screen width automatically
- Window resize support with redraw
- Touch-friendly on mobile devices

#### **Legend Display**
Located in top-right corner:
- **Congestion Level**: Title
- 🟢 Low (0-40%)
- 🟡 Medium (40-70%)
- 🔴 High (70-100%)

#### **Educational Content**
Below the graph, detailed explanations:
- Bar height meaning
- Color significance
- Network count interpretation
- Star marker purpose
- Non-overlapping channel info (1, 6, 11)
- Best practices for channel selection

## Web Interface

### Accessing the Channel Graph

**Primary Access:**
- Direct URL: `http://<device-ip>/channel/graph`
- From Channel Analysis: Click "📊 View Channel Graph" button
- From Analysis Dashboard: Navigate through Channel section

### Interface Layout

#### **1. Header Section**
```
┌─────────────────────────────────────┐
│ 📊 Channel Graph                    │
│ Visual WiFi Channel Spectrum &      │
│ Signal Strength                      │
└─────────────────────────────────────┘
```

#### **2. Scan Button**
- Purple gradient "🔄 Scan Channels" button
- Initiates new spectrum scan
- Redirects to Channel Analysis page
- Updates graph data

#### **3. Channel Spectrum Visualization**
```
    Congestion / Signal Strength
        ▲
    100%│                Legend:
     90%│                ─────────
     80%│                🟢 Low (0-40%)
     70%│    [Bar]       🟡 Medium (40-70%)
     60%│    [Bar]       🔴 High (70-100%)
     50%│    [Bar]
     40%│  ⭐[Bar]
     30%│    [Bar]
     20%│  ⭐[Bar]
     10%│    [Bar]⭐
      0%└─────────────────────────►
         Ch1 Ch2 Ch3...Ch14
              WiFi Channel
```

#### **4. Channel Recommendations Section**
Display boxes showing:
- **Best Channel**: Automatically detected
- **Least Crowded**: With available capacity percentage
- **Total Networks Found**: Count of all networks
- **Recommended Non-Overlapping**: Channels 1, 6, 11

#### **5. Understanding the Graph Section**
Educational content explaining:
- How to read the graph
- What colors mean
- Importance of non-overlapping channels
- Best practices for channel selection

### Example Graph Display

**Typical Urban Environment:**
```
Channel 1:  ████████░░ 80% (5 nets) 🔴 High Congestion
Channel 2:  ████████░░ 85% (6 nets) 🔴
Channel 3:  ██████░░░░ 65% (4 nets) 🟡
Channel 4:  ████░░░░░░ 45% (3 nets) 🟡
Channel 5:  ███░░░░░░░ 35% (2 nets) 🟢
Channel 6:  █████████░ 95% (8 nets) 🔴 High Congestion
Channel 7:  ███░░░░░░░ 30% (2 nets) 🟢 ⭐ Recommended
Channel 8:  ██░░░░░░░░ 25% (1 net)  🟢 ⭐ Recommended
Channel 9:  ██░░░░░░░░ 20% (1 net)  🟢 ⭐ Best
Channel 10: ███░░░░░░░ 35% (2 nets) 🟢
Channel 11: ████████░░ 75% (6 nets) 🔴 High Congestion
Channel 12: ████░░░░░░ 40% (3 nets) 🟡
Channel 13: ███░░░░░░░ 30% (2 nets) 🟢
Channel 14: █░░░░░░░░░ 10% (0 nets) 🟢 (Not used in most regions)
```

**Recommendation**: Use Channel 9 (Best) or Channel 8/7 (Recommended)

## Channel Analysis

### Understanding WiFi Channels

#### **2.4GHz Channel Overview**

The 2.4GHz WiFi band consists of 14 channels (13 in most regions):

| Channel | Frequency (MHz) | Width (MHz) |
|---------|----------------|-------------|
| 1 | 2412 | 22 |
| 2 | 2417 | 22 |
| 3 | 2422 | 22 |
| 4 | 2427 | 22 |
| 5 | 2432 | 22 |
| 6 | 2437 | 22 |
| 7 | 2442 | 22 |
| 8 | 2447 | 22 |
| 9 | 2452 | 22 |
| 10 | 2457 | 22 |
| 11 | 2462 | 22 |
| 12 | 2467 | 22 |
| 13 | 2472 | 22 |
| 14 | 2484 | 22 |

#### **Channel Overlap Problem**

Each WiFi channel occupies 22MHz bandwidth in a ~100MHz total spectrum:

```
        2.4GHz WiFi Spectrum (2400-2500 MHz)
┌────────────────────────────────────────────────┐
│ Ch1 ███████████████████                        │
│ Ch2   ███████████████████                      │
│ Ch3     ███████████████████                    │
│ Ch4       ███████████████████                  │
│ Ch5         ███████████████████                │
│ Ch6           ███████████████████              │ ← Non-overlapping
│ Ch7             ███████████████████            │
│ Ch8               ███████████████████          │
│ Ch9                 ███████████████████        │
│ Ch10                  ███████████████████      │
│ Ch11                    ███████████████████    │ ← Non-overlapping
│ Ch12                      ███████████████████  │
│ Ch13                        ███████████████████│
└────────────────────────────────────────────────┘
```

**Key Insight**: Channels 1, 6, and 11 don't overlap!

### Non-Overlapping Channels

#### **Why 1, 6, 11 are Special**

These three channels are spaced far enough apart (25MHz) to avoid interference:

```
Channel 1:  2401-2423 MHz  ┃
                            ┃ 25 MHz gap
Channel 6:  2426-2448 MHz  ┃
                            ┃ 25 MHz gap
Channel 11: 2451-2473 MHz  ┃
```

**Best Practice**: Always use channels 1, 6, or 11 for optimal performance

#### **Why Other Channels Exist**

Channels 2-5, 7-10, 12-13:
- **Allowed by regulations** in many countries
- **Useful for specific scenarios**:
  - Point-to-point links
  - Isolated environments
  - Temporary networks
- **Generally not recommended** due to overlap

### Congestion Scoring

#### **How Congestion is Calculated**

The graph uses multi-factor analysis:

1. **Network Count** (40% weight)
   - More networks = higher congestion
   - Each network adds to channel load

2. **Signal Strength** (30% weight)
   - Stronger signals = more interference
   - RSSI values weighted by strength

3. **Channel Overlap** (30% weight)
   - Adjacent channel interference
   - Overlapping channel penalty

**Formula Example**:
```
Base Score = (Networks × 10) + (Avg RSSI × 0.5) + (Overlap × 15)
Congestion % = min(100, Base Score)
```

#### **Congestion Level Guidelines**

| Level | Range | Assessment | Action |
|-------|-------|------------|--------|
| Low | 0-40% | Excellent | ✅ Use this channel |
| Medium | 40-70% | Acceptable | ⚠️ Monitor performance |
| High | 70-100% | Problematic | ❌ Avoid this channel |

## Usage Examples

### Scenario 1: New Router Setup

**Goal**: Find optimal channel for new wireless access point

**Steps:**
1. Navigate to `/channel/graph`
2. Click "🔄 Scan Channels" if data not current
3. Wait for scan completion
4. View the channel graph
5. Identify channels with:
   - ✅ Green or yellow bars (low-medium congestion)
   - ✅ Star markers (⭐)
   - ✅ Low network count
   - ✅ Channels 1, 6, or 11 preferred

**Example Result:**
- **Channel 1**: 80% congestion (5 networks) - ❌ Avoid
- **Channel 6**: 95% congestion (8 networks) - ❌ Avoid
- **Channel 11**: 30% congestion (2 networks) - ✅ **BEST CHOICE**

**Action**: Configure router to use Channel 11

### Scenario 2: Troubleshooting Poor WiFi

**Goal**: Identify if channel congestion is causing problems

**Steps:**
1. Note current router channel (e.g., Channel 6)
2. Navigate to `/channel/graph`
3. Scan if needed
4. Check current channel's congestion level

**Analysis:**
```
Current Channel 6:
┌────────────────────────────┐
│ Congestion: 95% 🔴         │
│ Networks: 8                 │
│ Status: HIGH CONGESTION     │
│ Recommendation: CHANGE      │
└────────────────────────────┘

Alternative Channel 9:
┌────────────────────────────┐
│ Congestion: 20% 🟢         │
│ Networks: 1                 │
│ Status: LOW CONGESTION      │
│ Recommendation: OPTIMAL ⭐  │
└────────────────────────────┘
```

**Action**: Change router from Channel 6 to Channel 9

### Scenario 3: Multi-AP Deployment

**Goal**: Configure multiple access points without interference

**Steps:**
1. Scan WiFi spectrum with `/channel/graph`
2. Identify congestion on channels 1, 6, and 11
3. Distribute APs across least-congested non-overlapping channels

**Example Deployment:**
```
Building Layout:
┌──────────────────────────┐
│  AP1 (Ch 1) ◄──┐         │ Floor 1
│                 │ 25MHz   │
├─────────────────┼─────────┤
│  AP2 (Ch 6) ◄──┤         │ Floor 2
│                 │ 25MHz   │
├─────────────────┼─────────┤
│  AP3 (Ch 11) ◄─┘         │ Floor 3
└──────────────────────────┘
```

**Result**: No interference between APs, optimal coverage

### Scenario 4: Dense Urban Environment

**Goal**: Find any usable channel in crowded apartment building

**Steps:**
1. Run channel scan
2. View graph showing all channels congested
3. Identify least-congested option
4. Consider 5GHz as alternative

**Typical Result:**
```
All channels 40-100% congested
Best available: Channel 8 (55% - Medium)
Alternative: Switch to 5GHz band
```

**Recommendation**: Use 5GHz if device supports it, otherwise Channel 8

## Technical Implementation

### Canvas Rendering

#### **Graph Dimensions**
```javascript
Canvas Size: 1000 × 400 pixels
Padding: 60px (all sides)
Graph Area: 880 × 280 pixels
Channel Width: 62.86px (880/14)
Bar Width: 52.86px (with 10px gap)
```

#### **Drawing Process**

1. **Clear Canvas**
   ```javascript
   ctx.clearRect(0, 0, width, height);
   ```

2. **Draw Grid**
   - Horizontal lines (10 divisions)
   - Vertical lines (14 channels)
   - Light gray color (#e5e7eb)

3. **Draw Axes**
   - X-axis (bottom)
   - Y-axis (right side)
   - Dark color (#333)
   - 2px width

4. **Draw Labels**
   - Y-axis: 0%, 10%, 20%...100%
   - X-axis: Ch 1, Ch 2...Ch 14
   - Axis titles (rotated for Y-axis)

5. **Draw Channel Bars**
   ```javascript
   for each channel:
     - Calculate bar height from congestion %
     - Determine color (green/yellow/red)
     - Draw filled rectangle
     - Draw border outline
     - Add network count text
     - Add star if recommended
   ```

6. **Draw Legend**
   - Position: top-right
   - Color swatches with labels
   - Congestion level descriptions

#### **Color Scheme**

```javascript
// Congestion Colors
Low:    #10b981 (Green)
Medium: #fbbf24 (Yellow)
High:   #ef4444 (Red)

// UI Elements
Grid:   #e5e7eb (Light Gray)
Axes:   #333333 (Dark Gray)
Text:   #666666 (Medium Gray)
Star:   #fbbf24 (Gold)
```

### Data Integration

Channel data comes from `lastChannelAnalysis` structure:

```cpp
struct ChannelCongestionData {
    uint8_t channel;           // Channel number (1-14)
    uint8_t network_count;     // Networks on channel
    float congestion_score;    // 0-100%
    bool is_recommended;       // Star marker flag
};
```

JavaScript data array:
```javascript
const channelData = [
  {ch: 1, nets: 5, cong: 80.0, rec: false},
  {ch: 6, nets: 8, cong: 95.0, rec: false},
  {ch: 11, nets: 2, cong: 30.0, rec: true},
  // ... channels 2-10, 12-14
];
```

## Integration

### Channel Analysis Page

The Channel Graph complements the Channel Analysis page:

**Channel Analysis** (`/channel`):
- Tabular data display
- Detailed channel metrics
- Scan controls
- Recommendations

**Channel Graph** (`/channel/graph`):
- Visual spectrum view
- Interactive bar chart
- Comparative analysis
- Best channel identification

**Workflow:**
1. Run scan from Channel Analysis
2. View detailed data in table
3. Click "View Channel Graph" button
4. Analyze visual spectrum
5. Identify optimal channel
6. Configure router

### Analysis Dashboard

Accessible from unified dashboard:

**Dashboard** (`/analysis`):
- Channel Analysis card
- Quick scan button
- Status display
- Link to both Analysis and Graph

## Best Practices

### Channel Selection Guidelines

#### **Priority Order**

1. **First Choice**: Non-overlapping channels (1, 6, 11)
2. **Second Choice**: Low congestion on any channel
3. **Third Choice**: Least congested overlapping channel
4. **Last Resort**: 5GHz band migration

#### **Decision Matrix**

| Scenario | Recommended Channel |
|----------|-------------------|
| All non-overlapping low | Use Channel 1, 6, or 11 |
| Channel 1/11 low, 6 high | Use Channel 1 or 11 |
| All non-overlapping high | Use Channel 8 or 9 |
| All channels high | Consider 5GHz |

### Scanning Frequency

**Recommended Schedule:**
- **New Installation**: Scan before deployment
- **Performance Issues**: Scan immediately
- **Routine Monitoring**: Monthly
- **Environment Changes**: Scan after changes
- **Dense Areas**: Weekly scans

### Performance Optimization

**Tips for Best Results:**
- Avoid channels with >70% congestion
- Minimize adjacent channel interference
- Consider physical obstacles
- Account for non-WiFi interference (Bluetooth, microwaves)
- Test actual performance after channel change

## Troubleshooting

### Graph Not Displaying

**Symptoms:**
- Blank white area where graph should be
- "No channel data available" message

**Solutions:**
1. Click "🔄 Scan Channels" button
2. Wait for scan completion
3. Return to `/channel/graph`
4. Refresh page if needed

### Inaccurate Results

**Symptoms:**
- Congestion levels seem wrong
- Missing networks
- Incorrect channel data

**Solutions:**
1. Run new scan (data may be stale)
2. Move device for better reception
3. Ensure device antenna is functional
4. Check for WiFi connectivity

### Graph Performance Issues

**Symptoms:**
- Slow rendering
- Laggy interactions
- High CPU usage

**Solutions:**
1. Close other browser tabs
2. Use modern browser (Chrome, Firefox, Safari)
3. Refresh page to clear memory
4. Check device performance

## Educational Resources

### Understanding WiFi Spectrum

**Key Concepts:**
- **Frequency**: 2.4GHz = 2400-2500 MHz range
- **Channel Width**: 20 MHz (22 MHz including guard bands)
- **Channel Spacing**: 5 MHz between adjacent channels
- **Overlap**: Adjacent channels interfere with each other
- **Non-Overlapping**: Channels separated by ≥25 MHz

### Interference Sources

**Common 2.4GHz Interferers:**
- Microwave ovens (2450 MHz)
- Bluetooth devices (2400-2483.5 MHz)
- Cordless phones (2.4GHz models)
- Baby monitors
- Wireless cameras
- Gaming controllers

**Mitigation:**
- Use 5GHz band when possible
- Position router away from interference sources
- Choose channels away from interference peaks

## Conclusion

The Channel Graph Visualization provides essential insights into WiFi spectrum usage, enabling informed decisions about channel selection and network optimization. Regular use of this tool helps maintain optimal wireless network performance in all environments.

For more information, see:
- [Channel Analysis Documentation](CHANNEL_ANALYSIS.md)
- [Signal Strength Monitor](SIGNAL_STRENGTH_MONITOR.md)
- [Analysis Dashboard Guide](ANALYSIS_DASHBOARD.md)
