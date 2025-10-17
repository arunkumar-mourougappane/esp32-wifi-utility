# Test Suite Documentation

## Overview

This document describes the test suite for the ESP32 WiFi Utility project, including tests for the clickable Access Point details view feature (Issue #10).

## Test Environment

- **Framework**: Unity Test Framework
- **Boards**:
  - ESP32 DevKit (test)
  - Adafruit Feather ESP32-S3 TFT (test_feather)
- **Test Runner**: PlatformIO

## Running Tests

### Run all tests on ESP32

```bash
pio test --environment test
```

### Run all tests on Feather ESP32-S3

```bash
pio test --environment test_feather
```

### Build tests without hardware upload

```bash
pio test --environment test --without-uploading
```

## Test Coverage

### Basic System Validation (6 tests)

1. **test_basic_system_validation**

   - Verifies basic test framework functionality
   - Validates arithmetic operations

2. **test_config_constants**

   - Checks test configuration constants
   - Validates timeout and heap requirements

3. **test_board_identification**

   - Identifies correct board type (ESP32 vs Feather)
   - Validates NeoPixel availability based on board

4. **test_helper_functions**

   - Tests range checking utilities
   - Validates channel validation functions
   - Checks recommended channel identification

5. **test_mock_calculations**

   - Tests mock congestion score calculation
   - Validates capping behavior at 100%

6. **test_heap_memory_availability**
   - Verifies sufficient heap memory for operations
   - Checks against MIN_FREE_HEAP threshold

### WiFi and Network Tests (5 tests)

7. **test_wifi_scan_constants**

   - Validates WiFi scan timeout ranges
   - Ensures reasonable timeout values

8. **test_network_security_levels**

   - Verifies encryption type distinctions
   - Tests WPA/WPA2/WPA3 differentiation

9. **test_rssi_range_validation**

   - Validates RSSI values are in typical WiFi range (-120 to 0 dBm)
   - Tests multiple signal strength values

10. **test_2_4ghz_channel_range**

    - Validates 2.4GHz channel numbers (1-14)
    - Tests recommended non-overlapping channels (1, 6, 11)

11. **test_bssid_mac_format**
    - Validates MAC address (BSSID) structure
    - Checks 6-byte format and valid byte ranges

### Scan Details Cache Tests (8 tests)

12. **test_cache_valid_with_recent_scan**

    - Tests cache validity with recent scan results
    - Validates cache timeout mechanism

13. **test_cache_invalid_when_empty**

    - Verifies cache is invalid when empty
    - Tests initial state handling

14. **test_cache_handles_max_networks**

    - Tests cache capacity limit (50 networks)
    - Validates overflow handling

15. **test_cache_network_data_integrity**

    - Validates cached network data structure
    - Tests SSID, RSSI, and channel storage

16. **test_network_signal_quality_levels**

    - Tests signal quality categorization (Excellent to Very Weak)
    - Validates RSSI-to-quality mapping:
      - \>= -50 dBm: Excellent
      - \>= -60 dBm: Very Good
      - \>= -70 dBm: Good
      - \>= -75 dBm: Fair
      - \>= -80 dBm: Weak
      - < -80 dBm: Very Weak

17. **test_encryption_type_names**

    - Validates encryption type distinctions
    - Tests OPEN, WPA-PSK, WPA2-PSK, WPA3-PSK

18. **test_channel_frequency_conversion**

    - Tests channel to frequency mapping (2.4GHz)
    - Validates formula: freq = 2407 + (channel × 5)
    - Special case: Channel 14 = 2484 MHz

19. **test_network_id_validation**
    - Tests valid network ID range (0 to count-1)
    - Validates rejection of negative and out-of-range IDs

## Test Results

### Latest Test Run

**ESP32 DevKit (test):**

- ✅ 19 test cases: 19 succeeded
- ⏱️ Duration: ~14 seconds

**Feather ESP32-S3 TFT (test_feather):**

- ✅ 19 test cases: 19 succeeded
- ⏱️ Duration: ~5.5 seconds (without upload)

## Feature-Specific Tests (Issue #10)

The following tests specifically validate the clickable Access Point details view feature:

### Cache Management

- Cache validity with timeout (5 minutes)
- Empty cache detection
- Maximum network capacity (50 networks)
- Data integrity (SSID, RSSI, channel, encryption, BSSID)

### Network Details Display

- Signal quality calculation and categorization
- Encryption type identification
- Channel to frequency conversion
- Network ID validation for safe array access

### Error Handling

- Invalid network ID detection
- Cache expiration handling
- Missing BSSID handling (some networks don't provide BSSID)

## Mock Data Structure

```cpp
struct MockCachedScanResult {
    String ssid;
    int32_t rssi;
    uint8_t channel;
    wifi_auth_mode_t encryptionType;
    uint8_t bssid[6];
    bool hasBssid;
};
```

## Configuration Constants

- `MAX_CACHED_NETWORKS_TEST`: 50
- `CACHE_TIMEOUT_MS_TEST`: 300000 (5 minutes)
- `MIN_FREE_HEAP`: 20000 bytes
- `TEST_CHANNEL_COUNT`: 14 (2.4GHz channels)

## Memory Considerations

Tests validate that:

- Free heap remains above MIN_FREE_HEAP (20KB)
- Cache structure size is reasonable (<200 bytes per network)
- Total cache footprint is acceptable (<10KB)

## Future Test Enhancements

Potential additions for future development:

1. **Integration Tests**

   - Full web server request/response cycle
   - Route handler validation
   - HTML generation verification

2. **Performance Tests**

   - Cache access time
   - HTML generation speed
   - Memory allocation patterns

3. **Edge Cases**

   - Hidden network handling (empty SSID)
   - Very weak signals (< -100 dBm)
   - Channel 14 special case (Japan)
   - Enterprise encryption types

4. **Concurrency Tests**
   - Multiple simultaneous cache accesses
   - Cache refresh during access
   - Race condition prevention

## Continuous Integration

These tests are designed to run in CI/CD pipelines:

```yaml
# Example CI configuration
script:
  - pio test --environment test --without-uploading
  - pio test --environment test_feather --without-uploading
```

## Troubleshooting

### Tests Fail to Build

- Ensure all dependencies are installed: `pio lib install`
- Clean build: `pio test --environment test -c`

### Tests Timeout

- Increase test timeout in platformio.ini
- Check serial connection (9600 baud)
- Ensure board is properly connected

### Tests Fail on Hardware

- Verify board is powered correctly
- Check USB connection and permissions
- Try manual board reset before test

## Contributing

When adding new features:

1. Add corresponding test cases
2. Ensure all tests pass on both boards
3. Update this documentation
4. Maintain minimum 80% code coverage

## Contact

For questions or issues with tests, please refer to the project's issue tracker or documentation.
