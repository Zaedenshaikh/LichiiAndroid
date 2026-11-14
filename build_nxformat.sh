#!/bin/bash
# Build script for nxformat Android library

set -e

# Configuration
ANDROID_NDK="${ANDROID_NDK:-/opt/android-ndk}"
ANDROID_API="${ANDROID_API:-24}"
ABI="arm64-v8a"
BUILD_DIR="$(pwd)/nxformat-build"
INSTALL_DIR="$(pwd)/LichiiAndroid/app/src/main/jniLibs/${ABI}"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "Building nxformat for Android ${ABI}..."

# Check NDK
if [ ! -d "$ANDROID_NDK" ]; then
    echo -e "${RED}Error: ANDROID_NDK not found at $ANDROID_NDK${NC}"
    echo "Set ANDROID_NDK environment variable or install NDK at /opt/android-ndk"
    exit 1
fi

# Clone nxformat if not present
if [ ! -d "nxformat" ]; then
    echo "Cloning nxformat..."
    git clone https://github.com/fab918/nxformat.git
fi

cd nxformat

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
cmake -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
      -DANDROID_ABI="$ABI" \
      -DANDROID_PLATFORM="android-$ANDROID_API" \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=ON \
      -DNXFORMAT_BUILD_TOOLS=OFF \
      ../nxformat

# Build
cmake --build . --config Release -j$(nproc)

# Create install directory
mkdir -p "$INSTALL_DIR"

# Copy library
if [ -f "libnxformat.so" ]; then
    cp libnxformat.so "$INSTALL_DIR/"
    echo -e "${GREEN}Success! libnxformat.so copied to $INSTALL_DIR${NC}"
else
    echo -e "${RED}Error: libnxformat.so not found${NC}"
    exit 1
fi

# Also copy headers if needed for JNI
mkdir -p "$(pwd)/../../LichiiAndroid/app/src/main/cpp/nxformat"
cp -r ../nxformat/dist/*.hpp "$(pwd)/../../LichiiAndroid/app/src/main/cpp/nxformat/" 2>/dev/null || true

echo "Done!"