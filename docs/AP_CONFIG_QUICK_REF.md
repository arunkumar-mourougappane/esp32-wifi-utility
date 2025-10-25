# AP Configuration Quick Reference

## Save Configuration

```bash
# Save current AP settings (must be in AP mode)
ap save

# Save custom configuration
ap save MyNetwork Password123         # Default channel 1, auto-start enabled
ap save MyNetwork Password123 6       # Channel 6, auto-start enabled
ap save MyNetwork Password123 6 yes   # Explicit auto-start
ap save MyNetwork Password123 1 no    # No auto-start

# With spaces (use quotes)
ap save "My Network" "My Password 123" 11 yes
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

| Parameter | Required | Valid Values | Default |
|-----------|----------|--------------|---------|
| SSID      | Yes      | 1-32 chars   | -       |
| Password  | Yes      | 8-63 chars   | -       |
| Channel   | No       | 1-13         | 1       |
| Auto-start| No       | yes/no, true/false, 1/0 | yes |

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
ap save DevNetwork DevPass123 1 no
```

### Production Deployment
```bash
# Save with auto-start for headless operation
ap save ProductionAP SecurePass2024 6 yes
```

### Guest Network
```bash
# Configure guest network with clear channel
ap save GuestWiFi GuestPass123 11 yes
```

## Troubleshooting

**Configuration not saved?**
- Check SSID length (1-32 chars)
- Check password length (8-63 chars)
- Look for error messages in serial output

**Auto-start not working?**
- Verify with `ap show` that auto-start is "Yes"
- Ensure proper device reset (not just WiFi off/on)

**Need to reset?**
```bash
ap clear              # Clear saved config
mode off              # Turn off WiFi
reset                 # Restart device
```
