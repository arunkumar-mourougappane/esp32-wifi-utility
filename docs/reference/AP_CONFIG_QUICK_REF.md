# AP Configuration Quick Reference

## Save Configuration

```bash
# Save current AP settings (must be in AP mode)
ap save

# Save custom configuration
ap save MyNetwork Password123                    # Default: channel 1, WPA2, auto-start
ap save MyNetwork Password123 6                  # Channel 6, WPA2, auto-start
ap save MyNetwork Password123 6 wpa2 yes         # Explicit WPA2, auto-start
ap save MyNetwork Password123 11 wpa3 yes        # WPA3 security
ap save MyNetwork Password123 6 mixed yes        # WPA2/WPA3 mixed mode
ap save "Guest WiFi" "" 1 open yes               # Open network (no password)
ap save MyNetwork Password123 6 wpa2 no          # No auto-start

# With spaces (use quotes)
ap save "My Network" "My Password 123" 11 wpa2 yes
```

## View Configuration

```bash
# Show saved or default configuration
ap show
```

## Load Configuration

```bash
# Load saved configuration and apply to current settings
ap load

# Then start AP with loaded settings
mode ap
```

## Clear Configuration

```bash
# Remove saved configuration (disables auto-start)
ap clear
```

## Parameters

| Parameter | Required | Valid Values | Default | Description |
|-----------|----------|--------------|---------|-------------|
| SSID      | Yes      | 1-32 chars   | -       | Network name |
| Password  | Yes*     | 8-63 chars or "" | -   | Password (empty for open) |
| Channel   | No       | 1-13         | 1       | WiFi channel |
| Security  | No       | open, wpa2, wpa3, mixed | wpa2 | Security type |
| Auto-start| No       | yes/no, true/false, 1/0 | yes | Auto-start on boot |

*Password not required for `open` security type

## Boot Behavior

| Scenario | Boot Behavior |
|----------|---------------|
| No saved config | Device starts in IDLE mode |
| Saved config with auto-start=yes | Device auto-starts in AP mode |
| Saved config with auto-start=no | Device starts in IDLE, uses saved settings when AP manually started |

## Common Use Cases

### Development/Testing
```bash
# Disable auto-start for development
ap save DevNetwork DevPass123 1 wpa2 no
```

### Production Deployment
```bash
# Save with auto-start for headless operation
ap save ProductionAP SecurePass2024 6 wpa2 yes
```

### Guest Network (Open)
```bash
# Configure guest network with no password
ap save "Guest WiFi" "" 11 open yes
```

### Maximum Security (WPA3)
```bash
# WPA3-only network (ESP32-S2/S3/C3+ only)
ap save SecureNetwork VerySecurePass!2024 6 wpa3 yes
```

### Mixed Mode (Compatibility)
```bash
# WPA2/WPA3 mixed for broader device support
ap save FamilyWiFi FamilyPass2024 6 mixed yes
```

## Security Types

| Security Type | Password Required | Description | Compatibility |
|---------------|-------------------|-------------|---------------|
| `open`        | No (use "")       | No encryption | All devices |
| `wpa2`        | Yes (8-63 chars)  | WPA2-PSK (recommended) | All modern devices |
| `wpa3`        | Yes (8-63 chars)  | WPA3-PSK (maximum security) | ESP32-S2/S3/C3/C6, WPA3 clients |
| `mixed`       | Yes (8-63 chars)  | WPA2/WPA3 mixed mode | All devices (auto-select) |

## Troubleshooting

**Configuration not saved?**
- Check SSID length (1-32 chars)
- Check password length (8-63 chars for WPA2/WPA3, empty for open)
- Verify security type spelling: `open`, `wpa2`, `wpa3`, `mixed`
- Look for error messages in serial output

**Auto-start not working?**
- Verify with `ap show` that auto-start is "Yes"
- Ensure proper device reset (not just WiFi off/on)

**WPA3 not working?**
- Check ESP32 variant: WPA3 fully supported on S2/S3/C3/C6 only
- ESP32 Classic has limited WPA3 support (use WPA2 or Mixed)
- Verify client devices support WPA3

**Clients can't connect?**
- Check password is correct (case-sensitive)
- Verify security type matches client expectations
- For open networks, ensure password is empty `""`
- Try Mixed mode for compatibility issues

**Need to reset?**
```bash
ap clear              # Clear saved config
mode off              # Turn off WiFi
reset                 # Restart device
```
