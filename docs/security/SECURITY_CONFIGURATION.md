# WiFi Security Configuration Guide

## Overview

The ESP32 WiFi Utility provides comprehensive security configuration options for both Access Point (AP) and Station modes. This guide explains available security types, configuration methods, hardware compatibility, and best practices for securing your WiFi connections.

## Table of Contents

- [Access Point Security](#access-point-security)
  - [Available Security Types](#available-security-types)
  - [Configuration Methods](#ap-configuration-methods)
  - [Hardware Compatibility](#hardware-compatibility)
- [Station Security Preferences](#station-security-preferences)
  - [Security Preference Types](#security-preference-types)
  - [Configuration Methods](#station-configuration-methods)
  - [Connection Behavior](#connection-behavior)
- [Serial Command Examples](#serial-command-examples)
- [Web Interface Configuration](#web-interface-configuration)
- [Troubleshooting](#troubleshooting)
- [Security Best Practices](#security-best-practices)

---

## Access Point Security

### Available Security Types

When configuring your ESP32 as an Access Point, you can choose from four security types:

#### 1. **Open** (`open`)

- **Description**: No password required, network is accessible to anyone
- **Use Case**: Public hotspots, temporary networks, testing environments
- **Security Level**: ⚠️ **None** - All traffic unencrypted
- **Password**: Leave empty (`""`)
- **Compatibility**: All devices

**When to Use:**
- Captive portals (with application-layer security)
- Public event WiFi
- Temporary development/testing
- Guest networks (with traffic isolation)

**⚠️ Warning:** Open networks expose all traffic to eavesdropping. Use only when necessary and implement additional security layers.

#### 2. **WPA2-PSK** (`wpa2`)

- **Description**: Industry-standard WiFi security protocol
- **Use Case**: General purpose secure networks (default, recommended)
- **Security Level**: ✅ **Strong** - AES encryption
- **Password**: 8-63 characters required
- **Compatibility**: All modern devices (2004+)

**When to Use:**
- Home networks
- Small office networks
- IoT device communication
- Maximum device compatibility required

**Benefits:**
- Proven security track record
- Universal device support
- Optimal performance
- Industry standard since 2004

#### 3. **WPA3-PSK** (`wpa3`)

- **Description**: Latest WiFi security standard with enhanced protection
- **Use Case**: Maximum security, modern devices only
- **Security Level**: 🔒 **Highest** - SAE authentication, forward secrecy
- **Password**: 8-63 characters required
- **Compatibility**: ESP32-S2/S3/C3/C6+ and WPA3-capable clients only

**When to Use:**
- High-security applications
- Environments with WPA3-capable devices
- Protection against dictionary attacks
- Networks requiring forward secrecy

**Benefits:**
- Protection against offline dictionary attacks
- Forward secrecy (past sessions remain secure)
- Individualized data encryption
- Stronger authentication (SAE)

**⚠️ Hardware Limitation:** 
- ESP32 Classic: Limited/experimental WPA3 support
- ESP32-S2/S3/C3/C6: Full WPA3 support

#### 4. **WPA2/WPA3 Mixed** (`mixed`)

- **Description**: Compatibility mode supporting both WPA2 and WPA3 clients
- **Use Case**: Transitional networks with mixed device capabilities
- **Security Level**: ⚡ **Adaptive** - WPA3 for capable devices, WPA2 fallback
- **Password**: 8-63 characters required
- **Compatibility**: All devices (modern use WPA3, legacy use WPA2)

**When to Use:**
- Gradual WPA3 migration
- Mixed device environments
- Maximum compatibility with enhanced security where available

**Benefits:**
- Seamless device compatibility
- WPA3 security for capable devices
- No client configuration required
- Future-proof setup

---

### AP Configuration Methods

#### Method 1: Serial Commands

##### Quick Start AP Mode
```bash
# Start AP with security type
mode ap MyHotspot MyPassword123 wpa2     # WPA2 (recommended)
mode ap MyHotspot MyPassword123 wpa3     # WPA3 (maximum security)
mode ap MyHotspot MyPassword123 mixed    # Mixed mode (compatibility)
mode ap GuestNetwork "" open             # Open network
```

##### Save Persistent Configuration
```bash
# Save AP config with security (persists across reboots)
ap save MyHotspot MyPassword123 6 wpa2 yes      # WPA2 on channel 6
ap save MyHotspot MyPassword123 11 wpa3 yes     # WPA3 on channel 11
ap save "Guest WiFi" "" 1 open yes              # Open network
ap save MyHotspot MyPassword123 6 mixed no      # Mixed, no auto-start

# Load and use saved config
ap show                                         # View saved configuration
ap load                                         # Apply saved settings
mode ap                                         # Start AP with loaded config

# Clear saved configuration
ap clear
```

##### Parameter Reference
| Parameter | Required | Valid Values | Default | Description |
|-----------|----------|--------------|---------|-------------|
| SSID      | Yes      | 1-32 chars   | -       | Network name |
| Password  | Yes*     | 8-63 chars or "" | -   | Password (empty for open) |
| Channel   | No       | 1-13         | 1       | WiFi channel |
| Security  | No       | open, wpa2, wpa3, mixed | wpa2 | Security type |
| Auto-start| No       | yes/no, 1/0  | yes     | Auto-start on boot |

*Password not required for open networks

#### Method 2: Web Interface

1. Access web configuration page:
   - From AP mode: `http://192.168.4.1` → Config
   - From Station: `http://[device-ip]` → Config

2. Navigate to **Access Point Configuration** section

3. Fill in configuration:
   - **SSID**: Network name
   - **Password**: Leave empty for Open, or 8-63 characters
   - **Channel**: Select 1-13 (default: 1)
   - **Security Type**: Select from dropdown
     - Open (No Password)
     - WPA2-PSK (Recommended)
     - WPA3-PSK (Maximum Security)
     - WPA2/WPA3 Mixed Mode
   - **Auto-start**: Enable to start AP on boot

4. Click **Save AP Configuration**

5. Use **Quick Mode Switch** to activate AP immediately

**JavaScript Behavior:**
- Password field automatically disables when "Open" security is selected
- Password validation enforces 8-63 characters for WPA2/WPA3
- Real-time feedback on configuration status

---

### Hardware Compatibility

Different ESP32 variants have varying levels of WPA3 support:

| ESP32 Variant | Open | WPA2-PSK | WPA3-PSK | WPA2/WPA3 Mixed | Notes |
|---------------|------|----------|----------|-----------------|-------|
| **ESP32 Classic** | ✅ Full | ✅ Full | ⚠️ Limited | ⚠️ Limited | WPA3 experimental, may not be stable |
| **ESP32-S2** | ✅ Full | ✅ Full | ✅ Full | ✅ Full | Full WPA3 support |
| **ESP32-S3** | ✅ Full | ✅ Full | ✅ Full | ✅ Full | Full WPA3 support |
| **ESP32-C3** | ✅ Full | ✅ Full | ✅ Full | ✅ Full | Full WPA3 support |
| **ESP32-C6** | ✅ Full | ✅ Full | ✅ Full | ✅ Full | Full WPA3 support + WiFi 6 |

**Recommendations:**
- **ESP32 Classic**: Use WPA2 for production, WPA3 for testing only
- **ESP32-S2/S3/C3/C6**: All security types fully supported
- **For maximum compatibility**: Use WPA2 or Mixed mode
- **For maximum security**: Use WPA3 on supported hardware

---

## Station Security Preferences

When connecting to WiFi networks as a client (Station mode), you can specify security preferences to enforce minimum security requirements or prefer specific security types.

### Security Preference Types

#### 1. **Auto** (`auto`)

- **Description**: Accept any security type offered by network (default)
- **Behavior**: Connects to WPA3, WPA2, WEP, or Open networks
- **Use Case**: Maximum compatibility, no security enforcement
- **Security Level**: ⚡ **Variable** - Depends on network

**Connection Priority:**
1. Try network's advertised security
2. Accept any successful authentication
3. No rejection based on security type

**When to Use:**
- Public WiFi networks
- Unknown network security
- Maximum connection success rate needed
- Testing different network types

#### 2. **WPA3 Preferred** (`wpa3prefer`)

- **Description**: Prefer WPA3, fallback to WPA2 if WPA3 unavailable
- **Behavior**: 
  - ✅ Connects to WPA3 networks (preferred)
  - ✅ Connects to WPA2 networks (fallback)
  - ✗ Rejects WEP and Open networks
- **Use Case**: Modern security with compatibility fallback
- **Security Level**: 🔒 **Strong to Highest** (WPA3 when available)

**Connection Logic:**
1. Check if network supports WPA3
2. If WPA3 available: Use WPA3
3. If only WPA2 available: Use WPA2
4. If WEP/Open only: Reject connection

**When to Use:**
- Mixed WPA2/WPA3 environments
- Gradual WPA3 migration
- Balance between security and compatibility
- Modern networks with legacy fallback

#### 3. **WPA3 Only** (`wpa3only`)

- **Description**: Require WPA3, reject all other security types
- **Behavior**:
  - ✅ Connects to WPA3 networks only
  - ✗ Rejects WPA2, WEP, and Open networks
- **Use Case**: Maximum security enforcement, WPA3-only networks
- **Security Level**: 🔒 **Highest** - WPA3 mandatory

**When to Use:**
- High-security environments
- WPA3-only infrastructure
- Compliance requirements
- Protection against downgrade attacks

**⚠️ Compatibility Impact:** Will fail to connect to WPA2 networks, even if credentials are correct.

#### 4. **WPA2 Minimum** (`wpa2min`)

- **Description**: Require at least WPA2 security
- **Behavior**:
  - ✅ Connects to WPA3 networks
  - ✅ Connects to WPA2 networks
  - ✗ Rejects WEP and Open networks
- **Use Case**: Enforce modern security, reject insecure networks
- **Security Level**: ✅ **Strong or Higher** - WPA2+ only

**When to Use:**
- Reject insecure WEP/Open networks
- Enforce minimum security standards
- Corporate security policies
- IoT devices in secure environments

**Best for:** Enforcing baseline security while maintaining broad compatibility with modern networks.

#### 5. **WPA2 Only** (`wpa2only`)

- **Description**: Require exactly WPA2, reject WPA3 and lower
- **Behavior**:
  - ✗ Rejects WPA3 networks
  - ✅ Connects to WPA2 networks only
  - ✗ Rejects WEP and Open networks
- **Use Case**: Compatibility with WPA2-only infrastructure, WPA3 incompatibility workarounds
- **Security Level**: ✅ **Strong** - WPA2 specific

**When to Use:**
- WPA3 compatibility issues
- Legacy enterprise infrastructure
- Specific WPA2-only requirements
- Testing and debugging

**⚠️ Note:** Rarely needed; prefer `wpa2min` for broader compatibility.

---

### Station Configuration Methods

#### Method 1: Serial Commands

##### Direct Connection with Security
```bash
# Switch to station mode first
mode station

# Connect with security preference
connect HomeNetwork MyPassword auto            # Accept any security
connect HomeNetwork MyPassword wpa3prefer     # Prefer WPA3
connect HomeNetwork MyPassword wpa3only       # Require WPA3
connect HomeNetwork MyPassword wpa2min        # Minimum WPA2
connect HomeNetwork MyPassword wpa2only       # Exactly WPA2

# Open network (no password)
connect "Coffee Shop WiFi" "" auto
```

##### Save Persistent Configuration
```bash
# Save with security preference (persists across reboots)
station save HomeNetwork MyPassword auto yes          # Auto, with auto-connect
station save HomeNetwork MyPassword wpa3prefer yes   # Prefer WPA3
station save HomeNetwork MyPassword wpa3only yes     # Require WPA3
station save HomeNetwork MyPassword wpa2min no       # WPA2 minimum, no auto-connect

# View saved configuration
station show                    # Password is masked for security

# Load and connect
station load                    # Display saved config
mode station                    # Switch to station mode
# Auto-connects if autoConnect is enabled

# Clear configuration
station clear
```

##### Parameter Reference
| Parameter | Required | Valid Values | Default | Description |
|-----------|----------|--------------|---------|-------------|
| SSID      | Yes      | 1-32 chars   | -       | Network name |
| Password  | Yes      | 0-63 chars   | -       | Password (empty for open) |
| Security  | No       | auto, wpa3prefer, wpa3only, wpa2min, wpa2only | auto | Security preference |
| Auto-connect | No    | yes/no, 1/0  | yes     | Auto-connect on boot |

#### Method 2: Web Interface

1. Access web configuration page:
   - From AP mode: `http://192.168.4.1` → Config
   - From Station: `http://[device-ip]` → Config

2. Navigate to **Station Configuration** section

3. Fill in configuration:
   - **SSID**: Target network name
   - **Password**: Network password (leave empty for open)
   - **Security Preference**: Select from dropdown
     - Auto (Accept Any Security)
     - Prefer WPA3 (Fallback to WPA2)
     - WPA3 Only (Reject WPA2)
     - WPA2 Minimum (Reject WEP/Open)
     - WPA2 Only (Reject WPA3)
   - **Auto-connect**: Enable to auto-connect on boot

4. Click **Save Station Configuration**

5. Use **Quick Mode Switch** to connect immediately

---

### Connection Behavior

Security preferences affect how the ESP32 handles connection attempts:

| Network Security | auto | wpa3prefer | wpa3only | wpa2min | wpa2only |
|------------------|------|------------|----------|---------|----------|
| **WPA3**         | ✅ Connect | ✅ Connect (Preferred) | ✅ Connect | ✅ Connect | ✗ Reject |
| **WPA2**         | ✅ Connect | ✅ Connect (Fallback) | ✗ Reject | ✅ Connect | ✅ Connect |
| **WEP**          | ✅ Connect | ✗ Reject | ✗ Reject | ✗ Reject | ✗ Reject |
| **Open**         | ✅ Connect | ✗ Reject | ✗ Reject | ✗ Reject | ✗ Reject |

**Connection Flow:**
1. ESP32 scans for network with specified SSID
2. Checks network's advertised security type
3. Compares against configured security preference
4. If match: Attempts connection with password
5. If mismatch: Rejects connection, reports security preference violation

---

## Serial Command Examples

### Access Point Configuration

```bash
# Example 1: Home Network (WPA2, Channel 6)
ap save "Home Network" "SecurePassword123" 6 wpa2 yes
mode ap

# Example 2: Maximum Security (WPA3, Channel 11)
ap save "Secure Hotspot" "StrongPass456!" 11 wpa3 yes
mode ap

# Example 3: Guest Network (Open, Channel 1)
ap save "Guest WiFi" "" 1 open yes
mode ap

# Example 4: Mixed Mode (Compatibility)
ap save "Family WiFi" "FamilyPass789" 6 mixed yes
mode ap

# Example 5: Temporary AP (No Auto-start)
mode ap TempHotspot TempPass123 wpa2

# View configuration
ap show

# Clear and reset
ap clear
```

### Station Configuration

```bash
# Example 1: Home Network (Prefer WPA3)
mode station
station save "Home WiFi" "HomePassword123" wpa3prefer yes
# Auto-connects on next boot

# Example 2: Work Network (Require WPA3)
station save "Corporate WiFi" "WorkPass456!" wpa3only yes

# Example 3: Coffee Shop (Auto, No Auto-connect)
station save "Cafe WiFi" "guest123" auto no
# Manual connection required

# Example 4: Secure Network (Minimum WPA2)
station save "Secure Net" "SecurePass789" wpa2min yes

# Example 5: Direct Connection (WPA3 Preferred)
mode station
connect "Friend WiFi" "FriendPass" wpa3prefer

# View saved config
station show

# Clear configuration
station clear
```

---

## Web Interface Configuration

### Accessing the Web Interface

**From Access Point Mode:**
1. Connect device (laptop/phone) to ESP32's WiFi network
2. Open browser: `http://192.168.4.1`
3. Click **⚙️ Config** in navigation menu

**From Station Mode:**
1. Ensure ESP32 is connected to WiFi
2. Find device IP (serial console: `status` command)
3. Open browser: `http://[device-ip]`
4. Click **⚙️ Config** in navigation menu

### Access Point Configuration (Web)

**Configuration Form:**

```
┌─────────────────────────────────────────┐
│ 📡 Access Point Configuration           │
├─────────────────────────────────────────┤
│ SSID:       [MyHotspot____________]    │
│ Password:   [MyPassword123________]    │
│ Channel:    [6_____] (1-13)            │
│ Security:   [▼ WPA2-PSK___________]    │
│             ○ Open (No Password)        │
│             ● WPA2-PSK (Recommended)    │
│             ○ WPA3-PSK (Max Security)   │
│             ○ WPA2/WPA3 Mixed Mode      │
│ Auto-start: [✓] Enable                  │
│                                         │
│ [Save AP Configuration]                 │
└─────────────────────────────────────────┘
```

**Features:**
- Password field disables for Open security
- Real-time validation (8-63 chars for WPA2/WPA3)
- Channel dropdown (1-13)
- Auto-start checkbox
- Save confirmation message

### Station Configuration (Web)

**Configuration Form:**

```
┌─────────────────────────────────────────┐
│ 📶 Station Configuration                │
├─────────────────────────────────────────┤
│ SSID:       [HomeNetwork__________]    │
│ Password:   [Password123__________]    │
│ Security:   [▼ Auto (Any)_________]    │
│             ● Auto (Accept Any)         │
│             ○ Prefer WPA3               │
│             ○ WPA3 Only                 │
│             ○ WPA2 Minimum              │
│             ○ WPA2 Only                 │
│ Auto-connect: [✓] Enable                │
│                                         │
│ [Save Station Configuration]            │
└─────────────────────────────────────────┘
```

**Features:**
- Password field for open networks (leave empty)
- Security preference dropdown
- Auto-connect checkbox
- Save confirmation message
- Saved config display (password masked)

### Quick Mode Switch

Located at top of configuration page:

```
┌─────────────────────────────────────────┐
│ 🔄 Quick Mode Switch                    │
│ Current Mode: Station (Connected)       │
│                                         │
│ [📡 Switch to Access Point]             │
│ [📶 Switch to Station Mode]             │
└─────────────────────────────────────────┘
```

- Instantly switches between AP and Station modes
- Uses saved configurations when available
- Real-time status feedback

---

## Troubleshooting

### Access Point Issues

#### AP Won't Start with WPA3

**Symptom:** `mode ap MyAP MyPass wpa3` fails or clients can't connect

**Possible Causes:**
- ESP32 Classic variant (limited WPA3 support)
- Client devices don't support WPA3

**Solutions:**
```bash
# Check ESP32 variant (serial output on boot)
# For ESP32 Classic: Use WPA2 or Mixed mode
ap save MyAP MyPass 6 wpa2 yes       # WPA2 only
ap save MyAP MyPass 6 mixed yes      # WPA2/WPA3 mixed

# For full WPA3: Use ESP32-S2/S3/C3/C6
```

#### Clients Can't See Open Network

**Symptom:** Open AP saved but not visible to clients

**Check Configuration:**
```bash
ap show                               # Verify security is "Open"
ap save GuestAP "" 1 open yes        # Ensure empty password
mode ap                              # Start AP
ap info                              # Verify AP is running
```

#### Password Too Short Error

**Symptom:** "Password must be 8-63 characters" for WPA2/WPA3

**Solution:**
```bash
# WPA2/WPA3 requires 8-63 characters
ap save MyAP "ShortPW" 6 wpa2       # ✗ Too short (7 chars)
ap save MyAP "LongerPassword" 6 wpa2 # ✅ Valid (14 chars)

# For no password, use Open security
ap save MyAP "" 1 open yes          # ✅ Valid (open network)
```

### Station Connection Issues

#### Connection Fails with "Security Preference Violation"

**Symptom:** Connection rejected due to security mismatch

**Diagnose:**
```bash
mode station
scan now                              # Check network's actual security type
station show                          # Check your security preference
```

**Solutions:**
```bash
# If network is WPA2 but you set wpa3only:
station save MyWiFi MyPass wpa3prefer yes  # Allow WPA2 fallback

# If network is Open but you set wpa2min:
station save MyWiFi "" auto yes           # Accept open networks

# If network is WPA3 but you set wpa2only:
station save MyWiFi MyPass wpa2min yes    # Accept WPA3 too
```

#### WPA3 Connection Fails on Newer Network

**Symptom:** Can't connect to WPA3 network despite having credentials

**Possible Causes:**
- ESP32 Classic variant (limited WPA3 support)
- Network's WPA3 implementation incompatibility

**Solutions:**
```bash
# Try WPA3 Prefer (fallback to WPA2)
station save MyWiFi MyPass wpa3prefer yes

# Check if network offers WPA2
scan now                              # Look for security type

# Ask network admin to enable Mixed mode (WPA2/WPA3)
```

#### Auto-Connect Not Working

**Symptom:** Device doesn't auto-connect on boot despite configuration

**Check Priority:**
```bash
ap show                               # AP takes priority if configured
station show                          # Check if autoConnect is "Yes"

# If both configured, AP has priority
# Clear AP to use Station auto-connect:
ap clear
reset
```

**Verify Configuration:**
```bash
station show                          # Ensure autoConnect: Yes
station save MyWiFi MyPass auto yes  # Re-save with auto-connect
reset                                 # Reboot to test
```

### Web Interface Issues

#### Security Dropdown Not Changing Password Field

**Symptom:** Can't enter password for Open security

**Expected Behavior:**
- **Open**: Password field disabled (grayed out)
- **WPA2/WPA3/Mixed**: Password field enabled

**Solution:**
- Refresh webpage
- Clear browser cache
- Try different browser
- Check browser console for JavaScript errors

#### Configuration Not Saving

**Symptom:** Click "Save" but configuration doesn't persist

**Solutions:**
```bash
# Check serial output for error messages
# Verify NVS (non-volatile storage) is not full

# Clear and re-save via serial:
ap clear
station clear
ap save MyAP MyPass 6 wpa2 yes
station save MyWiFi MyPass auto yes
```

---

## Security Best Practices

### Access Point Security

1. **Use WPA2 or WPA3 (Never Open)**
   - Open networks expose all traffic
   - Use WPA2 minimum for any sensitive data
   - Prefer WPA3 on supported hardware

2. **Strong Passwords**
   ```bash
   # Bad
   ap save MyAP "12345678" 6 wpa2    # Too simple
   
   # Good
   ap save MyAP "MyStr0ng!P@ssw0rd#2024" 6 wpa2
   ```

3. **Avoid WEP and Open**
   - WEP is broken and easily cracked
   - Open networks have zero encryption
   - Use WPA2 minimum

4. **Channel Selection**
   - Use channel analyzer: `channel scan`
   - Avoid congested channels
   - Channels 1, 6, 11 don't overlap (recommended)

5. **Change Default Credentials**
   ```bash
   # Don't use defaults in production
   # Customize SSID and password
   ap save "MyUniqueAP" "MyUniquePassword" 6 wpa2 yes
   ```

### Station Security

1. **Enforce Minimum Security**
   ```bash
   # Reject insecure networks
   station save MyWiFi MyPass wpa2min yes
   ```

2. **Prefer WPA3 When Available**
   ```bash
   # Automatic upgrade to WPA3
   station save MyWiFi MyPass wpa3prefer yes
   ```

3. **Avoid Auto-Accept for Public WiFi**
   ```bash
   # Don't auto-connect to unknown networks
   station save "Public WiFi" "guest" auto no
   ```

4. **Verify Network Security Before Connecting**
   ```bash
   mode station
   scan now                            # Check security type
   # Verify it's the expected network (not evil twin)
   connect MyWiFi MyPass wpa2min
   ```

5. **Clear Credentials Before Device Disposal**
   ```bash
   ap clear
   station clear
   reset
   ```

### General Recommendations

1. **Keep Firmware Updated**
   - Security patches for WiFi vulnerabilities
   - WPA3 improvements and fixes

2. **Use Unique Passwords**
   - Different password for AP vs Station
   - Don't reuse passwords across devices

3. **Monitor Connected Clients (AP Mode)**
   ```bash
   ap clients                          # Check who's connected
   deauth <mac>                       # Remove suspicious clients
   ```

4. **Limit Physical Access**
   - Serial console can read credentials
   - Secure devices physically

5. **Network Isolation**
   - Separate guest from production networks
   - Use VLANs if possible
   - Implement firewall rules

---

## Additional Resources

- **Quick Reference Guides:**
  - [AP Config Quick Reference](../reference/AP_CONFIG_QUICK_REF.md)
  - [Station Config Quick Reference](../reference/STATION_CONFIG_QUICK_REF.md)

- **Security Best Practices:**
  - [Security Best Practices Guide](../security/SECURITY_BEST_PRACTICES.md)

- **Web Interface:**
  - [Web Configuration Guide](../configuration/WEB_CONFIGURATION.md)

- **Technical Details:**
  - [Configuration System Architecture](../configuration/CONFIGURATION_SYSTEM.md)
  - [WiFi Task Architecture](technical/WIFI_TASK_ARCHITECTURE.md)

---

## Conclusion

The ESP32 WiFi Utility provides flexible and comprehensive security configuration for both Access Point and Station modes. By understanding available security types, hardware compatibility, and configuration methods, you can:

- **Maximize Security**: Use WPA3 where supported
- **Ensure Compatibility**: Use Mixed or WPA2 modes for broader device support
- **Enforce Standards**: Reject insecure networks with security preferences
- **Simplify Management**: Use persistent configuration and web interface

For questions or issues, refer to the troubleshooting section or consult the complete documentation index.
