# Configuration Persistence System - Complete Implementation

## Overview

The ESP32 WiFi Utility now has a complete persistent configuration system supporting both Access Point (AP) and Station modes. All configurations are saved to ESP32 Non-Volatile Storage (NVS) and survive power cycles and resets.

## System Architecture

### Two Configuration Systems

1. **AP Configuration** (`ap_config`)
   - SSID, Password, Channel
   - Auto-start flag
   - Priority: Highest

2. **Station Configuration** (`sta_config`)
   - SSID, Password
   - Auto-connect flag
   - Priority: Secondary

### Boot Priority Logic

```
┌─────────────────────┐
│   Device Powers On  │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Check AP Config     │
│ with auto-start?    │
└──────────┬──────────┘
           │
     ┌─────┴─────┐
     │ YES       │ NO
     ▼           ▼
┌─────────┐  ┌─────────────────────┐
│ Start   │  │ Check Station Config│
│ as AP   │  │ with auto-connect?  │
└─────────┘  └──────────┬──────────┘
                        │
                  ┌─────┴─────┐
                  │ YES       │ NO
                  ▼           ▼
           ┌─────────┐  ┌─────────┐
           │ Connect │  │ Start   │
           │ to WiFi │  │ in IDLE │
           └─────────┘  └─────────┘
```

## Commands Reference

### AP Configuration

| Command | Description |
|---------|-------------|
| `ap save` | Save current AP config |
| `ap save <ssid> <pass> [ch] [auto]` | Save custom AP config |
| `ap load` | Load and apply saved config |
| `ap show` | Display saved configuration |
| `ap clear` | Clear saved configuration |

### Station Configuration

| Command | Description |
|---------|-------------|
| `station save` | Save current connection |
| `station save <ssid> <pass> [auto]` | Save custom credentials |
| `station load` | Load saved credentials |
| `station show` | Display saved config (password masked) |
| `station clear` | Clear saved credentials |

## Use Case Scenarios

### Scenario 1: Home IoT Device

**Goal**: Auto-connect to home WiFi on boot

**Configuration**:
```bash
# Clear any AP config
ap clear

# Save home WiFi credentials
station save HomeWiFi HomePassword123 yes

# Reboot - device will auto-connect
reset
```

**Boot Behavior**: Device automatically connects to HomeWiFi

---

### Scenario 2: Guest WiFi Hotspot

**Goal**: Device always boots as Access Point

**Configuration**:
```bash
# Clear station config
station clear

# Save AP configuration
ap save GuestWiFi GuestPass123 6 yes

# Reboot - device will start as AP
reset
```

**Boot Behavior**: Device automatically starts as AP on channel 6

---

### Scenario 3: Development Device

**Goal**: Manual control, no auto-start/connect

**Configuration**:
```bash
# Save both but disable auto
ap save DevAP DevAPPass 1 no
station save DevWiFi DevWiFiPass no

# Reboot - manual control
reset
```

**Boot Behavior**: Device starts in IDLE, credentials available for manual use

---

### Scenario 4: Dual-Mode Device

**Goal**: Switch between AP and Station modes

**Configuration**:
```bash
# Save both configurations
ap save MyAP APPass 6 yes
station save MyWiFi WiFiPass yes

# Default: AP mode (higher priority)
reset

# Switch to Station mode
ap clear
reset

# Switch back to AP mode
station clear
ap save MyAP APPass 6 yes
reset
```

---

### Scenario 5: Portable Demo Device

**Goal**: Quickly switch networks at different locations

**Configuration**:

At Location A:
```bash
station clear
station save LocationA_WiFi PassA yes
reset
```

At Location B:
```bash
station clear
station save LocationB_WiFi PassB yes
reset
```

---

## Configuration Priority Matrix

| AP Config | Station Config | Boot Behavior |
|-----------|---------------|---------------|
| auto-start=yes | auto-connect=yes | **AP Mode** (AP priority) |
| auto-start=yes | auto-connect=no | **AP Mode** |
| auto-start=yes | (none) | **AP Mode** |
| auto-start=no | auto-connect=yes | **Station Mode** |
| auto-start=no | auto-connect=no | **IDLE Mode** |
| auto-start=no | (none) | **IDLE Mode** |
| (none) | auto-connect=yes | **Station Mode** |
| (none) | auto-connect=no | **IDLE Mode** |
| (none) | (none) | **IDLE Mode** |

## Security Considerations

### Password Storage

- **AP Passwords**: Stored in NVS, visible via `ap show`
- **Station Passwords**: Stored in NVS, **masked** via `station show`
- **NVS Security**: Not encrypted by default
- **Physical Access**: Anyone with serial access can retrieve configs

### Best Practices

1. ✓ Use strong passwords (WPA2/WPA3 standards)
2. ✓ Clear configs before disposing of devices
3. ✓ Secure physical access to devices
4. ✓ Document saved credentials separately
5. ✓ Use `station show` to verify without exposing password
6. ⚠ Assume serial access = full credential access

## Management Commands

### View All Configurations

```bash
# Check both configurations
ap show
station show

# Check which will take priority on boot
# (Look for auto-start/auto-connect flags)
```

### Reset All Configurations

```bash
# Clear everything
ap clear
station clear

# Verify
ap show
station show

# Device will boot in IDLE mode
reset
```

### Switch Priority

From AP to Station:
```bash
ap clear
# station config now takes priority
reset
```

From Station to AP:
```bash
station clear
# ap config now takes priority
reset
```

## Technical Specifications

### Storage

- **Technology**: ESP32 NVS (Non-Volatile Storage)
- **Namespaces**: 
  - `ap_config` - AP configuration
  - `sta_config` - Station configuration
- **Library**: Preferences (Arduino ESP32 wrapper)
- **Persistence**: Across power cycles, resets, firmware updates

### Data Limits

| Field | Minimum | Maximum |
|-------|---------|---------|
| SSID | 1 char | 32 chars |
| Password (AP) | 8 chars | 63 chars |
| Password (Station) | 0 chars (open) | 63 chars |
| Channel | 1 | 13 |

### Memory Usage

- **Flash**: ~5.8 KB total overhead
- **RAM**: Minimal (static structures only)
- **NVS**: Dynamic based on string lengths

## Troubleshooting Guide

### Issue: Wrong Mode on Boot

**Symptoms**: Device boots as AP when you want Station (or vice versa)

**Solution**:
```bash
# Check configurations
ap show
station show

# Clear the unwanted priority
ap clear        # To use Station mode
# or
station clear   # To use AP mode

# Verify and reboot
reset
```

---

### Issue: Auto-Connect Fails

**Symptoms**: Device tries to connect but fails

**Checks**:
1. Verify network is available: `scan now`
2. Check saved credentials: `station show`
3. Verify password is correct
4. Check if AP has priority: `ap show`

**Solution**:
```bash
# Clear and re-save with correct credentials
station clear
connect CorrectSSID CorrectPassword
station save
reset
```

---

### Issue: Configuration Won't Save

**Symptoms**: Save command fails with error

**Common Causes**:
- SSID too long/short
- Password too long/short (AP must be 8-63)
- Invalid characters

**Solution**:
```bash
# Verify lengths
# SSID: 1-32 characters
# AP Password: 8-63 characters
# Station Password: 0-63 characters

# Use quotes for spaces
station save "My Network" "My Password" yes
```

---

### Issue: Can't Remember Saved Password

**Solution**:
```bash
# For Station mode - can't retrieve password
# Must clear and re-save
station clear
mode station
connect MyNetwork NewPassword
station save

# For AP mode - password is visible
ap show
```

---

## Migration Guide

### From Manual Configuration

**Before**: Manually entering credentials on each boot

**After**: Save credentials once, auto-connect

```bash
# Old workflow (manual)
mode station
connect MyWiFi MyPassword
# ... work ...
# On reboot: repeat above

# New workflow (automated)
mode station
connect MyWiFi MyPassword
station save
# Reboots: automatic connection
```

### From Hardcoded Credentials

**Before**: Credentials in code (config.h)

**After**: Runtime configuration with NVS

```bash
# Deploy device with defaults
# On first boot:
station save ProductionWiFi ProductionPassword yes
# Device now uses saved config, not defaults
```

## API Summary

### C++ Functions

#### AP Configuration
```cpp
bool initAPConfig();
bool saveAPConfig(const APConfig& config);
bool loadAPConfig(APConfig& config);
bool clearAPConfig();
bool hasAPConfig();
void printAPConfig(const APConfig& config);
```

#### Station Configuration
```cpp
bool initStationConfig();
bool saveStationConfig(const StationConfig& config);
bool loadStationConfig(StationConfig& config);
bool clearStationConfig();
bool hasStationConfig();
void printStationConfig(const StationConfig& config);
```

## Files and Documentation

### Implementation Files

- `include/ap_config.h` - AP config API
- `src/ap_config.cpp` - AP config implementation
- `include/station_config.h` - Station config API
- `src/station_config.cpp` - Station config implementation

### Documentation

- `docs/AP_CONFIG_PERSISTENCE.md` - AP configuration guide
- `docs/AP_CONFIG_QUICK_REF.md` - AP quick reference
- `docs/STATION_CONFIG_PERSISTENCE.md` - Station configuration guide
- `docs/STATION_CONFIG_QUICK_REF.md` - Station quick reference
- `docs/CONFIGURATION_SYSTEM.md` - This document

## Version History

### v1.0 (2025-10-25)

- ✓ Initial implementation
- ✓ AP configuration persistence
- ✓ Station configuration persistence
- ✓ Priority system
- ✓ Auto-start/auto-connect
- ✓ Password masking (Station mode)
- ✓ Comprehensive documentation
- ✓ Command interface
- ✓ Build verification

## Support and Resources

### Command Help

Type `help` in serial console for command list

### Documentation

- See `docs/` directory for detailed guides
- Quick reference cards available
- Examples included in documentation

### Testing

Recommended test checklist in each configuration guide

### Community

Report issues via GitHub with:
- Configuration details (`ap show`, `station show`)
- Serial output
- Expected vs actual behavior

---

**Implementation Complete** ✓  
**Build Status**: SUCCESS ✓  
**Documentation**: COMPLETE ✓  
**Testing**: READY ✓
