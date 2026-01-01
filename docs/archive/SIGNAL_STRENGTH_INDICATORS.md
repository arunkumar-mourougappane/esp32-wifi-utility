# Signal Strength Indicators - Visual Guide

## 📊 Overview

The WiFi scan page now uses an intuitive **color-coded filled circle system** to display signal strength at a glance. This provides immediate visual feedback about network quality without requiring users to understand dBm values.

## 🎨 Signal Strength Ranges

### Excellent Signal (5 Circles - Green)
**RSSI: -50 dBm or better**
```
● ● ● ● ● -45 dBm
```
- **Color:** Green (#10b981)
- **Quality:** Excellent
- **Performance:** Maximum throughput, minimal latency
- **Range:** Very close to access point (typically < 10 feet)
- **Use Case:** Ideal for high-bandwidth applications, 4K streaming, gaming

### Very Good Signal (4 Circles - Green)
**RSSI: -50 to -59 dBm**
```
● ● ● ● ○ -55 dBm
```
- **Color:** Green (#10b981)
- **Quality:** Very Good
- **Performance:** High throughput, low latency
- **Range:** Close to access point (typically 10-30 feet)
- **Use Case:** HD streaming, video calls, file transfers

### Good Signal (3 Circles - Yellow)
**RSSI: -60 to -66 dBm**
```
● ● ● ○ ○ -65 dBm
```
- **Color:** Yellow (#fbbf24)
- **Quality:** Good
- **Performance:** Moderate throughput, acceptable latency
- **Range:** Medium distance (typically 30-50 feet)
- **Use Case:** Web browsing, SD streaming, general use

### Fair Signal (2 Circles - Orange)
**RSSI: -67 to -74 dBm**
```
● ● ○ ○ ○ -70 dBm
```
- **Color:** Orange (#fb923c)
- **Quality:** Fair
- **Performance:** Reduced throughput, increased latency
- **Range:** Far from access point (typically 50-80 feet)
- **Use Case:** Basic web browsing, email, light use
- **Note:** May experience occasional drops or slowdowns

### Poor Signal (1 Circle - Red)
**RSSI: -75 dBm or worse**
```
● ○ ○ ○ ○ -80 dBm
```
- **Color:** Red (#ef4444)
- **Quality:** Poor
- **Performance:** Low throughput, high latency, unstable
- **Range:** Very far or obstructed (typically > 80 feet)
- **Use Case:** Minimal connectivity, may be unreliable
- **Note:** Not recommended for sustained use

## 🔧 Technical Implementation

### SVG Circle System

**Filled Circle (Active):**
```html
<svg width="14" height="14">
    <circle cx="7" cy="7" r="6" fill="{color}"/>
</svg>
```

**Empty Circle (Inactive):**
```html
<svg width="14" height="14">
    <circle cx="7" cy="7" r="6" fill="none" stroke="#d1d5db" stroke-width="2"/>
</svg>
```

### Color Palette

| Quality | Hex Color | RGB | Description |
|---------|-----------|-----|-------------|
| Excellent/Very Good | `#10b981` | rgb(16, 185, 129) | Emerald Green |
| Good | `#fbbf24` | rgb(251, 191, 36) | Amber Yellow |
| Fair | `#fb923c` | rgb(251, 146, 60) | Orange |
| Poor | `#ef4444` | rgb(239, 68, 68) | Red |
| Empty | `#d1d5db` | rgb(209, 213, 219) | Gray |

### RSSI Thresholds

```cpp
if (rssi >= -50)      { bars = 5; color = GREEN; }    // Excellent
else if (rssi >= -60) { bars = 4; color = GREEN; }    // Very Good
else if (rssi >= -67) { bars = 3; color = YELLOW; }   // Good
else if (rssi >= -75) { bars = 2; color = ORANGE; }   // Fair
else                  { bars = 1; color = RED; }      // Poor
```

## 📱 User Experience

### Visual Design
- **Circle Size:** 14×14 pixels (optimized for mobile and desktop)
- **Spacing:** 3px gap between circles
- **Layout:** Horizontal flex layout with gap
- **Alignment:** Centered vertically
- **Tooltip:** Shows exact dBm value on hover

### Example Display
```
Network A    ● ● ● ● ● -45 dBm     (Excellent - Green)
Network B    ● ● ● ● ○ -55 dBm     (Very Good - Green)
Network C    ● ● ● ○ ○ -63 dBm     (Good - Yellow)
Network D    ● ● ○ ○ ○ -72 dBm     (Fair - Orange)
Network E    ● ○ ○ ○ ○ -82 dBm     (Poor - Red)
```

### Information Hierarchy
1. **Primary:** Visual circles (instant recognition)
2. **Secondary:** dBm value (technical reference)
3. **Tertiary:** Tooltip on hover (accessibility)

## 🎯 Benefits

### Instant Recognition
- **Color Coding:** Universal traffic light system (green = good, red = bad)
- **Quantity:** More circles = better signal (intuitive)
- **No Math Required:** Users don't need to understand dBm values

### Accessibility
- **Color Blind Friendly:** Quantity of circles provides redundant information
- **Contrast:** High contrast circles and text
- **Tooltips:** Exact values available on hover for technical users

### Professional Appearance
- **Clean SVG Graphics:** Crisp at any resolution
- **Modern Design:** Matches current UI trends
- **Responsive:** Works on mobile and desktop

## 📊 RSSI Reference Guide

### Understanding dBm Values

| RSSI Range | Signal Quality | Typical Speed | Connection Stability |
|------------|----------------|---------------|----------------------|
| -30 dBm | Amazing | Maximum | Rock solid |
| -40 dBm | Excellent | Maximum | Very stable |
| -50 dBm | Excellent | High | Stable |
| -60 dBm | Very Good | High | Stable |
| -67 dBm | Good | Medium | Mostly stable |
| -70 dBm | Fair | Medium-Low | Occasional issues |
| -75 dBm | Fair-Poor | Low | Frequent issues |
| -80 dBm | Poor | Very Low | Very unstable |
| -85 dBm | Very Poor | Minimal | Barely usable |
| -90 dBm | Unusable | None | Disconnects |

### Factors Affecting Signal Strength

**Physical Distance:**
- Inverse square law: doubling distance reduces signal by ~6 dBm
- Every 20 feet typically reduces signal by 3-5 dBm

**Obstacles:**
- Wood walls: -3 to -5 dBm loss
- Drywall: -5 to -7 dBm loss
- Brick walls: -10 to -15 dBm loss
- Concrete: -15 to -20 dBm loss
- Metal: -20+ dBm loss (nearly complete blockage)

**Interference:**
- Other WiFi networks on same channel
- Bluetooth devices
- Microwave ovens
- Cordless phones
- USB 3.0 devices (near 2.4GHz)

##  Usage Tips

### For Network Selection
1. **Prioritize Green (5-4 circles):** Best performance
2. **Consider Yellow (3 circles):** Acceptable for most uses
3. **Avoid Red (1 circle):** Unreliable, slow performance

### For Troubleshooting
- **Compare Multiple Scans:** Signal fluctuates, take average
- **Check Channel Congestion:** Good RSSI + slow speed = interference
- **Reposition:** Move closer or remove obstacles
- **Switch Bands:** Consider 5GHz if available (less interference)

### For Optimal Placement
- **Target -50 to -60 dBm:** Sweet spot for AP placement
- **Minimum -67 dBm:** For coverage edge areas
- **Overlap Coverage:** Ensure handoff zones have -60 dBm or better

## 🔍 Code Reference

### Location
- **File:** `src/web_server.cpp`
- **Function:** `handleScan()`
- **Lines:** Network list generation loop

### Key Variables
```cpp
int rssi = WiFi.RSSI(i);     // Get signal strength
int bars = 0;                 // Number of filled circles (1-5)
String color;                 // Circle color (hex)
```

### Customization
To adjust thresholds, modify the if-else chain:
```cpp
if (rssi >= -50)      { bars = 5; color = "#10b981"; }  // Your excellent threshold
else if (rssi >= -60) { bars = 4; color = "#10b981"; }  // Your very good threshold
// ... etc
```

## 📈 Future Enhancements

### Potential Additions
- **5GHz Signal Strength:** Different thresholds for 5GHz band
- **Signal Trend:** Arrow showing if signal is improving/degrading
- **Historical Data:** Mini-graph of signal over time
- **Signal Quality Score:** Combined RSSI + noise + interference metric
- **Best Network Recommendation:** Auto-highlight optimal network
- **Connection Prediction:** Estimate speed based on RSSI

### Advanced Features
- **Real-Time Updates:** Auto-refresh signal strength
- **Signal Heatmap:** Visual map of coverage area
- **A/B Comparison:** Compare two network signals side-by-side
- **Alert Threshold:** Notify when signal drops below threshold

## ✅ Testing Checklist

- [x] Circles render correctly in all browsers
- [x] Colors match signal strength ranges
- [x] dBm values display accurately
- [x] Tooltips show on hover
- [x] Responsive on mobile devices
- [x] Empty circles have proper outline
- [x] Spacing is consistent
- [x] Build succeeds without errors
- [x] Memory impact acceptable (+664 bytes)

## 🎊 Summary

The new signal strength indicator system provides:
- ✅ **Intuitive visual feedback** with traffic light colors
- ✅ **5-level granularity** for precise signal assessment
- ✅ **Universal understanding** - no technical knowledge required
- ✅ **Professional appearance** with clean SVG graphics
- ✅ **Accessibility** with redundant information encoding
- ✅ **Technical reference** with exact dBm values
- ✅ **Minimal memory impact** (+664 bytes flash)

---

**Version:** 2.1.0+signal-strength-indicators  
**Date:** October 15, 2025  
**Flash Impact:** +664 bytes (+0.04%)  
**Feature:** Color-coded filled circle signal strength display
