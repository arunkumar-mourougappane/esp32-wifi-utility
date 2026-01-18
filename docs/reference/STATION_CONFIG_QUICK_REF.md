# Station Configuration Quick Reference

## Save Configuration

```bash
# Save current connection (must be connected)
station save

# Save custom credentials
station save MyNetwork Password123                      # Default: auto security, auto-connect
station save MyNetwork Password123 auto yes            # Explicit auto security
station save MyNetwork Password123 wpa3prefer yes      # Prefer WPA3, fallback WPA2
station save MyNetwork Password123 wpa3only yes        # Require WPA3 only
station save MyNetwork Password123 wpa2min yes         # Minimum WPA2 (reject WEP/Open)
station save MyNetwork Password123 wpa2only yes        # Require exactly WPA2
station save MyNetwork Password123 auto no             # No auto-connect

# Open network (no password)
station save OpenNetwork "" auto yes

# With spaces (use quotes)
station save "My Network" "My Password 123" wpa2min yes
```

## View Configuration

```bash
# Show saved configuration (password masked)
station show
```

## Load Configuration

```bash
# Load and display saved configuration
station load
```

## Clear Configuration

```bash
# Remove saved configuration (disables auto-connect)
station clear
```

## Parameters

| Parameter | Required | Valid Values | Default | Description |
|-----------|----------|--------------|---------|-------------|
| SSID      | Yes      | 1-32 chars   | -       | Network name |
| Password  | Yes      | 0-63 chars   | -       | Password (empty for open) |
| Security  | No       | auto, wpa3prefer, wpa3only, wpa2min, wpa2only | auto | Security preference |
| Auto-connect | No    | yes/no, true/false, 1/0 | yes | Auto-connect on boot |

## Security Preferences

| Preference    | WPA3 Networks | WPA2 Networks | WEP/Open Networks | Use Case |
|---------------|---------------|---------------|-------------------|----------|
| `auto`        | ✅ Connect    | ✅ Connect    | ✅ Connect        | Maximum compatibility |
| `wpa3prefer`  | ✅ Connect (preferred) | ✅ Connect (fallback) | ✗ Reject | Modern security with fallback |
| `wpa3only`    | ✅ Connect    | ✗ Reject      | ✗ Reject          | Maximum security only |
| `wpa2min`     | ✅ Connect    | ✅ Connect    | ✗ Reject          | Enforce modern security |
| `wpa2only`    | ✗ Reject      | ✅ Connect    | ✗ Reject          | WPA2-specific networks |

## Boot Behavior

| Scenario | Boot Behavior |
|----------|---------------|
| No saved config | Device starts in IDLE mode |
| Saved config with auto-connect=yes | Device connects to saved network |
| Saved config with auto-connect=no | Device starts in IDLE, credentials available |
| Both AP and Station saved | **AP takes priority** |

## Priority System

When both AP and Station configs are saved:

1. **AP Config with auto-start**: Device boots as AP (highest priority)
2. **Station Config with auto-connect**: Device connects to WiFi
3. **Neither with auto**: Device starts in IDLE

To change priority:
```bash
# Use Station mode
ap clear
station save MyNetwork Pass yes

# Use AP mode
station clear
ap save MyAP APPass 6 yes
```

## Common Use Cases

### Home Network (Prefer WPA3)
```bash
station save "Home WiFi" "HomePassword123" wpa3prefer yes
# Auto-connects on boot, prefers WPA3
```

### Corporate Network (Minimum WPA2)
```bash
station save "Corporate WiFi" "CorpPass456!" wpa2min yes
# Rejects insecure WEP/Open networks
```

### Public WiFi (No Auto-connect)
```bash
station save "Coffee Shop" "guest123" auto no
# Manual connection required
```

### Maximum Security (WPA3 Only)
```bash
station save "Secure Network" "VerySecure!Pass" wpa3only yes
# Only connects to WPA3 networks
```

### IoT Backend (Auto Security)
```bash
station save "IoT_Backend" "IoTPass789" auto yes
# Accepts any security type
```

### IoT Device (Home Network)
```bash
connect MyHomeWiFi Password123
station save
# Device will auto-connect on every boot
```

### Development Device
```bash
station save DevNetwork DevPass no
# Credentials saved but no auto-connect
# Use 'connect MyDevWiFi DevPass' when needed
```

### Portable Device (Multiple Networks)
```bash
# At home
station save HomeWiFi HomePass yes

# At office (clear home network first)
station clear
station save OfficeWiFi OfficePass yes
```

### Backup Configuration
```bash
# Save backup network
station save GuestWiFi GuestPass no
# Won't auto-connect but credentials are saved
```

## Workflow Examples

### First Time Setup
```bash
1. mode station          # Enter station mode
2. scan now              # Find networks
3. connect MyWiFi Pass   # Connect
4. station save          # Save credentials
5. reset                 # Test auto-connect
```

### Change Network
```bash
1. station clear         # Clear old network
2. connect NewWiFi Pass  # Connect to new
3. station save          # Save new network
4. reset                 # Test
```

### Temporary Connection
```bash
1. mode station
2. connect TempWiFi TempPass  # Connect without saving
3. # Work...
4. disconnect            # Disconnect
# Saved config unchanged
```

## Troubleshooting

**Not auto-connecting?**
```bash
station show             # Check if auto-connect is "Yes"
ap show                  # Check if AP has priority
ap clear                 # Clear AP if needed
```

**Wrong credentials saved?**
```bash
station clear
connect CorrectSSID CorrectPass auto
station save
```

**Can't connect?**
```bash
mode station
scan now                 # Check if network is visible
# Verify SSID spelling and password
station clear
station save SSID Pass auto yes
reset
```

**Security preference violation?**
```bash
# Check network's actual security
scan now

# Adjust security preference
station save MyWiFi Pass wpa3prefer yes    # Allow WPA2 fallback
# Or
station save MyWiFi Pass auto yes          # Accept any security
```

**WPA3 connection fails?**
```bash
# Try WPA3 Prefer (allows WPA2 fallback)
station save MyWiFi Pass wpa3prefer yes

# Or enforce minimum WPA2
station save MyWiFi Pass wpa2min yes
```

**Want manual control?**
```bash
# Save credentials but disable auto-connect
station save MyWiFi MyPass auto no

# Connect manually when needed
mode station
connect MyWiFi MyPass auto
```

## Security Tips

✓ Use strong passwords (WPA2/WPA3 standards)
✓ Clear credentials before disposing device
✓ Keep device physically secure
✓ Use `station show` to verify (password masked)
⚠ Serial access can retrieve credentials
⚠ Physical access = full access to config

## Quick Commands

```bash
# Save current
station save

# View saved
station show

# Clear
station clear

# Priority check
ap show && station show

# Reset all
ap clear && station clear && reset
```
