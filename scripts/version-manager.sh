#!/bin/bash

# ESP32 WiFi Utility - Version Management Script
# This script helps manage version bumps in platformio.ini and creates automated releases

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to display usage
show_usage() {
    echo -e "${BLUE}ESP32 WiFi Utility Version Management${NC}"
    echo ""
    echo "Usage: $0 [COMMAND] [VERSION]"
    echo ""
    echo -e "${YELLOW}Commands:${NC}"
    echo "  show              Show current version"
    echo "  bump [VERSION]    Bump version to specified version"
    echo "  patch             Bump patch version (e.g., 2.0.0 -> 2.0.1)"
    echo "  minor             Bump minor version (e.g., 2.0.0 -> 2.1.0)"
    echo "  major             Bump major version (e.g., 2.0.0 -> 3.0.0)"
    echo "  check             Check if version format is valid"
    echo "  help              Show this help message"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  $0 show                    # Show current version"
    echo "  $0 bump 2.1.0             # Set version to 2.1.0"
    echo "  $0 patch                  # Increment patch version"
    echo "  $0 minor                  # Increment minor version"
    echo "  $0 major                  # Increment major version"
    echo ""
    echo -e "${YELLOW}Notes:${NC}"
    echo "- Version format must follow semantic versioning (e.g., 2.0.0)"
    echo "- Changes to platformio.ini will trigger automated GitHub release"
    echo "- Make sure you're on the main branch before bumping versions"
    echo "- The automated release includes firmware for both ESP32dev and Feather ESP32-S3 TFT"
}

# Function to get current version
get_current_version() {
    if [ ! -f "platformio.ini" ]; then
        echo -e "${RED}Error: platformio.ini not found in current directory${NC}"
        exit 1
    fi
    
    VERSION=$(grep -o 'DVERSION="[^"]*"' platformio.ini | head -1 | sed 's/DVERSION="//;s/"//')
    if [ -z "$VERSION" ]; then
        echo -e "${RED}Error: Could not extract version from platformio.ini${NC}"
        exit 1
    fi
    echo "$VERSION"
}

# Function to validate version format
validate_version() {
    local version="$1"
    if [[ ! $version =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9]+)?$ ]]; then
        echo -e "${RED}Error: Invalid version format: $version${NC}"
        echo -e "${YELLOW}Version must follow semantic versioning (e.g., 2.0.0, 2.1.0-beta)${NC}"
        exit 1
    fi
}

# Function to update version in platformio.ini
update_version() {
    local new_version="$1"
    local current_version=$(get_current_version)
    
    validate_version "$new_version"
    
    if [ "$new_version" == "$current_version" ]; then
        echo -e "${YELLOW}Version is already $new_version${NC}"
        return 0
    fi
    
    echo -e "${BLUE}Updating version from $current_version to $new_version${NC}"
    
    # Update version in platformio.ini for both environments
    sed -i "s/-DVERSION=\"[^\"]*\"/-DVERSION=\"$new_version\"/g" platformio.ini
    
    # Verify the update
    local updated_version=$(get_current_version)
    if [ "$updated_version" == "$new_version" ]; then
        echo -e "${GREEN}✅ Version successfully updated to $new_version${NC}"
        echo -e "${YELLOW}📝 platformio.ini has been modified${NC}"
        echo -e "${BLUE}💡 Commit and push to main branch to trigger automated release${NC}"
    else
        echo -e "${RED}Error: Failed to update version${NC}"
        exit 1
    fi
}

# Function to increment version components
increment_version() {
    local current_version=$(get_current_version)
    local type="$1"
    
    # Parse semantic version
    if [[ $current_version =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)(-.*)?$ ]]; then
        local major=${BASH_REMATCH[1]}
        local minor=${BASH_REMATCH[2]}
        local patch=${BASH_REMATCH[3]}
        local suffix=${BASH_REMATCH[4]}
        
        case $type in
            "major")
                major=$((major + 1))
                minor=0
                patch=0
                ;;
            "minor")
                minor=$((minor + 1))
                patch=0
                ;;
            "patch")
                patch=$((patch + 1))
                ;;
            *)
                echo -e "${RED}Error: Invalid increment type: $type${NC}"
                exit 1
                ;;
        esac
        
        # Remove suffix for auto-increments
        local new_version="$major.$minor.$patch"
        update_version "$new_version"
    else
        echo -e "${RED}Error: Could not parse current version: $current_version${NC}"
        exit 1
    fi
}

# Function to show git status and recommendations
show_git_info() {
    echo ""
    echo -e "${BLUE}Git Status:${NC}"
    
    # Check if we're in a git repository
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        echo -e "${RED}Not in a git repository${NC}"
        return
    fi
    
    # Show current branch
    local branch=$(git branch --show-current)
    echo -e "Current branch: ${YELLOW}$branch${NC}"
    
    # Check if there are uncommitted changes
    if ! git diff-index --quiet HEAD --; then
        echo -e "${YELLOW}⚠️  You have uncommitted changes${NC}"
        echo -e "${BLUE}Modified files:${NC}"
        git status --porcelain
    else
        echo -e "${GREEN}✅ Working tree is clean${NC}"
    fi
    
    # Show recommendation
    if [ "$branch" == "main" ]; then
        echo -e "${GREEN}✅ On main branch - ready for version bump${NC}"
    else
        echo -e "${YELLOW}💡 Consider switching to main branch before version bump${NC}"
    fi
}

# Main script logic
case "${1:-help}" in
    "show")
        current_version=$(get_current_version)
        echo -e "${BLUE}Current version:${NC} ${GREEN}$current_version${NC}"
        show_git_info
        ;;
    
    "bump")
        if [ -z "$2" ]; then
            echo -e "${RED}Error: Version number required${NC}"
            echo "Usage: $0 bump <version>"
            exit 1
        fi
        update_version "$2"
        show_git_info
        ;;
    
    "patch"|"minor"|"major")
        increment_version "$1"
        show_git_info
        ;;
    
    "check")
        current_version=$(get_current_version)
        validate_version "$current_version"
        echo -e "${GREEN}✅ Current version format is valid: $current_version${NC}"
        ;;
    
    "help"|"-h"|"--help")
        show_usage
        ;;
    
    *)
        echo -e "${RED}Error: Unknown command: $1${NC}"
        echo ""
        show_usage
        exit 1
        ;;
esac