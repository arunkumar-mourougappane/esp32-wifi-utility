# WiFi Security Best Practices

## Overview

This guide provides comprehensive security recommendations for deploying ESP32 WiFi devices in various environments. Follow these best practices to ensure secure WiFi communication, protect against common attacks, and maintain compliance with security standards.

## Table of Contents

- [Security Type Selection](#security-type-selection)
- [Password Management](#password-management)
- [Access Point Best Practices](#access-point-best-practices)
- [Station Mode Best Practices](#station-mode-best-practices)
- [Hardware-Specific Considerations](#hardware-specific-considerations)
- [Network Architecture](#network-architecture)
- [Common Security Pitfalls](#common-security-pitfalls)
- [Compliance and Standards](#compliance-and-standards)
- [Security Checklist](#security-checklist)

---

## Security Type Selection

### When to Use Each Security Type

#### Open Networks (Not Recommended)

**Use Only When:**
- ✓ Implementing captive portal with application-layer security
- ✓ Temporary development/testing in controlled environment
- ✓ Public event WiFi with traffic isolation
- ✓ Guest network with strict firewall rules

**Never Use For:**
- ✗ Home networks
- ✗ Corporate networks
- ✗ IoT device communication
- ✗ Any sensitive data transmission

**Risks:**
- All traffic visible to eavesdroppers
- Man-in-the-middle attacks trivial
- No authentication or encryption
- Session hijacking easy

**Mitigation (if Open is required):**
```bash
# Use VPN or TLS for all traffic
# Implement network isolation
# Deploy firewall rules
ap save "GuestPublic" "" 1 open yes
```

#### WPA2-PSK (Recommended for Most Cases)

**Use For:**
- ✓ Home networks
- ✓ Small office networks
- ✓ IoT device communication
- ✓ Maximum device compatibility
- ✓ General purpose secure networks

**Benefits:**
- Industry standard since 2004
- Universal device support
- Strong AES encryption
- Proven security track record
- Optimal performance

**Configuration:**
```bash
# Home network
ap save "HomeNetwork" "StrongPassword!2024" 6 wpa2 yes

# IoT devices
ap save "IoT_Network" "IoT!Secure#Pass456" 11 wpa2 yes
```

**Best Practices:**
- Use strong passwords (16+ characters)
- Rotate passwords periodically (every 90 days recommended)
- Don't share credentials unnecessarily

#### WPA3-PSK (Maximum Security)

**Use For:**
- ✓ High-security applications
- ✓ Corporate sensitive data
- ✓ Environments with WPA3-capable devices
- ✓ Protection against dictionary attacks
- ✓ Networks requiring forward secrecy

**Benefits:**
- Protection against offline dictionary attacks
- Forward secrecy (past sessions remain secure)
- Individualized data encryption (per-client)
- Stronger authentication (SAE vs PSK)
- Management frame protection built-in

**Hardware Requirements:**
- ESP32-S2, S3, C3, C6 (full support)
- ESP32 Classic (limited/experimental)
- Client devices must support WPA3

**Configuration:**
```bash
# Maximum security network
ap save "SecureNet" "Very!Strong#Passphrase$2024" 11 wpa3 yes

# Station: Require WPA3
station save "Corporate_WPA3" "Corp!Pass#123" wpa3only yes
```

**Migration Strategy:**
```bash
# Step 1: Start with Mixed mode
ap save "TransitionNet" "MigrationPass!2024" 6 mixed yes

# Step 2: Verify all clients connect successfully
ap clients    # Monitor connections

# Step 3: Switch to WPA3-only after full client migration
ap save "SecureNet" "MigrationPass!2024" 6 wpa3 yes
```

#### WPA2/WPA3 Mixed Mode (Compatibility)

**Use For:**
- ✓ Gradual WPA3 migration
- ✓ Mixed device environments (legacy + modern)
- ✓ Future-proof setup
- ✓ Maximum compatibility with enhanced security

**Benefits:**
- WPA3 for capable devices
- WPA2 fallback for legacy devices
- No client configuration required
- Automatic security upgrade

**Considerations:**
- WPA2 clients don't get WPA3 benefits
- Slightly increased overhead
- All WPA2 vulnerabilities still present for legacy clients

**Configuration:**
```bash
# Optimal for most environments
ap save "FamilyWiFi" "Family!Secure#2024" 6 mixed yes
```

---

## Password Management

### Password Strength Requirements

**Minimum Requirements:**
- **WPA2/WPA3**: 8-63 characters (enforced)
- **Recommended**: 16+ characters with complexity

**Strong Password Characteristics:**
```bash
# Weak (Avoid)
ap save "MyAP" "12345678" 6 wpa2         # ✗ Too simple
ap save "MyAP" "password" 6 wpa2         # ✗ Dictionary word
ap save "MyAP" "qwerty123" 6 wpa2        # ✗ Common pattern

# Strong (Recommended)
ap save "MyAP" "Tr0pic@l!Sunset#2024" 6 wpa2     # ✅ Mixed case, symbols, numbers
ap save "MyAP" "Correct-Horse-Battery-Staple" 6 wpa2  # ✅ Long passphrase
ap save "MyAP" "MyD0g$Name&IsMax#2024" 6 wpa2    # ✅ Personal + complexity
```

### Password Generation

**Recommended Method - Passphrase:**
```bash
# Use 4-5 random words with separators
"Forest-Mountain-River-Sky"
"Purple@Elephant!Dances&Freely"
"Quick$Brown*Fox#Jumps"
```

**Benefits:**
- Easy to remember
- Hard to crack (high entropy)
- Meets complexity requirements

**Tools:**
```bash
# Linux: Generate random password
openssl rand -base64 24

# Output: K7x9Qw2mP4nL8sT6vY0uZ5cA
```

### Password Rotation

**Recommended Schedule:**

| Environment | Rotation Frequency | Reason |
|-------------|-------------------|--------|
| Home Network | Every 90-180 days | Low risk |
| Office Network | Every 60-90 days | Medium risk |
| Corporate Network | Every 30-60 days | High risk |
| Public Hotspot | After each event | Very high risk |

**Rotation Process:**
```bash
# 1. Save new configuration
ap save "MyNetwork" "NewStrongPassword!2024" 6 wpa2 yes

# 2. Notify users
# 3. Update all client devices
# 4. Monitor connections
ap clients

# 5. Verify all clients reconnected
```

### Password Storage

**❌ Don't:**
- Store in plain text files
- Share via unencrypted email
- Write on sticky notes
- Use same password for multiple networks

**✅ Do:**
- Use password manager
- Encrypt password files
- Share via secure channels (encrypted messaging)
- Use unique passwords per network

**Retrieving Stored Passwords:**
```bash
# Passwords are stored in NVS (encrypted)
# View configuration (password masked in output)
ap show
station show

# Serial access = full access
# Secure physical access to device!
```

---

## Access Point Best Practices

### SSID Configuration

**Recommendations:**

```bash
# ✗ Avoid: Revealing device information
ap save "ESP32-Living-Room" "pass" 6 wpa2    # Reveals location
ap save "Default-ESP32-AP" "pass" 6 wpa2     # Generic default

# ✅ Good: Non-descriptive but memorable
ap save "Skyline42" "StrongPass!2024" 6 wpa2
ap save "CoffeeTime" "Brew$Strong#123" 6 wpa2
```

**SSID Best Practices:**
- Don't reveal device type, location, or purpose
- Avoid personal information
- Keep it memorable but non-descriptive
- Don't use names that attract attention

### Channel Selection

**Optimal Channel Selection:**

```bash
# Step 1: Scan current spectrum
channel scan detailed

# Step 2: Get recommendation
channel recommend

# Step 3: Configure based on recommendation
ap save "MyNetwork" "SecurePass!2024" [recommended-channel] wpa2 yes
```

**Channel Guidelines:**
- **Channels 1, 6, 11**: Non-overlapping (recommended)
- **Avoid**: Channels 2-5, 7-10 (overlap issues)
- **Consider**: Neighboring network interference
- **Monitor**: Reassess every 3-6 months

**Interference Management:**
```bash
# Monitor channel congestion
channel monitor start

# Check status periodically
channel status

# Export data for analysis
channel export
```

### Client Management

**Monitor Connected Clients:**
```bash
# List all clients
ap clients

# Monitor for suspicious activity
# Unknown MAC addresses
# Unusual connection times
# High bandwidth usage
```

**Deauthentication:**
```bash
# Remove specific client
deauth XX:XX:XX:XX:XX:XX

# Emergency: Disconnect all clients
deauth all

# Then change password
ap save "MyNetwork" "NewSecurePass!2024" 6 wpa2 yes
mode ap
```

### QR Code Security

**QR Code Best Practices:**
```bash
# Generate QR code for easy client connection
qr

# ⚠️ Security Considerations:
# - QR code displays credentials in plain text
# - Anyone who photographs QR can access network
# - Clear QR code when not needed
```

**Recommended Usage:**
- Display QR temporarily during device setup
- Cover/hide QR code when not actively adding devices
- Regenerate QR after credential changes
- Don't photograph/share QR codes publicly

---

## Station Mode Best Practices

### Security Preference Selection

**Use Cases and Recommendations:**

#### Home/Personal Networks
```bash
# Prefer maximum security, fallback to compatibility
station save "HomeWiFi" "HomePass!2024" wpa3prefer yes
```

#### Corporate Networks
```bash
# Enforce minimum WPA2 security
station save "CorpWiFi" "Corp!Pass#123" wpa2min yes

# Or require WPA3 if infrastructure supports
station save "CorpWiFi" "Corp!Pass#123" wpa3only yes
```

#### Public WiFi (Coffee Shops, Hotels)
```bash
# Accept any security, but don't auto-connect
station save "CoffeeShop" "guest123" auto no

# Manually connect when needed
mode station
connect "CoffeeShop" "guest123" auto
```

#### IoT/Machine Networks
```bash
# Enforce WPA2 minimum for security
station save "IoT_Backend" "IoT!Secure#456" wpa2min yes
```

### Network Verification

**Before Connecting - Verify Network:**

```bash
# Scan for networks
mode station
scan now

# Verify:
# 1. Correct SSID (beware evil twins)
# 2. Expected security type
# 3. Reasonable signal strength
# 4. Known BSSID (MAC address) if possible

# Then connect with security preference
connect "VerifiedNetwork" "KnownPassword" wpa2min
```

**Evil Twin Detection:**
- Multiple networks with same SSID = suspicious
- Verify BSSID (MAC address) matches known AP
- Check signal strength (too strong = nearby rogue AP)
- Confirm network owner before connecting

### Auto-Connect Configuration

**When to Enable Auto-Connect:**

```bash
# ✅ Enable for trusted networks
station save "HomeWiFi" "HomePass" wpa3prefer yes

# ✅ Enable for fixed installations
station save "FactoryWiFi" "SecurePass" wpa2min yes
```

**When to Disable Auto-Connect:**

```bash
# ❌ Disable for public WiFi
station save "PublicWiFi" "guest" auto no

# ❌ Disable for temporary connections
station save "FriendWiFi" "temppass" auto no

# ❌ Disable for networks you rarely use
station save "AirportWiFi" "traveler" auto no
```

### Credential Management

**Principle: Least Exposure**

```bash
# Clear credentials before disposing device
station clear
ap clear
reset

# Don't save credentials for one-time connections
# Use direct connect instead:
mode station
connect "OneTimeNetwork" "temppass" auto
# Don't use: station save
```

---

## Hardware-Specific Considerations

### ESP32 Classic

**Limitations:**
- WPA3 support experimental/limited
- May have compatibility issues with WPA3 networks
- Mixed mode may default to WPA2 for AP

**Recommendations:**
```bash
# Use WPA2 for production
ap save "ESP32_AP" "SecurePass!2024" 6 wpa2 yes

# Station: Use wpa3prefer (will fallback to WPA2)
station save "Network" "Pass" wpa3prefer yes

# Avoid wpa3only on ESP32 Classic
```

### ESP32-S2, S3, C3, C6

**Capabilities:**
- Full WPA3 support
- WPA3-SAE (Simultaneous Authentication of Equals)
- Management frame protection
- All security types fully functional

**Recommendations:**
```bash
# Leverage WPA3 capabilities
ap save "S3_SecureAP" "VerySecure!Pass#2024" 11 wpa3 yes

# Station: Prefer or require WPA3
station save "ModernNet" "Pass" wpa3prefer yes
station save "HighSecNet" "Pass" wpa3only yes
```

### Hardware Compatibility Matrix

| Feature | ESP32 Classic | ESP32-S2/S3/C3/C6 |
|---------|---------------|-------------------|
| Open | ✅ Full | ✅ Full |
| WPA2-PSK | ✅ Full | ✅ Full |
| WPA3-PSK | ⚠️ Limited | ✅ Full |
| WPA2/WPA3 Mixed | ⚠️ May default WPA2 | ✅ Full |
| Management Frame Protection | ⚠️ Limited | ✅ Full |
| **Recommendation** | Use WPA2 | Use WPA3 or Mixed |

---

## Network Architecture

### Network Segmentation

**Recommended Topology:**

```
Internet
    │
┌───┴───────────────────┐
│  Router/Firewall      │
│  (Main Gateway)       │
└───┬───────────────────┘
    │
    ├─── Production Network (VLAN 10)
    │    └─ ESP32 Devices (WPA3)
    │
    ├─── Guest Network (VLAN 20)
    │    └─ Public Access (WPA2, isolated)
    │
    └─── IoT Network (VLAN 30)
         └─ Sensors/Devices (WPA2, restricted)
```

**Implementation:**
```bash
# Production AP (Secure)
ap save "Production" "Prod!Secure#123" 6 wpa3 yes

# Guest AP (Isolated)
ap save "Guest" "Guest!Pass#456" 1 wpa2 yes
# Configure firewall: Internet only, no internal access

# IoT AP (Restricted)
ap save "IoT_Secure" "IoT!Complex#789" 11 wpa2 yes
# Configure firewall: Specific service access only
```

### Firewall Rules

**Recommended Rules:**

**Guest Network:**
- Allow: Internet access (HTTP, HTTPS, DNS)
- Deny: Internal network access
- Deny: ESP32 management interface
- Deny: Other VLANs

**IoT Network:**
- Allow: Specific services (MQTT, HTTP API)
- Allow: Time sync (NTP)
- Deny: Unnecessary internet access
- Deny: Cross-VLAN except required

**Production Network:**
- Allow: Required services only
- Monitor: Unusual traffic patterns
- Log: All connection attempts

### Physical Security

**Device Placement:**
- ✅ Secure mounting (prevent theft)
- ✅ Protected from environmental tampering
- ✅ Limited physical access
- ❌ Avoid public areas with easy access

**Serial Console Access:**
```bash
# ⚠️ Warning: Serial access = full credentials access
# Protect devices physically

# For production:
# - Disable serial output if possible
# - Use secure enclosures
# - Monitor physical access logs
```

---

## Common Security Pitfalls

### Pitfall 1: Using Default Credentials

**Problem:**
```bash
# ✗ Never use defaults in production
ap save "ESP32_AP" "12345678" 6 wpa2
```

**Solution:**
```bash
# ✅ Always customize
ap save "UniqueAP42" "RandomStrong!Pass#2024" 6 wpa2 yes
```

### Pitfall 2: Password Reuse

**Problem:**
```bash
# ✗ Same password for AP and Station
ap save "MyAP" "SamePass123" 6 wpa2 yes
station save "MyNetwork" "SamePass123" wpa2min yes
```

**Solution:**
```bash
# ✅ Unique passwords for each context
ap save "MyAP" "AP!Unique#Pass$123" 6 wpa2 yes
station save "MyNetwork" "Net!Different#Pass$456" wpa2min yes
```

### Pitfall 3: Auto-Connect to Public WiFi

**Problem:**
```bash
# ✗ Auto-connect to untrusted network
station save "Airport_Free_WiFi" "guest" auto yes
```

**Solution:**
```bash
# ✅ Manual connection for public networks
station save "Airport_Free_WiFi" "guest" auto no
# Connect manually when needed
```

### Pitfall 4: Ignoring Security Preferences

**Problem:**
```bash
# ✗ Always using 'auto' (no enforcement)
station save "Network" "Pass" auto yes
```

**Solution:**
```bash
# ✅ Enforce minimum security standards
station save "Network" "Pass" wpa2min yes
# Or prefer WPA3
station save "Network" "Pass" wpa3prefer yes
```

### Pitfall 5: Exposing QR Codes

**Problem:**
- Displaying QR code continuously
- Photographing QR code and sharing
- Public QR code visibility

**Solution:**
```bash
# Display QR only during device setup
qr

# Clear after use (disconnect or mode switch)
mode idle

# Regenerate after credential changes
ap save "MyAP" "NewPass" 6 wpa2 yes
mode ap
qr
```

### Pitfall 6: Not Clearing Credentials Before Disposal

**Problem:**
- Selling/disposing device with stored credentials
- Credentials recoverable from NVS

**Solution:**
```bash
# Before disposal/resale
ap clear
station clear
reset

# Verify cleared
ap show        # Should show "No saved configuration"
station show   # Should show "No saved configuration"
```

---

## Compliance and Standards

### Industry Standards

**WiFi Alliance Certification:**
- WPA2-Personal: IEEE 802.11i (2004)
- WPA3-Personal: WiFi Alliance (2018)
- WPA3-SAE: Dragonfly handshake (RFC 7664)

**Compliance Considerations:**

| Standard | Requirement | Implementation |
|----------|-------------|----------------|
| **PCI DSS** | No default passwords | Customize all credentials |
| **HIPAA** | Encryption required | Use WPA2/WPA3 minimum |
| **GDPR** | Data protection | Secure WiFi + encryption layers |
| **ISO 27001** | Access control | Authenticate all clients (no Open) |
| **NIST** | Strong authentication | WPA3 preferred, WPA2 minimum |

### Enterprise Recommendations

**For Compliance:**
```bash
# Minimum WPA2 enforcement
station save "CorpNet" "ComplexPass!2024" wpa2min yes

# Password complexity
# - Minimum 16 characters
# - Mixed case, numbers, symbols
# - Rotation every 60 days

# Audit logging
# - Monitor connection attempts
# - Log deauthentication events
# - Track configuration changes
```

**Documentation Requirements:**
- Security policy documentation
- Password rotation schedule
- Incident response procedures
- Configuration change logs
- Client access records

---

## Security Checklist

### Pre-Deployment Checklist

**Configuration:**
- [ ] Changed default SSID to non-descriptive name
- [ ] Set strong password (16+ characters, complexity)
- [ ] Selected appropriate security type (WPA2/WPA3)
- [ ] Configured optimal WiFi channel
- [ ] Disabled auto-start if not needed
- [ ] Configured security preferences for Station mode

**Network:**
- [ ] Implemented network segmentation (if applicable)
- [ ] Configured firewall rules
- [ ] Tested connectivity and security
- [ ] Verified QR code security (if used)
- [ ] Documented network architecture

**Physical:**
- [ ] Secure device mounting
- [ ] Limited physical access
- [ ] Protected serial console
- [ ] Environmental protection

### Post-Deployment Checklist

**Monitoring:**
- [ ] Monitor connected clients regularly
- [ ] Review connection logs
- [ ] Check for suspicious activity
- [ ] Verify security configuration periodically

**Maintenance:**
- [ ] Schedule password rotation
- [ ] Update firmware regularly
- [ ] Reassess channel selection
- [ ] Review security preferences

**Documentation:**
- [ ] Document configuration
- [ ] Record password changes
- [ ] Log security incidents
- [ ] Update network diagrams

### Incident Response Checklist

**If Compromise Suspected:**
1. [ ] Disconnect affected device
2. [ ] Change all passwords immediately
3. [ ] Deauthenticate all clients
4. [ ] Review connection logs
5. [ ] Investigate unauthorized access
6. [ ] Update configuration
7. [ ] Monitor for recurrence
8. [ ] Document incident

**Commands:**
```bash
# Emergency response
deauth all                          # Disconnect all clients
ap clear                            # Clear AP config
station clear                       # Clear Station config
ap save "NewAP" "NewPass!2024" 6 wpa2 yes
mode ap
# Document incident details
```

---

## Conclusion

Security is not a one-time configuration but an ongoing process. By following these best practices, you can:

- **Minimize Attack Surface**: Use strong security types and passwords
- **Enforce Standards**: Implement security preferences
- **Maintain Vigilance**: Monitor and audit regularly
- **Stay Updated**: Keep firmware and security practices current
- **Plan for Incidents**: Have response procedures ready

### Key Takeaways

1. **Never use Open networks** for sensitive data
2. **WPA2 minimum** for all production deployments
3. **WPA3 preferred** on supported hardware
4. **Strong passwords**: 16+ characters with complexity
5. **Rotate passwords** regularly (60-90 days)
6. **Enforce security preferences** for Station mode
7. **Monitor connected clients** and deauthenticate suspicious activity
8. **Physical security** is as important as network security
9. **Clear credentials** before device disposal
10. **Document everything** for compliance and incident response

### Additional Resources

- [Security Configuration Guide](../security/SECURITY_CONFIGURATION.md)
- [AP Config Quick Reference](../reference/AP_CONFIG_QUICK_REF.md)
- [Station Config Quick Reference](../reference/STATION_CONFIG_QUICK_REF.md)
- [Web Configuration Guide](../configuration/WEB_CONFIGURATION.md)
- [WiFi Alliance WPA3 Specification](https://www.wi-fi.org/discover-wi-fi/security)

---

**Stay Secure!**
