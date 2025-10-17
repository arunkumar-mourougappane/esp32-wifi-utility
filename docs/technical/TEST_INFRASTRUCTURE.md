# ESP32 WiFi Utility - Test Infrastructure

## Overview

This document describes the comprehensive test infrastructure for the ESP32 WiFi Utility project, supporting both standard ESP32 development boards and Adafruit Feather ESP32-S3 TFT boards.

## Test Environments

### 1. Standard ESP32 Environment (`test`)
- **Target Board**: ESP32 Development Board
- **Test File**: `test/test_main.cpp`
- **Features Tested**:
  - Basic WiFi functionality
  - Command interface parsing
  - Built-in LED control
  - Memory usage optimization
  - Performance benchmarks
  - Module integration

### 2. Feather ESP32-S3 Environment (`test_feather`)
- **Target Board**: Adafruit Feather ESP32-S3 TFT
- **Test Files**: 
  - `test/test_feather_neopixel.cpp`
  - `test/test_channel_analyzer_comprehensive.cpp`
- **Special Features**:
  - NeoPixel LED integration
  - TFT display support (future)
  - Hardware-specific functionality
  - Conditional compilation with `USE_NEOPIXEL=1`

## Test Categories

### Core Functionality Tests
- **WiFi Manager**: Network scanning, connection management
- **Access Point Manager**: AP configuration, client handling
- **Command Interface**: Command parsing, execution, help system
- **LED Controller**: Visual feedback, status indication

### Advanced Feature Tests
- **Channel Analyzer**: Spectrum analysis, congestion detection, recommendations
- **Performance Analysis**: Latency testing, throughput measurement
- **Memory Management**: Heap usage, buffer management
- **Error Handling**: Graceful failure recovery

### Hardware-Specific Tests
- **NeoPixel Integration** (Feather only): Color control, patterns, brightness
- **Built-in LED** (Standard ESP32): State indication, patterns
- **Board Detection**: Automatic feature detection

## Running Tests

### Quick Test Execution
```bash
# Run all tests automatically
./run_tests.sh

# Test specific environment
pio test -e test          # Standard ESP32
pio test -e test_feather  # Feather ESP32-S3
```

### Individual Test Categories
```bash
# Build verification
pio run -e esp32dev
pio run -e adafruit_feather_esp32s3_tft

# Specific test files
pio test -e test -f test_main
pio test -e test_feather -f test_feather_neopixel
pio test -e test_feather -f test_channel_analyzer_comprehensive
```

## Test Configuration

### Environment Variables
- `USE_NEOPIXEL=1`: Enables NeoPixel-specific code and tests
- `VERSION="4.0.0"`: Sets version information for tests

### Test Constants (test_config.h)
```cpp
#define TEST_TIMEOUT_MS 5000              // General test timeout
#define MAX_CHANNEL_SCAN_TIME_MS 5000     // Channel analysis timeout  
#define MIN_RECOMMENDED_FREE_HEAP 50000   // Memory threshold
#define ACCEPTABLE_CONGESTION_THRESHOLD 50.0  // Channel congestion limit
```

## Test File Structure

### test_main.cpp (Standard ESP32)
- System initialization tests
- Basic WiFi and AP functionality
- Command interface validation
- Performance benchmarks
- Memory usage verification
- Module integration tests

### test_feather_neopixel.cpp (Feather Board)
- NeoPixel initialization and control
- Color setting and brightness
- Pattern generation
- Board-specific hardware tests
- Conditional compilation validation

### test_channel_analyzer_comprehensive.cpp (Both Boards)
- Channel frequency calculations
- Congestion score algorithms
- Overlap detection logic
- Interference pattern recognition
- JSON export format validation
- Performance optimization tests

## Mock Testing Strategy

Since hardware testing can be unreliable in automated environments, the test suite uses intelligent mocking:

### Mock Data Sources
- **Network Lists**: Simulated WiFi networks with varying signal strengths
- **Channel Data**: Predefined congestion patterns and interference scenarios
- **Hardware States**: Simulated sensor readings and device responses
- **Timing**: Controlled execution times for performance validation

### Validation Approaches
- **Algorithm Testing**: Mathematical functions tested with known inputs/outputs
- **State Management**: Proper state transitions and error conditions
- **Resource Usage**: Memory allocation patterns and cleanup verification
- **Integration**: Module interaction patterns and data flow validation

## Continuous Integration

### Build Matrix
The test infrastructure supports multiple build configurations:

1. **esp32dev + test**: Standard ESP32 with full test suite
2. **adafruit_feather_esp32s3_tft + test_feather**: Feather with NeoPixel tests
3. **Cross-compilation**: Ensures code compatibility across platforms

### Test Reports
- **Unity Test Framework**: Structured test output with pass/fail statistics
- **Memory Usage**: Heap utilization reports
- **Performance Metrics**: Execution time measurements
- **Coverage Analysis**: Code coverage reporting (when enabled)

## Debugging Test Failures

### Common Issues
1. **Memory Constraints**: Tests failing due to insufficient heap
   - Solution: Reduce test data size or optimize algorithms

2. **Timing Sensitivity**: Tests failing due to execution speed variations
   - Solution: Increase timeout values or use relative timing

3. **Hardware Dependencies**: Tests expecting specific hardware responses
   - Solution: Enhanced mocking or conditional test execution

4. **Compilation Errors**: Board-specific code not compiling
   - Solution: Proper conditional compilation guards

### Debug Commands
```bash
# Verbose test output
pio test -e test -v

# Monitor serial output during tests
pio device monitor

# Clean build before testing
pio run -t clean
pio test -e test
```

## Adding New Tests

### Test Development Guidelines

1. **Use Descriptive Names**: Test functions should clearly indicate what they test
2. **Mock External Dependencies**: Avoid relying on network connectivity or specific hardware states
3. **Test Edge Cases**: Include boundary conditions and error scenarios
4. **Validate Assumptions**: Test both positive and negative cases
5. **Keep Tests Fast**: Individual tests should complete within seconds

### Example Test Structure
```cpp
void test_new_feature_functionality(void) {
    // Arrange: Set up test data and conditions
    MockData testData = createMockData();
    
    // Act: Execute the functionality being tested
    Result result = executeNewFeature(testData);
    
    // Assert: Verify expected outcomes
    TEST_ASSERT_EQUAL(EXPECTED_VALUE, result.value);
    TEST_ASSERT_TRUE(result.success);
}

#ifdef USE_NEOPIXEL
void test_neopixel_specific_feature(void) {
    // Board-specific test implementation
    TEST_ASSERT_TRUE(neopixel_feature_works());
}
#endif
```

## Performance Benchmarks

### Expected Performance Metrics
- **Channel Scan**: < 5 seconds for complete 2.4GHz spectrum analysis
- **Command Processing**: < 500ms for command parsing and execution
- **Memory Usage**: < 50% of available heap during normal operation
- **WiFi Scan**: < 3 seconds for network discovery

### Benchmark Validation
Tests automatically validate that the system meets performance requirements and will fail if benchmarks are not achieved.

## Future Enhancements

### Planned Test Additions
1. **TFT Display Testing**: Visual output validation for Feather board
2. **Network Integration Tests**: Real WiFi network interaction (optional)
3. **Stress Testing**: Long-running stability tests
4. **Power Consumption**: Energy usage measurement and optimization
5. **Real-time Performance**: Interrupt handling and timing precision tests

### Test Infrastructure Improvements
- **Automated Hardware-in-Loop**: Physical device testing automation
- **Test Data Generation**: Dynamic test case creation
- **Performance Regression**: Automated performance tracking over time
- **Cross-Platform Validation**: Testing across different ESP32 variants