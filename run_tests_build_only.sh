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

# Function to build test environment (no upload/execution)
build_test_env() {
    local env_name=$1
    local description=$2
    
    print_status "Building tests for $description..."
    echo "Environment: $env_name"
    
    if pio test -e "$env_name" --without-uploading; then
        print_success "$description test build PASSED"
        ((tests_passed++))
    else
        print_error "$description test build FAILED"
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

# 2. Build comprehensive test suites
print_status "Phase 2: Test Build Verification"
echo "======================================="

# Build standard ESP32 environment tests
build_test_env "test" "Standard ESP32"

# Build Feather board environment tests
build_test_env "test_feather" "Feather ESP32-S3 TFT"

# 3. Summary
echo "=================================================="
echo "TEST EXECUTION SUMMARY"
echo "=================================================="

echo "Tests Passed: $tests_passed"
echo "Tests Failed: $tests_failed"
echo "Total Tests:  $((tests_passed + tests_failed))"

if [ $tests_failed -eq 0 ]; then
    print_success "ALL BUILDS PASSED! ✅"
    echo ""
    echo "Your ESP32 WiFi Utility project builds successfully!"
    echo "All test environments compile without errors."
    echo ""
    echo "Next steps:"
    echo "  1. Deploy to hardware: pio run -e esp32dev -t upload"
    echo "  2. Deploy to Feather:   pio run -e adafruit_feather_esp32s3_tft -t upload"
    echo "  3. Run tests on hardware (with device connected):"
    echo "     pio test -e test"
    echo "     pio test -e test_feather"
    echo ""
else
    print_error "SOME BUILDS FAILED! ❌"
    echo ""
    echo "Please review the failed builds above and fix any issues."
    echo "Debug with individual builds:"
    echo "  pio test -e test --without-uploading -v"
    echo "  pio test -e test_feather --without-uploading -v"
    echo ""
fi

# Return appropriate exit code
exit $tests_failed