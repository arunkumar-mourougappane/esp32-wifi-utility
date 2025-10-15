#!/bin/bash

# ESP32 WiFi Utility Test Runner
# Tests both ESP32 and Feather board configurations

echo "=================================================="
echo "ESP32 WiFi Utility - Comprehensive Test Suite"
echo "=================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    print_error "PlatformIO is not installed or not in PATH"
    exit 1
fi

print_status "PlatformIO found, proceeding with tests..."

# Test results tracking
tests_passed=0
tests_failed=0

# Function to run test environment
run_test_env() {
    local env_name=$1
    local description=$2
    
    print_status "Running tests for $description..."
    echo "Environment: $env_name"
    
    if pio test -e "$env_name"; then
        print_success "$description tests PASSED"
        ((tests_passed++))
    else
        print_error "$description tests FAILED"
        ((tests_failed++))
    fi
    
    echo ""
}

# Function to build environment without running tests
build_env() {
    local env_name=$1
    local description=$2
    
    print_status "Building $description environment..."
    
    if pio run -e "$env_name"; then
        print_success "$description build PASSED"
        ((tests_passed++))
    else
        print_error "$description build FAILED"
        ((tests_failed++))
    fi
    
    echo ""
}

echo "Starting test execution..."
echo ""

# 1. Build main environments first to ensure code compiles
print_status "Phase 1: Build Verification"
echo "================================"

build_env "esp32dev" "ESP32 Development Board"
build_env "adafruit_feather_esp32s3_tft" "Adafruit Feather ESP32-S3 TFT"

# 2. Run comprehensive test suites
print_status "Phase 2: Unit Testing"
echo "============================="

# Test standard ESP32 environment
run_test_env "test" "Standard ESP32"

# Test Feather board environment
run_test_env "test_feather" "Feather ESP32-S3 TFT"

# 3. Summary
echo "=================================================="
echo "TEST EXECUTION SUMMARY"
echo "=================================================="

echo "Tests Passed: $tests_passed"
echo "Tests Failed: $tests_failed"
echo "Total Tests:  $((tests_passed + tests_failed))"

if [ $tests_failed -eq 0 ]; then
    print_success "ALL TESTS PASSED! ✅"
    echo ""
    echo "Your ESP32 WiFi Utility is ready for deployment!"
    echo ""
    echo "Build and upload commands:"
    echo "  Standard ESP32:     pio run -e esp32dev -t upload"
    echo "  Feather ESP32-S3:   pio run -e adafruit_feather_esp32s3_tft -t upload"
    echo ""
else
    print_error "SOME TESTS FAILED! ❌"
    echo ""
    echo "Please review the failed tests above and fix any issues."
    echo "Run individual test environments with:"
    echo "  pio test -e test"
    echo "  pio test -e test_feather"
    echo ""
fi

# Return appropriate exit code
exit $tests_failed