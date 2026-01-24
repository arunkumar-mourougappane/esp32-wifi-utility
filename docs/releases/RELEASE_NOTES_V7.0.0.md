# Release Notes - Version 7.0.0

**Release Date**: January 24, 2026  
**Type**: Major Release - Mobile Web Interface & TFT Display Enhancements

## 🎉 Overview

Version 7.0.0 is a **major release** focusing on **mobile web interface enhancements** and **TFT display optimizations**. This release introduces a professional animated hamburger menu with smooth X toggle effect, current page indicator in mobile navigation, improved TFT rendering performance, and RTC time persistence across reboots.

---

## 📱 Major Features

### 🍔 Animated Hamburger Menu

Professional CSS animation for mobile navigation toggle:

#### Visual Design

**Hamburger Icon (☰):**
- 3 horizontal bars (25px × 3px each)
- 5px spacing between bars
- Dark gray color (#212529)
- Light background (#f8f9fa)
- Rounded corners (border-radius: 0.25rem)
- Absolute positioning (right: 1rem)

**Toggle Animation (☰ → ✕):**

When the hamburger is clicked/tapped, it transforms into an X:

```css
/* First bar - rotates and moves down */
.hamburger.active span:nth-child(1) {
    transform: rotate(45deg) translate(5px, 5px);
}

/* Middle bar - fades out */
.hamburger.active span:nth-child(2) {
    opacity: 0;
}

/* Third bar - rotates and moves up */
.hamburger.active span:nth-child(3) {
    transform: rotate(-45deg) translate(7px, -6px);
}
```

**Animation Properties:**
- **Duration**: 0.3s smooth transitions
- **Easing**: Default ease function
- **Applied to**: all transform properties

#### User Experience Benefits

✅ **Clear Visual Feedback** - Users immediately understand menu state  
✅ **Professional Feel** - Smooth animations improve perceived quality  
✅ **Standard Pattern** - Follows mobile UI best practices  
✅ **Touch-Friendly** - Large tap target (44px minimum)

---

### 📍 Current Page Indicator

Mobile navigation now displays which page you're currently viewing:

#### Visual Design

**Location**: Between content and hamburger icon in navigation bar

**Typography:**
- Font size: 1.125rem (18px)
- Font weight: 500 (medium)
- Color: #212529 (dark gray)
- Text alignment: center

**Layout:**
- Flexbox flex: 1 (takes available space)
- Padding: 0 1rem (left and right spacing)

#### Display Behavior

| Screen Width | Display State | Purpose |
|--------------|---------------|---------|
| > 768px (Desktop) | Hidden | Desktop has full menu visible |
| ≤ 768px (Mobile) | Visible | Helps users know current location |

#### Page Title Mapping

Every page handler passes its title to the navigation:

| Route | Page Title | Icon |
|-------|------------|------|
| `/` | Home | 🏠 |
| `/status` | Status | 📊 |
| `/config` | Config | ⚙️ |
| `/scan` | Network Scan | 🔍 |
| `/analysis` | Analysis | 🔬 |
| `/channel` | Channel | 📡 |
| `/signal` | Signal | 📶 |
| `/iperf` | iPerf | ⚡ |
| `/latency` | Latency | 📉 |
| `/portscan` | Port Scanner | 🔒 |

#### Implementation

```javascript
// Dynamic title injection on page load
String generateNav(const String& pageTitle) {
    String nav = FPSTR(NAV_MENU_START);
    if (pageTitle.length() > 0) {
        nav += F("<script>document.addEventListener('DOMContentLoaded',function(){");
        nav += F("const t=document.getElementById('pageTitle');");
        nav += F("if(t)t.textContent='");
        nav += pageTitle;
        nav += F("'})</script>");
    }
    return nav;
}
```

#### User Experience Benefits

✅ **Better Orientation** - Always know which page you're on  
✅ **Reduced Confusion** - No need to open menu to check location  
✅ **Cleaner Interface** - Title disappears when menu is open  
✅ **Consistent Experience** - Works across all pages

---

### 📐 Full-Width Navigation Layout

Professional responsive navigation system:

#### Desktop Layout (> 768px)

```
+--------------------------------------------------------+
|  [🏠 Home] [📊 Status] [⚙️ Config] [🔬 Analysis ▾]      |
|          (Centered, max-width: 1200px)                 |
+--------------------------------------------------------+
|                                                        |
|              [Page Content]                            |
|          (Centered, max-width: 1200px)                 |
+--------------------------------------------------------+
```

#### Mobile Layout (≤ 768px)

**Menu Closed:**
```
+----------------------------------------+
|  📊 Status                      [☰]    |
+----------------------------------------+
|                                        |
|         [Page Content]                 |
|                                        |
+----------------------------------------+
```

**Menu Open:**
```
+----------------------------------------+
|  📊 Status                      [✕]    |
+----------------------------------------+
|  🏠 Home                               |
|  📊 Status                             |
|  ⚙️ Config                              |
|  🔬 Analysis ▾                         |
+----------------------------------------+
|  (Menu overlays content)               |
+----------------------------------------+
```

#### CSS Architecture

**Z-Index Hierarchy:**
- Page content: z-index: 1
- Navigation bar: z-index: 1000
- Mobile menu items: z-index: 1020
- Hamburger icon: z-index: 1030
- Dropdown menus: z-index: 1100

**Key CSS Classes:**

```css
/* Navigation container */
.nav {
    width: 100%;
    position: relative;
    border-bottom: 1px solid #dee2e6;
    z-index: 1000;
}

/* Centered content wrapper */
.nav-container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 0 1rem;
    display: flex;
    justify-content: center;
    align-items: center;
    position: relative;
}

/* Page title (mobile only) */
.page-title {
    display: none;
    font-size: 1.125rem;
    font-weight: 500;
    color: #212529;
}

@media (max-width: 768px) {
    .page-title {
        display: block;
        flex: 1;
        text-align: center;
        padding: 0 1rem;
    }
}
```

#### User Experience Benefits

✅ **Modern Design** - Full-width nav is current best practice  
✅ **Better Hierarchy** - Clear separation between nav and content  
✅ **No Overlay Issues** - Proper z-index prevents content from covering menu  
✅ **Consistent Width** - Content always centered at same width

---

## 🖥️ TFT Display Optimizations

### ⚡ Performance Improvements

**Optimized GFX Library Calls:**
- Reduced unnecessary screen updates
- Batch similar drawing operations
- Minimize display buffer writes
- Improved frame rate and responsiveness

**Battery Monitor Efficiency:**
- Optimized ADC reading frequency
- Reduced polling overhead
- Better power management
- Extended battery life

**Measurable Improvements:**
- Faster screen refresh rates
- Smoother animations
- Reduced power consumption
- Better user experience

### 🕐 RTC Time Persistence

**Problem Solved:**  
Previously, the device time would reset to 00:00 on every reboot.

**Solution:**  
RTC (Real-Time Clock) time now persists across device reboots.

**Benefits:**
- Consistent time display
- Reliable timestamps
- Better logging accuracy
- Improved user experience

**Implementation:**
- Time stored in non-volatile memory
- Restored on boot
- Maintains accuracy across power cycles

### 🔒 WiFi Encryption Display

**Enhanced TFT Information:**
- Display shows WiFi encryption types
- Differentiated display for various security protocols
- Visual indicators for:
  - Open networks (🔓)
  - WPA2 networks (🔒)
  - WPA3 networks (🔐)
  - Enterprise networks (🏢)

**User Experience:**
- Better security awareness
- Quick identification of network types
- Informed connection decisions

---

## 📚 Documentation Updates

### Updated Documentation Files

**README.md:**
- Updated latest updates section for v7.0.0
- Mobile navigation feature highlights
- Version badge updated to 7.0.0

**WEB_INTERFACE.md:**
- Comprehensive mobile navigation section
- Hamburger animation CSS specifications
- Current page title display behavior
- Touch-friendly interaction patterns
- Z-index stacking documentation

**FEATURES.md:**
- Enhanced web interface section
- Mobile navigation enhancements
- Professional CSS animations details
- Current page indicator feature

**RESPONSIVE_MENU.md:**
- Page title display specifications
- Hamburger animation CSS implementation
- Mobile navigation layout diagrams
- Updated CSS classes reference
- Z-index values documentation

### New Documentation Content

**Mobile Navigation Diagrams:**
- ASCII art layouts showing menu states
- Before/after animation states
- Z-index hierarchy visualization

**CSS Implementation Examples:**
- Complete code snippets
- Commented explanations
- Best practices

**Page Title Mapping Table:**
- All routes documented
- Consistent icon usage
- Implementation examples

---

## 🔧 Technical Details

### Build Information

**Compilation Status:**
- ✅ ESP32dev: Success
- ✅ Feather ESP32-S3 TFT: Success
- ✅ Zero warnings
- ✅ All tests passing

**Memory Usage:**
- Flash: 44.0% (1,385,104 / 3,145,728 bytes)
- RAM: 16.4% (53,612 / 327,680 bytes)
- Free Heap: ~274KB
- PSRAM: Available on Feather boards

### Code Statistics

**Files Modified:**
- `lib/WebServer/web_server.cpp` (230 insertions, 61 deletions)
- `lib/TFTDisplay/tft_display.cpp` (optimizations)
- `lib/TFTDisplay/tft_display.h` (interface updates)
- `lib/WiFiManager/wifi_manager.cpp` (minor updates)

**Documentation Modified:**
- `README.md` (updated)
- `CHANGELOG.md` (new v7.0.0 entry)
- `docs/releases/VERSION_HISTORY.md` (updated)
- `docs/releases/RELEASE_NOTES_V7.0.0.md` (this file)
- `docs/guides/WEB_INTERFACE.md` (enhanced)
- `docs/features/FEATURES.md` (enhanced)
- `docs/features/RESPONSIVE_MENU.md` (enhanced)

**Total Changes:**
- 388 insertions
- 83 deletions
- Net gain: 305 lines

### Browser Compatibility

**Tested Browsers:**
- ✅ Chrome/Chromium (Desktop & Mobile)
- ✅ Firefox (Desktop & Mobile)
- ✅ Safari (Desktop & iOS)
- ✅ Edge (Desktop)

**Mobile Devices Tested:**
- ✅ iOS (iPhone/iPad)
- ✅ Android (various devices)
- ✅ Responsive breakpoints (320px - 1920px)

---

## 🎯 Key Benefits

### For Mobile Users

1. **Better Navigation** - Know which page you're on at all times
2. **Professional Feel** - Smooth animations improve perceived quality
3. **Easy Access** - Hamburger menu works consistently across pages
4. **Clear Feedback** - X icon clearly indicates menu can be closed

### For All Users

5. **Improved Performance** - TFT display renders faster
6. **Better Reliability** - Time persists across reboots
7. **Enhanced Security** - See WiFi encryption types clearly
8. **Complete Documentation** - All features well-documented

### For Developers

9. **Cleaner Code** - Consolidated navigation generation
10. **Better Architecture** - Proper z-index hierarchy
11. **Easy Extension** - Add new pages easily with title parameter
12. **Well Documented** - Implementation details clearly explained

---

## 📦 Installation

### Pre-Built Firmware

Download from [GitHub Releases](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/releases/v7.0.0):

- `esp32-wifi-utility-esp32dev-v7.0.0.bin`
- `esp32-wifi-utility-feather-s3-tft-v7.0.0.bin`
- `esp32-wifi-utility-feather-s3-reversetft-v7.0.0.bin`

### Build From Source

```bash
# Clone repository
git clone https://github.com/arunkumar-mourougappane/esp32-wifi-utility.git
cd esp32-wifi-utility

# Checkout v7.0.0
git checkout v7.0.0

# Build for your board
pio run -e esp32dev -t upload
# or
pio run -e adafruit_feather_esp32s3_tft -t upload
```

---

## 🔄 Upgrading from v6.0.0

### What's Changed

- Web interface navigation has improved mobile UX
- TFT display performs better
- No breaking changes to commands or API
- All existing configurations preserved

### Steps to Upgrade

1. **Backup Configuration** (optional):
   ```
   config export
   ```

2. **Flash New Firmware**:
   ```bash
   pio run -e <your-board> -t upload
   ```

3. **Verify Operation**:
   - Check web interface on mobile device
   - Test hamburger menu animation
   - Verify page title displays correctly

### No Action Required

- ✅ NVS configuration automatically preserved
- ✅ WiFi credentials remain intact
- ✅ All saved settings maintained

---

## 🐛 Known Issues

None reported at this time.

---

## 🔮 Future Enhancements

Potential features for upcoming releases:

- Dark mode toggle for web interface
- Network favorites/bookmarks
- Advanced channel analysis graphs
- Historical signal strength charts
- Notification system for events

---

## 🤝 Contributing

We welcome contributions! To contribute:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

---

## 📞 Support

- **Documentation**: [docs/](../../docs/)
- **Issues**: [GitHub Issues](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/issues)
- **Discussions**: [GitHub Discussions](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/discussions)
- **Pull Requests**: [#47](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/pull/47)

---

## 📄 License

This project is licensed under the MIT License. See [LICENSE](../../LICENSE) for details.

---

**Thank you for using ESP32 WiFi Utility!** 🎉

*Maintained by [Arunkumar Mourougappane](https://github.com/arunkumar-mourougappane)*
