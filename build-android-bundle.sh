#!/bin/bash
#
# Build script for creating a signed Android App Bundle for Google Play Store
#
# Usage:
#   ./build-android-bundle.sh
#
# The script will prompt for the keystore password if not already set.
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}DB48X Android App Bundle Builder${NC}"
echo "=================================="
echo ""

# Check if keystore exists
KEYSTORE_PATH="$HOME/.local/android_release.keystore"
if [ ! -f "$KEYSTORE_PATH" ]; then
    echo -e "${RED}Error: Keystore not found at $KEYSTORE_PATH${NC}"
    exit 1
fi
echo -e "${GREEN}✓${NC} Found keystore: $KEYSTORE_PATH"

# Check if password is set
if [ -z "$ANDROID_KEYSTORE_PASS" ]; then
    echo ""
    echo -e "${YELLOW}Warning: ANDROID_KEYSTORE_PASS environment variable not set${NC}"
    echo "Please enter your keystore password:"
    read -s ANDROID_KEYSTORE_PASS
    export ANDROID_KEYSTORE_PASS
    echo ""
fi

# Verify required tools
echo ""
echo "Checking build environment..."

if [ ! -d "${ANDROID_SDK_ROOT:-/opt/homebrew/share/android-commandlinetools}" ]; then
    echo -e "${RED}Error: Android SDK not found${NC}"
    echo "Set ANDROID_SDK_ROOT to your Android SDK location"
    exit 1
fi
echo -e "${GREEN}✓${NC} Android SDK found"

if [ ! -d "${ANDROID_QT:-/Volumes/Qt/6.8.1/android_arm64_v8a}" ]; then
    echo -e "${YELLOW}Warning: Qt for Android not found at default location${NC}"
    echo "Make sure ANDROID_QT is set correctly if build fails"
else
    echo -e "${GREEN}✓${NC} Qt for Android found"
fi

# Clean previous build (optional)
if [ "$1" == "clean" ]; then
    echo ""
    echo "Cleaning previous build..."
    rm -rf android/
    rm -rf sim/android-*
    echo -e "${GREEN}✓${NC} Clean complete"
fi

# Build
echo ""
echo "Building Android App Bundle..."
echo "Target SDK: 35 (Android 15 - Required for Google Play)"
echo ""

if make android; then
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}Build Successful!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    echo "App Bundle location: android/db48x.aab"
    echo ""
    echo "Next steps:"
    echo "1. Test the bundle on a device or emulator"
    echo "2. Upload to Google Play Console"
    echo "3. Submit for review"
    echo ""
    
    # Show file info
    if [ -f "android/db48x.aab" ]; then
        SIZE=$(du -h android/db48x.aab | cut -f1)
        echo "Bundle size: $SIZE"
    fi
else
    echo ""
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}Build Failed!${NC}"
    echo -e "${RED}========================================${NC}"
    echo ""
    echo "Check the error messages above for details."
    exit 1
fi


