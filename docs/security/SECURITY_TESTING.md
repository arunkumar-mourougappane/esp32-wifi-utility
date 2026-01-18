# Security Configuration Testing Suite

**Document Version:** 1.0  
**Last Updated:** 2024  
**Related Issues:** [#38](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/issues/38), [#32](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/issues/32)

## Table of Contents

1. [Overview](#overview)
2. [Test Environment Setup](#test-environment-setup)
3. [Unit Tests](#unit-tests)
4. [Integration Tests](#integration-tests)
5. [Web Interface Tests](#web-interface-tests)
6. [Hardware Compatibility Tests](#hardware-compatibility-tests)
7. [Client Device Tests](#client-device-tests)
8. [Serial Command Tests](#serial-command-tests)
9. [Test Results](#test-results)

---

## Overview

This document provides comprehensive testing procedures for the WiFi Security Configuration features implemented in the ESP32 WiFi Utility. The testing suite covers:

- **Unit Tests**: Individual component validation
- **Integration Tests**: End-to-end security workflows
- **Web Interface Tests**: Browser-based configuration testing
- **Hardware Tests**: Cross-platform ESP32 variant validation
- **Client Device Tests**: QR code scanning and connection verification
- **Serial Command Tests**: CLI command validation

### Prerequisites

- ESP32 development environment configured
- PlatformIO installed
- Multiple ESP32 variants (Classic, S2, S3, C3/C6) for hardware testing
- Client devices (iOS/Android) with QR code scanner apps
- Browser for web interface testing
- Serial terminal application

### Related Documentation

- [Security Configuration Guide](../security/SECURITY_CONFIGURATION.md)
- [Security Best Practices](../security/SECURITY_BEST_PRACTICES.md)
- [AP Configuration Quick Reference](../reference/AP_CONFIG_QUICK_REF.md)
- [Station Configuration Quick Reference](../reference/STATION_CONFIG_QUICK_REF.md)

---

## Test Environment Setup

### Required Hardware

1. **ESP32 Development Boards**
   - ESP32 Classic (ESP-WROOM-32)
   - ESP32-S2 (ESP32-S2-SAOLA-1)
   - ESP32-S3 (Adafruit Feather ESP32-S3 Reverse TFT)
   - ESP32-C3 or ESP32-C6

2. **Client Devices**
   - iOS device (iPhone/iPad) with iOS 14+
   - Android device with Android 10+
   - Windows/Mac/Linux laptop with WiFi

3. **Test Router** (optional)
   - Router with WPA2/WPA3 support for Station mode testing

### Required Software

1. **Development Tools**
   ```bash
   # Install PlatformIO
   pip install platformio
   
   # Clone repository
   git clone https://github.com/arunkumar-mourougappane/esp32-wifi-utility.git
   cd esp32-wifi-utility
   ```

2. **QR Code Scanner Apps**
   - iOS: Native Camera app or "QR Code Reader"
   - Android: Native Camera app or "QR & Barcode Scanner"

3. **Serial Terminal**
   - PlatformIO Serial Monitor
   - Arduino Serial Monitor
   - PuTTY/Screen/Minicom

### Build and Flash

```bash
# Build for specific ESP32 variant
pio run -e esp32dev          # ESP32 Classic
pio run -e esp32-s2          # ESP32-S2
pio run -e esp32-s3          # ESP32-S3
pio run -e esp32-c3          # ESP32-C3

# Flash firmware
pio run -t upload -e <environment>

# Monitor serial output
pio device monitor
```

---

## Unit Tests

### Test 1: APConfig Security Type Validation

**Objective**: Verify APConfig correctly validates and stores security type values.

**Test Procedure**:
```cpp
// Test Case 1.1: Valid security types
APConfig config;
config.security_type = 0;  // Open
ASSERT_TRUE(config.validate());

config.security_type = 1;  // WPA2
ASSERT_TRUE(config.validate());

config.security_type = 2;  // WPA3
ASSERT_TRUE(config.validate());

config.security_type = 3;  // WPA2/WPA3 Mixed
ASSERT_TRUE(config.validate());

// Test Case 1.2: Invalid security type
config.security_type = 4;  // Invalid
ASSERT_FALSE(config.validate());

// Test Case 1.3: Password requirements for secured networks
config.security_type = 1;  // WPA2
config.password = "1234567";  // Too short (< 8 chars)
ASSERT_FALSE(config.validate());

config.password = "ValidPassword123";  // Valid
ASSERT_TRUE(config.validate());

// Test Case 1.4: No password required for Open network
config.security_type = 0;  // Open
config.password = "";
ASSERT_TRUE(config.validate());
```

**Expected Results**:
- Security types 0-3 are accepted
- Invalid security types (>3) are rejected
- Passwords for secured networks must be 8-63 characters
- Open networks don't require passwords

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed

---

### Test 2: StationConfig Security Preference Validation

**Objective**: Verify StationConfig correctly handles security preferences.

**Test Procedure**:
```cpp
// Test Case 2.1: Valid security preferences
StationConfig config;

config.security_preference = "auto";
ASSERT_TRUE(config.validate());

config.security_preference = "wpa3prefer";
ASSERT_TRUE(config.validate());

config.security_preference = "wpa3only";
ASSERT_TRUE(config.validate());

config.security_preference = "wpa2min";
ASSERT_TRUE(config.validate());

config.security_preference = "wpa2only";
ASSERT_TRUE(config.validate());

// Test Case 2.2: Invalid security preference
config.security_preference = "invalid";
ASSERT_FALSE(config.validate());

// Test Case 2.3: Case sensitivity
config.security_preference = "WPA3PREFER";  // Should fail
ASSERT_FALSE(config.validate());
```

**Expected Results**:
- Valid preferences: `auto`, `wpa3prefer`, `wpa3only`, `wpa2min`, `wpa2only`
- Invalid preferences are rejected
- Preferences are case-sensitive (lowercase only)

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed

---

### Test 3: NVS Persistence - Security Configuration

**Objective**: Verify security settings are correctly saved and loaded from NVS.

**Test Procedure**:
```cpp
// Test Case 3.1: Save and load AP security configuration
APConfig original_ap;
original_ap.ssid = "TestAP";
original_ap.security_type = 2;  // WPA3
original_ap.password = "SecurePassword123";
original_ap.save();  // Save to NVS

APConfig loaded_ap;
loaded_ap.load();  // Load from NVS

ASSERT_EQ(original_ap.ssid, loaded_ap.ssid);
ASSERT_EQ(original_ap.security_type, loaded_ap.security_type);
ASSERT_EQ(original_ap.password, loaded_ap.password);

// Test Case 3.2: Save and load Station security preference
StationConfig original_sta;
original_sta.ssid = "TestNetwork";
original_sta.security_preference = "wpa3prefer";
original_sta.save();

StationConfig loaded_sta;
loaded_sta.load();

ASSERT_EQ(original_sta.security_preference, loaded_sta.security_preference);

// Test Case 3.3: NVS namespace isolation
// Ensure AP and Station configs use separate namespaces
ASSERT_NE(original_ap.getNVSNamespace(), original_sta.getNVSNamespace());
```

**Expected Results**:
- AP security settings persist across reboots
- Station security preferences persist across reboots
- No cross-contamination between AP and Station configs

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed

---

### Test 4: QR Code Generation

**Objective**: Verify QR codes are generated with correct WiFi credentials format.

**Test Procedure**:
```cpp
// Test Case 4.1: Open network QR code
APConfig open_config;
open_config.ssid = "OpenNetwork";
open_config.security_type = 0;  // Open
String qr_text = open_config.generateQRCode();
ASSERT_EQ(qr_text, "WIFI:T:nopass;S:OpenNetwork;P:;;");

// Test Case 4.2: WPA2 network QR code
APConfig wpa2_config;
wpa2_config.ssid = "SecureNetwork";
wpa2_config.security_type = 1;  // WPA2
wpa2_config.password = "MyPassword123";
String qr_text_wpa2 = wpa2_config.generateQRCode();
ASSERT_EQ(qr_text_wpa2, "WIFI:T:WPA;S:SecureNetwork;P:MyPassword123;;");

// Test Case 4.3: WPA3 network QR code
APConfig wpa3_config;
wpa3_config.ssid = "WPA3Network";
wpa3_config.security_type = 2;  // WPA3
wpa3_config.password = "SecurePass456";
String qr_text_wpa3 = wpa3_config.generateQRCode();
ASSERT_EQ(qr_text_wpa3, "WIFI:T:WPA;S:WPA3Network;P:SecurePass456;;");

// Test Case 4.4: Special characters in SSID/password
APConfig special_config;
special_config.ssid = "My;Network:Test";
special_config.security_type = 1;
special_config.password = "Pass\\word\"123";
String qr_text_special = special_config.generateQRCode();
// Verify special characters are escaped: ; : \ "
ASSERT_TRUE(qr_text_special.indexOf("\\;") != -1);
ASSERT_TRUE(qr_text_special.indexOf("\\:") != -1);
ASSERT_TRUE(qr_text_special.indexOf("\\\\") != -1);
```

**Expected Results**:
- Open networks: `WIFI:T:nopass;S:<SSID>;P:;;`
- WPA2/WPA3: `WIFI:T:WPA;S:<SSID>;P:<PASSWORD>;;`
- Special characters (`;`, `:`, `\`, `"`) are escaped with backslash

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed

---

### Test 5: Security Validation Logic

**Objective**: Verify security validation enforces password requirements correctly.

**Test Procedure**:
```cpp
// Test Case 5.1: Open network - no password validation
ASSERT_TRUE(validateSecurity(0, ""));           // Open, empty password
ASSERT_TRUE(validateSecurity(0, "anytext"));    // Open, ignored password

// Test Case 5.2: WPA2 - password length validation
ASSERT_FALSE(validateSecurity(1, "short"));     // < 8 chars
ASSERT_TRUE(validateSecurity(1, "12345678"));   // Exactly 8 chars
ASSERT_TRUE(validateSecurity(1, "LongPasswordWithManyCharacters123456789012345678901234567890123")); // < 64 chars
ASSERT_FALSE(validateSecurity(1, "1234567890123456789012345678901234567890123456789012345678901234")); // Exactly 64 chars (too long)

// Test Case 5.3: WPA3 - password length validation
ASSERT_FALSE(validateSecurity(2, "short"));     // < 8 chars
ASSERT_TRUE(validateSecurity(2, "12345678"));   // Exactly 8 chars

// Test Case 5.4: WPA2/WPA3 Mixed - password length validation
ASSERT_FALSE(validateSecurity(3, "short"));
ASSERT_TRUE(validateSecurity(3, "ValidPass123"));

// Test Case 5.5: Password character set validation
ASSERT_TRUE(validateSecurity(1, "abcdefgh"));         // Lowercase
ASSERT_TRUE(validateSecurity(1, "ABCDEFGH"));         // Uppercase
ASSERT_TRUE(validateSecurity(1, "12345678"));         // Numbers
ASSERT_TRUE(validateSecurity(1, "!@#$%^&*"));         // Special chars
ASSERT_TRUE(validateSecurity(1, "Mix3d_P@ss"));       // Mixed
```

**Expected Results**:
- Open networks: Password not validated
- WPA2/WPA3/Mixed: Password must be 8-63 characters (inclusive)
- All printable ASCII characters are allowed in passwords

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed

---

## Integration Tests

### Test 6: AP Mode - Open Network

**Objective**: Verify ESP32 can create an open AP and clients can connect.

**Test Procedure**:

1. **Configure Open AP via Serial**:
   ```
   ap_config ssid OpenTestAP
   ap_config security_type 0
   ap_config save
   ap_config start
   ```

2. **Verify AP Status**:
   ```
   ap_config status
   ```
   - Confirm AP is running
   - Confirm security type is "Open"
   - Note assigned IP (typically 192.168.4.1)

3. **Connect Client Device**:
   - On laptop/phone, scan for WiFi networks
   - Locate "OpenTestAP"
   - Connect (no password required)

4. **Verify Connection**:
   - Check client receives IP address (192.168.4.x range)
   - Ping ESP32 AP: `ping 192.168.4.1`
   - Access web interface: `http://192.168.4.1`

**Expected Results**:
- ✅ AP starts successfully
- ✅ No password prompt on client
- ✅ Client receives DHCP IP address
- ✅ Client can ping ESP32 AP
- ✅ Web interface accessible

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 7: AP Mode - WPA2-PSK Network

**Objective**: Verify ESP32 can create a WPA2-secured AP.

**Test Procedure**:

1. **Configure WPA2 AP via Serial**:
   ```
   ap_config ssid WPA2TestAP
   ap_config security_type 1
   ap_config password SecurePass123
   ap_config save
   ap_config start
   ```

2. **Verify AP Status**:
   ```
   ap_config status
   ```
   - Confirm security type is "WPA2-PSK"

3. **Connect Client Device**:
   - Scan for "WPA2TestAP"
   - Enter password: `SecurePass123`
   - Connect

4. **Verify Security**:
   - Connection should use WPA2 encryption
   - Check client WiFi properties (should show WPA2-Personal/PSK)

5. **Test Wrong Password**:
   - Attempt connection with incorrect password
   - Verify connection is rejected

**Expected Results**:
- ✅ AP starts with WPA2 security
- ✅ Client prompted for password
- ✅ Correct password allows connection
- ✅ Incorrect password rejected
- ✅ Client shows WPA2 encryption

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 8: AP Mode - WPA3-PSK Network

**Objective**: Verify ESP32 can create a WPA3-secured AP (ESP32-S2/S3/C3/C6 only).

**Test Procedure**:

1. **Check Hardware Support**:
   - Confirm ESP32 variant supports WPA3 (S2/S3/C3/C6)
   - ESP32 Classic does NOT support WPA3

2. **Configure WPA3 AP via Serial**:
   ```
   ap_config ssid WPA3TestAP
   ap_config security_type 2
   ap_config password SecureWPA3Pass
   ap_config save
   ap_config start
   ```

3. **Verify AP Status**:
   ```
   ap_config status
   ```
   - Confirm security type is "WPA3-PSK"

4. **Connect WPA3-Capable Client**:
   - Use device with WPA3 support (recent iOS/Android/Windows 10+)
   - Connect to "WPA3TestAP" with password

5. **Verify Security**:
   - Check client WiFi properties (should show WPA3-Personal/SAE)

**Expected Results**:
- ✅ AP starts with WPA3 security (on supported hardware)
- ✅ WPA3-capable clients can connect
- ✅ Enhanced security features (SAE handshake)
- ⚠️ ESP32 Classic: Feature not supported, should fall back or show error

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Hardware**: _______________  
**Notes**: _______________________________________________

---

### Test 9: AP Mode - WPA2/WPA3 Mixed Network

**Objective**: Verify ESP32 can create a mixed-mode AP supporting both WPA2 and WPA3 clients.

**Test Procedure**:

1. **Configure Mixed AP**:
   ```
   ap_config ssid MixedTestAP
   ap_config security_type 3
   ap_config password MixedPassword
   ap_config save
   ap_config start
   ```

2. **Connect WPA3 Client**:
   - Use WPA3-capable device
   - Connect and verify WPA3 encryption used

3. **Connect WPA2-Only Client**:
   - Use older device (WPA2 only)
   - Connect and verify WPA2 encryption used

4. **Verify Both Connected Simultaneously**:
   ```
   ap_config status
   ```
   - Check connected clients count

**Expected Results**:
- ✅ Both WPA2 and WPA3 clients can connect
- ✅ Each client uses its best supported security
- ✅ Clients can communicate through AP

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 10: Station Mode - Auto Security Preference

**Objective**: Verify Station automatically selects best available security.

**Test Procedure**:

1. **Setup Test Router**:
   - Configure router with WPA2/WPA3 mixed mode
   - SSID: "TestRouter"
   - Password: "RouterPassword"

2. **Configure Station - Auto Preference**:
   ```
   sta_config ssid TestRouter
   sta_config password RouterPassword
   sta_config security_preference auto
   sta_config save
   sta_config connect
   ```

3. **Verify Connection**:
   ```
   sta_config status
   ```
   - Note which security was negotiated
   - On WPA3-capable hardware, should prefer WPA3
   - On ESP32 Classic, should use WPA2

4. **Test with WPA2-Only Router**:
   - Reconfigure router to WPA2 only
   - Reconnect Station
   - Verify WPA2 connection successful

**Expected Results**:
- ✅ Connects successfully with `auto` preference
- ✅ WPA3 preferred when available (on supported hardware)
- ✅ Falls back to WPA2 gracefully

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 11: Station Mode - WPA3 Prefer

**Objective**: Verify Station prefers WPA3 but accepts WPA2 fallback.

**Test Procedure**:

1. **Configure Station - WPA3 Prefer**:
   ```
   sta_config ssid TestRouter
   sta_config password RouterPassword
   sta_config security_preference wpa3prefer
   sta_config save
   sta_config connect
   ```

2. **Test with WPA3 Router**:
   - Configure router with WPA3
   - Verify Station connects with WPA3

3. **Test with WPA2-Only Router**:
   - Switch router to WPA2 only
   - Verify Station still connects (falls back to WPA2)

**Expected Results**:
- ✅ Connects to WPA3 when available
- ✅ Falls back to WPA2 when WPA3 unavailable
- ✅ No connection failure on fallback

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 12: Station Mode - WPA3 Only

**Objective**: Verify Station only connects to WPA3 networks.

**Test Procedure**:

1. **Configure Station - WPA3 Only**:
   ```
   sta_config ssid TestRouter
   sta_config password RouterPassword
   sta_config security_preference wpa3only
   sta_config save
   sta_config connect
   ```

2. **Test with WPA3 Router**:
   - Configure router with WPA3
   - Verify connection succeeds

3. **Test with WPA2-Only Router**:
   - Switch router to WPA2 only
   - Verify connection FAILS (security requirement not met)

**Expected Results**:
- ✅ Connects to WPA3 networks
- ✅ REFUSES to connect to WPA2-only networks
- ✅ Clear error message when security insufficient

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 13: Station Mode - WPA2 Minimum

**Objective**: Verify Station requires at least WPA2 (rejects Open/WEP).

**Test Procedure**:

1. **Configure Station - WPA2 Minimum**:
   ```
   sta_config ssid TestRouter
   sta_config password RouterPassword
   sta_config security_preference wpa2min
   sta_config save
   sta_config connect
   ```

2. **Test with WPA3 Router**:
   - Verify connection succeeds

3. **Test with WPA2 Router**:
   - Verify connection succeeds

4. **Test with Open Router**:
   - Configure router as Open network
   - Verify connection FAILS

**Expected Results**:
- ✅ Connects to WPA2 and WPA3
- ✅ REFUSES Open and WEP networks
- ✅ Error message indicates insufficient security

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 14: Station Mode - WPA2 Only

**Objective**: Verify Station only connects to WPA2 networks (not WPA3).

**Test Procedure**:

1. **Configure Station - WPA2 Only**:
   ```
   sta_config ssid TestRouter
   sta_config password RouterPassword
   sta_config security_preference wpa2only
   sta_config save
   sta_config connect
   ```

2. **Test with WPA2 Router**:
   - Verify connection succeeds

3. **Test with WPA3-Only Router**:
   - Configure router with WPA3 only
   - Verify connection behavior (may fail or connect depending on router compatibility)

**Expected Results**:
- ✅ Connects to WPA2 networks
- ⚠️ Behavior with WPA3-only networks: depends on ESP-IDF version

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 15: Configuration Persistence

**Objective**: Verify security settings persist across reboots.

**Test Procedure**:

1. **Configure and Save Settings**:
   ```
   # AP Configuration
   ap_config ssid PersistentAP
   ap_config security_type 2
   ap_config password PersistPass
   ap_config save
   
   # Station Configuration
   sta_config ssid PersistentSTA
   sta_config password STAPass
   sta_config security_preference wpa3prefer
   sta_config save
   ```

2. **Verify Settings Before Reboot**:
   ```
   ap_config status
   sta_config status
   ```
   - Document all settings

3. **Reboot ESP32**:
   - Hard reset (press EN button)
   - Or send reboot command: `reboot`

4. **Verify Settings After Reboot**:
   ```
   ap_config status
   sta_config status
   ```
   - Compare with pre-reboot settings

**Expected Results**:
- ✅ AP SSID, security type, and password persist
- ✅ Station SSID, password, and security preference persist
- ✅ AP auto-starts with saved configuration
- ✅ No settings lost or corrupted

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

## Web Interface Tests

### Test 16: AP Security Dropdown

**Objective**: Verify AP security type dropdown displays correct options and updates configuration.

**Test Procedure**:

1. **Access Web Interface**:
   - Start ESP32 in AP mode
   - Connect to AP
   - Navigate to `http://192.168.4.1`

2. **Locate AP Security Dropdown**:
   - Find "Access Point Configuration" section
   - Locate "Security Type" dropdown

3. **Verify Dropdown Options**:
   - Confirm options: Open, WPA2-PSK, WPA3-PSK, WPA2/WPA3 Mixed
   - Check dropdown values: 0, 1, 2, 3

4. **Test Password Field Behavior**:
   - Select "Open" → Password field should be disabled/hidden
   - Select "WPA2-PSK" → Password field should be enabled/required
   - Select "WPA3-PSK" → Password field should be enabled/required
   - Select "WPA2/WPA3 Mixed" → Password field should be enabled/required

5. **Submit Configuration**:
   - Fill in SSID and password
   - Click "Save" or "Apply"
   - Verify settings are saved and AP restarts

**Expected Results**:
- ✅ All security types displayed in dropdown
- ✅ Password field shows/hides based on selection
- ✅ Configuration saves correctly
- ✅ AP restarts with new settings

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Browser**: _______________  
**Notes**: _______________________________________________

---

### Test 17: Station Security Preference Dropdown

**Objective**: Verify Station security preference dropdown displays correct options.

**Test Procedure**:

1. **Access Station Configuration**:
   - Navigate to Station Configuration section in web UI

2. **Locate Security Preference Dropdown**:
   - Find "Security Preference" dropdown

3. **Verify Dropdown Options**:
   - Confirm options:
     - Auto (Best Available)
     - Prefer WPA3, Allow WPA2
     - WPA3 Only
     - WPA2 Minimum
     - WPA2 Only
   - Check dropdown values: auto, wpa3prefer, wpa3only, wpa2min, wpa2only

4. **Test Descriptions/Tooltips**:
   - Hover over each option
   - Verify helpful descriptions are shown

5. **Submit Configuration**:
   - Fill in SSID, password, and select preference
   - Click "Save" or "Connect"
   - Verify settings are saved

**Expected Results**:
- ✅ All 5 security preferences displayed
- ✅ Helpful descriptions for each option
- ✅ Configuration saves correctly

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Browser**: _______________  
**Notes**: _______________________________________________

---

### Test 18: Form Validation

**Objective**: Verify web form validates security configuration inputs.

**Test Procedure**:

1. **Test AP Password Validation**:
   - Select WPA2 security
   - Enter short password (< 8 chars): `short`
   - Attempt to submit
   - Verify error message: "Password must be 8-63 characters"

2. **Test Empty Password for Secured Network**:
   - Select WPA2 security
   - Leave password blank
   - Attempt to submit
   - Verify error message

3. **Test Long Password**:
   - Enter 64+ character password
   - Verify error message or truncation

4. **Test Open Network - No Password**:
   - Select Open security
   - Leave password blank
   - Verify submission succeeds (no error)

5. **Test SSID Validation**:
   - Leave SSID blank
   - Verify error: "SSID is required"
   - Enter SSID > 32 characters
   - Verify error or truncation

**Expected Results**:
- ✅ Password length validated (8-63 chars for secured)
- ✅ SSID required and length validated
- ✅ Clear error messages displayed
- ✅ Form prevents invalid submissions

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 19: Mobile Responsiveness

**Objective**: Verify security configuration forms work on mobile devices.

**Test Procedure**:

1. **Test on iOS Device**:
   - Connect to ESP32 AP from iPhone/iPad
   - Access web interface
   - Navigate to AP/Station configuration
   - Test dropdowns, text inputs, buttons

2. **Test on Android Device**:
   - Repeat tests on Android device

3. **Test Different Screen Sizes**:
   - Portrait orientation
   - Landscape orientation
   - Different device sizes (phone/tablet)

4. **Verify Usability**:
   - Dropdowns are easy to tap
   - Text is readable
   - Buttons are appropriately sized
   - No horizontal scrolling required

**Expected Results**:
- ✅ All form elements accessible on mobile
- ✅ Dropdowns work correctly
- ✅ Text is readable without zooming
- ✅ Forms submit successfully

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Device**: _______________  
**Notes**: _______________________________________________

---

## Hardware Compatibility Tests

### Test 20: ESP32 Classic (ESP-WROOM-32)

**Objective**: Verify security features on ESP32 Classic hardware.

**Test Procedure**:

1. **Flash Firmware**:
   ```bash
   pio run -t upload -e esp32dev
   ```

2. **Test AP Mode Security Types**:
   - Open: ⬜ Test
   - WPA2-PSK: ⬜ Test
   - WPA3-PSK: ⬜ Expected to fail (not supported)
   - WPA2/WPA3 Mixed: ⬜ Expected to fall back to WPA2

3. **Test Station Mode Security Preferences**:
   - auto: ⬜ Test (should use WPA2)
   - wpa3prefer: ⬜ Test (should fall back to WPA2)
   - wpa3only: ⬜ Test (should fail on WPA2-only networks)
   - wpa2min: ⬜ Test
   - wpa2only: ⬜ Test

4. **Test QR Code Generation**:
   - Verify QR codes display correctly on TFT (if available)

**Expected Results**:
- ✅ Open and WPA2 fully supported
- ⚠️ WPA3 NOT supported (ESP32 Classic limitation)
- ✅ WPA2 security preferences work correctly
- ✅ Clear error messages when WPA3 requested on unsupported hardware

**Test Results**:
| Feature | Status | Notes |
|---------|--------|-------|
| AP - Open | ⬜ | |
| AP - WPA2 | ⬜ | |
| AP - WPA3 | ⬜ | |
| STA - Auto | ⬜ | |
| STA - WPA3Prefer | ⬜ | |
| QR Code | ⬜ | |

**Overall Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Firmware Version**: _______________

---

### Test 21: ESP32-S2

**Objective**: Verify security features on ESP32-S2 hardware (WPA3 supported).

**Test Procedure**:

1. **Flash Firmware**:
   ```bash
   pio run -t upload -e esp32-s2
   ```

2. **Test AP Mode Security Types**:
   - Open: ⬜ Test
   - WPA2-PSK: ⬜ Test
   - WPA3-PSK: ⬜ Test (should work)
   - WPA2/WPA3 Mixed: ⬜ Test (should work)

3. **Test Station Mode Security Preferences**:
   - auto: ⬜ Test (should prefer WPA3)
   - wpa3prefer: ⬜ Test
   - wpa3only: ⬜ Test
   - wpa2min: ⬜ Test
   - wpa2only: ⬜ Test

4. **Test WPA3-Specific Features**:
   - PMF (Protected Management Frames) enabled
   - SAE handshake

**Expected Results**:
- ✅ All security types supported (Open, WPA2, WPA3, Mixed)
- ✅ WPA3 connects successfully to WPA3 routers
- ✅ Enhanced security features work

**Test Results**:
| Feature | Status | Notes |
|---------|--------|-------|
| AP - Open | ⬜ | |
| AP - WPA2 | ⬜ | |
| AP - WPA3 | ⬜ | |
| AP - Mixed | ⬜ | |
| STA - WPA3Only | ⬜ | |
| PMF Enabled | ⬜ | |

**Overall Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Firmware Version**: _______________

---

### Test 22: ESP32-S3

**Objective**: Verify security features on ESP32-S3 hardware (WPA3 supported).

**Test Procedure**:

1. **Flash Firmware**:
   ```bash
   pio run -t upload -e adafruit_feather_esp32s3_reversetft
   ```

2. **Test AP Mode Security Types**:
   - All types (Open, WPA2, WPA3, Mixed)

3. **Test Station Mode**:
   - All security preferences

4. **Test with TFT Display**:
   - Verify QR codes render correctly
   - Test security indicators on display

**Expected Results**:
- ✅ Full WPA3 support
- ✅ TFT display shows QR codes correctly
- ✅ Security status displayed on screen

**Test Results**:
| Feature | Status | Notes |
|---------|--------|-------|
| AP - All Types | ⬜ | |
| STA - All Prefs | ⬜ | |
| TFT QR Code | ⬜ | |
| TFT Security UI | ⬜ | |

**Overall Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Firmware Version**: _______________

---

### Test 23: ESP32-C3 / ESP32-C6

**Objective**: Verify security features on ESP32-C3/C6 hardware (WPA3 supported).

**Test Procedure**:

1. **Flash Firmware**:
   ```bash
   pio run -t upload -e esp32-c3
   # or
   pio run -t upload -e esp32-c6
   ```

2. **Test All Security Features**:
   - AP: Open, WPA2, WPA3, Mixed
   - Station: All security preferences

3. **Performance Testing**:
   - Measure connection time for WPA3 vs WPA2
   - Test throughput with iperf

**Expected Results**:
- ✅ Full WPA3 support
- ✅ Good performance with WPA3
- ✅ All features work as expected

**Test Results**:
| Feature | Status | Notes |
|---------|--------|-------|
| AP - All Types | ⬜ | |
| STA - All Prefs | ⬜ | |
| WPA3 Performance | ⬜ | |

**Overall Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Chip**: C3 ⬜ / C6 ⬜  
**Firmware Version**: _______________

---

## Client Device Tests

### Test 24: QR Code Scanning - iOS

**Objective**: Verify iOS devices can scan QR codes and connect to AP.

**Test Procedure**:

1. **Generate QR Code**:
   - Configure AP with WPA2 security
   - Start AP
   - Display QR code on TFT or web interface

2. **Scan with iOS Camera App**:
   - Open native Camera app
   - Point at QR code
   - Tap notification banner

3. **Verify Connection**:
   - iOS should prompt: "Join WiFi Network <SSID>?"
   - Tap "Join"
   - Verify connection succeeds

4. **Test Different Security Types**:
   - Open network QR code
   - WPA2 QR code
   - WPA3 QR code (on iOS 13+)

**Expected Results**:
- ✅ iOS recognizes QR code
- ✅ Prompts to join network
- ✅ Connection succeeds without manual password entry

**Test Results**:
| Security Type | iOS Version | Status | Notes |
|---------------|-------------|--------|-------|
| Open | ___ | ⬜ | |
| WPA2 | ___ | ⬜ | |
| WPA3 | ___ | ⬜ | |

**Overall Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**iOS Version**: _______________

---

### Test 25: QR Code Scanning - Android

**Objective**: Verify Android devices can scan QR codes and connect to AP.

**Test Procedure**:

1. **Generate QR Code**:
   - Configure AP
   - Display QR code

2. **Scan with Android Camera App**:
   - Open native Camera app (Android 10+) or third-party QR app
   - Scan QR code
   - Tap notification

3. **Verify Connection**:
   - Android should prompt to join network
   - Confirm connection

4. **Test Different Security Types**:
   - Open, WPA2, WPA3

**Expected Results**:
- ✅ Android recognizes QR code
- ✅ Prompts to join network
- ✅ Connection succeeds

**Test Results**:
| Security Type | Android Version | Status | Notes |
|---------------|-----------------|--------|-------|
| Open | ___ | ⬜ | |
| WPA2 | ___ | ⬜ | |
| WPA3 | ___ | ⬜ | |

**Overall Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Android Version**: _______________

---

### Test 26: Client Connection Compatibility

**Objective**: Verify various client devices can connect to different security types.

**Test Procedure**:

1. **Setup Test Matrix**:
   - Test devices: Windows 10/11, macOS, Linux, iOS, Android
   - Security types: Open, WPA2, WPA3, Mixed

2. **Test Each Combination**:
   - Configure AP with security type
   - Attempt connection from each device
   - Document success/failure

**Test Matrix**:

| Device | Open | WPA2 | WPA3 | Mixed |
|--------|------|------|------|-------|
| Windows 10 | ⬜ | ⬜ | ⬜ | ⬜ |
| Windows 11 | ⬜ | ⬜ | ⬜ | ⬜ |
| macOS | ⬜ | ⬜ | ⬜ | ⬜ |
| Linux | ⬜ | ⬜ | ⬜ | ⬜ |
| iOS | ⬜ | ⬜ | ⬜ | ⬜ |
| Android | ⬜ | ⬜ | ⬜ | ⬜ |

**Expected Results**:
- ✅ Modern devices connect to all types
- ⚠️ Older devices may not support WPA3

**Notes**: _______________________________________________

---

## Serial Command Tests

### Test 27: AP Security Commands

**Objective**: Verify all AP security serial commands work correctly.

**Test Procedure**:

1. **Test `ap_config security_type` Command**:
   ```
   # Set Open
   ap_config security_type 0
   ap_config status
   # Verify: Security: Open
   
   # Set WPA2
   ap_config security_type 1
   ap_config status
   # Verify: Security: WPA2-PSK
   
   # Set WPA3
   ap_config security_type 2
   ap_config status
   # Verify: Security: WPA3-PSK
   
   # Set Mixed
   ap_config security_type 3
   ap_config status
   # Verify: Security: WPA2/WPA3 Mixed
   
   # Test invalid value
   ap_config security_type 99
   # Verify: Error message displayed
   ```

2. **Test `ap_config password` Command**:
   ```
   # Set valid password
   ap_config password ValidPass123
   ap_config status
   # Verify: Password set (not displayed for security)
   
   # Test short password (should fail for secured networks)
   ap_config security_type 1
   ap_config password short
   # Verify: Error message
   
   # Test long password
   ap_config password 1234567890123456789012345678901234567890123456789012345678901234567890
   # Verify: Error or truncation
   ```

3. **Test Combined Workflow**:
   ```
   ap_config ssid TestAP
   ap_config security_type 1
   ap_config password SecurePass
   ap_config save
   ap_config start
   # Verify: AP starts with correct settings
   ```

**Expected Results**:
- ✅ All commands execute correctly
- ✅ Invalid inputs rejected with clear errors
- ✅ Settings saved and applied correctly

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 28: Station Security Commands

**Objective**: Verify all Station security serial commands work correctly.

**Test Procedure**:

1. **Test `sta_config security_preference` Command**:
   ```
   # Test each preference
   sta_config security_preference auto
   sta_config status
   # Verify: Security Preference: auto
   
   sta_config security_preference wpa3prefer
   sta_config status
   # Verify: Security Preference: wpa3prefer
   
   sta_config security_preference wpa3only
   sta_config status
   # Verify: Security Preference: wpa3only
   
   sta_config security_preference wpa2min
   sta_config status
   # Verify: Security Preference: wpa2min
   
   sta_config security_preference wpa2only
   sta_config status
   # Verify: Security Preference: wpa2only
   
   # Test invalid preference
   sta_config security_preference invalid
   # Verify: Error message
   ```

2. **Test Case Sensitivity**:
   ```
   # Should fail (uppercase)
   sta_config security_preference WPA3PREFER
   # Verify: Error or not accepted
   ```

3. **Test Combined Workflow**:
   ```
   sta_config ssid TestRouter
   sta_config password RouterPass
   sta_config security_preference wpa3prefer
   sta_config save
   sta_config connect
   # Verify: Connects with correct preference
   ```

**Expected Results**:
- ✅ All preferences accepted
- ✅ Case-sensitive validation works
- ✅ Invalid inputs rejected

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

### Test 29: Help and Documentation Commands

**Objective**: Verify help commands display security-related information.

**Test Procedure**:

1. **Test `ap_config help`**:
   ```
   ap_config help
   ```
   - Verify `security_type` parameter documented
   - Verify valid values (0-3) listed
   - Verify descriptions: Open, WPA2-PSK, WPA3-PSK, Mixed

2. **Test `sta_config help`**:
   ```
   sta_config help
   ```
   - Verify `security_preference` parameter documented
   - Verify valid values listed: auto, wpa3prefer, wpa3only, wpa2min, wpa2only
   - Verify descriptions for each preference

3. **Test Invalid Commands**:
   ```
   ap_config security_type
   # Verify: Shows help or error about missing value
   
   sta_config security_preference
   # Verify: Shows help or error
   ```

**Expected Results**:
- ✅ Help text includes security parameters
- ✅ Clear descriptions for each option
- ✅ Examples provided

**Status**: ⬜ Not Tested | ✅ Passed | ❌ Failed  
**Notes**: _______________________________________________

---

## Test Results

### Summary Table

| Test # | Test Name | Status | Priority | Notes |
|--------|-----------|--------|----------|-------|
| 1 | APConfig Security Type Validation | ⬜ | High | Unit test |
| 2 | StationConfig Security Preference Validation | ⬜ | High | Unit test |
| 3 | NVS Persistence | ⬜ | High | Unit test |
| 4 | QR Code Generation | ⬜ | High | Unit test |
| 5 | Security Validation Logic | ⬜ | High | Unit test |
| 6 | AP - Open Network | ⬜ | High | Integration |
| 7 | AP - WPA2-PSK | ⬜ | High | Integration |
| 8 | AP - WPA3-PSK | ⬜ | Medium | Integration |
| 9 | AP - Mixed Mode | ⬜ | Medium | Integration |
| 10 | Station - Auto | ⬜ | High | Integration |
| 11 | Station - WPA3 Prefer | ⬜ | Medium | Integration |
| 12 | Station - WPA3 Only | ⬜ | Medium | Integration |
| 13 | Station - WPA2 Min | ⬜ | Medium | Integration |
| 14 | Station - WPA2 Only | ⬜ | Low | Integration |
| 15 | Config Persistence | ⬜ | High | Integration |
| 16 | Web - AP Dropdown | ⬜ | High | Web UI |
| 17 | Web - Station Dropdown | ⬜ | High | Web UI |
| 18 | Web - Form Validation | ⬜ | High | Web UI |
| 19 | Web - Mobile Responsive | ⬜ | Medium | Web UI |
| 20 | Hardware - ESP32 Classic | ⬜ | High | HW Compat |
| 21 | Hardware - ESP32-S2 | ⬜ | High | HW Compat |
| 22 | Hardware - ESP32-S3 | ⬜ | High | HW Compat |
| 23 | Hardware - ESP32-C3/C6 | ⬜ | Medium | HW Compat |
| 24 | QR Scan - iOS | ⬜ | High | Client |
| 25 | QR Scan - Android | ⬜ | High | Client |
| 26 | Client Compatibility | ⬜ | Medium | Client |
| 27 | Serial - AP Commands | ⬜ | High | Serial |
| 28 | Serial - Station Commands | ⬜ | High | Serial |
| 29 | Serial - Help Commands | ⬜ | Low | Serial |

**Legend**:
- ⬜ Not Tested
- ✅ Passed
- ❌ Failed
- ⚠️ Partial/Conditional

### Test Environment

| Item | Details |
|------|---------|
| Firmware Version | v5.2.1 |
| ESP-IDF Version | _______________ |
| Test Date | _______________ |
| Tester | _______________ |

### Hardware Used

| Device | Model | Firmware | Status |
|--------|-------|----------|--------|
| ESP32 Classic | _______________ | _______________ | ⬜ |
| ESP32-S2 | _______________ | _______________ | ⬜ |
| ESP32-S3 | Adafruit Feather S3 Reverse TFT | _______________ | ⬜ |
| ESP32-C3 | _______________ | _______________ | ⬜ |
| ESP32-C6 | _______________ | _______________ | ⬜ |

### Client Devices Used

| Device | OS Version | WiFi Chip | WPA3 Support |
|--------|------------|-----------|--------------|
| _______________ | _______________ | _______________ | Yes ⬜ / No ⬜ |
| _______________ | _______________ | _______________ | Yes ⬜ / No ⬜ |
| _______________ | _______________ | _______________ | Yes ⬜ / No ⬜ |

---

## Known Issues and Limitations

### ESP32 Classic Limitations
- **No WPA3 Support**: ESP32 Classic (ESP-WROOM-32) does not support WPA3 due to hardware limitations
- **Fallback Behavior**: When WPA3 is requested on ESP32 Classic, the system should either:
  - Display clear error message
  - Automatically fall back to WPA2 (if Mixed mode selected)

### ESP-IDF Version Dependencies
- **WPA3 Requires ESP-IDF 4.3+**: Older ESP-IDF versions may not support WPA3
- **PMF Support**: Protected Management Frames (required for WPA3) need ESP-IDF 4.0+

### Client Device Compatibility
- **WPA3 Client Support**:
  - iOS 13+ supports WPA3
  - Android 10+ supports WPA3
  - Windows 10 May 2019 Update+ supports WPA3
  - macOS 10.15+ (Catalina) supports WPA3
  - Older devices will only connect in WPA2 mode

### QR Code Limitations
- **Format Standard**: Uses standard WiFi QR code format (`WIFI:T:...`)
- **Special Characters**: Some special characters in SSID/password need escaping
- **Display Size**: QR codes may be difficult to scan on small TFT displays

### Performance Considerations
- **WPA3 Connection Time**: WPA3 may take slightly longer to connect than WPA2 due to SAE handshake
- **Memory Usage**: WPA3 requires slightly more RAM than WPA2

---

## Appendix: Test Automation

### Automated Unit Tests (Future Work)

For automated testing, consider implementing tests using the PlatformIO test framework:

```cpp
// test/test_security_validation.cpp
#include <unity.h>
#include "APConfig.h"
#include "StationConfig.h"

void test_ap_security_types() {
    APConfig config;
    
    config.security_type = 0;
    TEST_ASSERT_TRUE(config.validate());
    
    config.security_type = 4;
    TEST_ASSERT_FALSE(config.validate());
}

void test_station_preferences() {
    StationConfig config;
    
    config.security_preference = "auto";
    TEST_ASSERT_TRUE(config.validate());
    
    config.security_preference = "invalid";
    TEST_ASSERT_FALSE(config.validate());
}

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_ap_security_types);
    RUN_TEST(test_station_preferences);
    return UNITY_END();
}
```

Run tests with:
```bash
pio test -e native
```

---

## Conclusion

This testing suite provides comprehensive validation of the WiFi Security Configuration features. Complete all tests before releasing updates to ensure security and reliability.

For questions or issues, refer to:
- [Security Configuration Guide](../security/SECURITY_CONFIGURATION.md)
- [Project Issue Tracker](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/issues)
- [Documentation Index](DOCUMENTATION_INDEX.md)

**Document Maintenance**: This document should be updated whenever security features are modified or new tests are added.
