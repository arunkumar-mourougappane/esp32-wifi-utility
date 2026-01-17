# ESP32 WiFi Utility - Release Notes v5.2.1

**Release Date:** January 17, 2026  
**Release Type:** Patch Release  
**Focus Areas:** Documentation Fixes

---

## 🔧 Overview

Version 5.2.1 is a **documentation patch release** that fixes image reference paths in the README to ensure proper display of the v5.2.0 TFT UI screenshots on GitHub.

---

## 🐛 Fixed

### Documentation Image References

Corrected all image paths in the main README.md to match the actual filenames stored in the `docs/images/` directory.

**Changes:**

| Incorrect Path | Correct Path | Description |
|---------------|--------------|-------------|
| `docs/images/welcome.png` | `docs/images/WelcomScreen.png` | Welcome screen with ESP32 logo |
| `docs/images/connecting.png` | `docs/images/STA_Connecting.png` | Station mode connecting screen |
| `docs/images/initializing_ap.png` | `docs/images/AP_Initialize.png` | AP initialization screen |
| `docs/images/disabled.png` | `docs/images/WiFi_Disabled.png` | WiFi disabled screen |
| `docs/images/ap_mode_qr.png` | `docs/images/AP_Info.png` | AP mode with QR code |

### Impact

- **GitHub Display**: All TFT UI screenshots now render correctly in README
- **Documentation Quality**: Professional presentation of v5.2.0 UI redesign features
- **User Experience**: New users can see accurate visual examples of the TFT interface

---

## 📦 Commits

1. **docs: Fix image references in README to match actual filenames** (3e1104e)
   - Updated 5 image paths to correct filenames
   - Ensures proper display of TFT UI documentation

---

## 🔗 Related

- **v5.2.0 Release**: [Complete TFT Display UI Redesign](RELEASE_NOTES_V5.2.0.md)
- **CHANGELOG**: [Full Version History](../CHANGELOG.md)
- **Repository**: [esp32-wifi-utility](https://github.com/arunkumar-mourougappane/esp32-wifi-utility)

---

## ✅ Verification

All image links in README.md now correctly reference actual files:
- ✅ `WelcomScreen.png` - Exists in `docs/images/`
- ✅ `STA_Connecting.png` - Exists in `docs/images/`
- ✅ `AP_Initialize.png` - Exists in `docs/images/`
- ✅ `WiFi_Disabled.png` - Exists in `docs/images/`
- ✅ `AP_Info.png` - Exists in `docs/images/`

---

**Happy WiFi Analysis! 📡**
