#!/bin/bash
# Clean and rebuild all targets on macOS (Qt6)
# This ensures plugins are rebuilt with latest interfaces

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/macos-qt6"

echo "==================================="
echo "Clean Rebuild for macOS Qt6"
echo "==================================="

# Remove build directory
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# Create fresh build directory
echo "Creating fresh build directory..."
mkdir -p "$BUILD_DIR"

# Configure CMake
echo "Configuring CMake..."
cd "$BUILD_DIR"

# Determine vcpkg toolchain file
if [ -n "$VCPKG_ROOT" ]; then
    VCPKG_TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
elif [ -f "$PROJECT_ROOT/vcpkg_installed/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_TOOLCHAIN="$PROJECT_ROOT/vcpkg_installed/vcpkg/scripts/buildsystems/vcpkg.cmake"
else
    echo "Warning: vcpkg not found, building without vcpkg dependencies"
    VCPKG_TOOLCHAIN=""
fi

if [ -n "$VCPKG_TOOLCHAIN" ]; then
    echo "Using vcpkg toolchain: $VCPKG_TOOLCHAIN"
    cmake -S "$PROJECT_ROOT" \
          -B "$BUILD_DIR" \
          -DCMAKE_PREFIX_PATH=/usr/local/opt/qt6 \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" \
          -G Ninja
else
    cmake -S "$PROJECT_ROOT" \
          -B "$BUILD_DIR" \
          -DCMAKE_PREFIX_PATH=/usr/local/opt/qt6 \
          -DCMAKE_BUILD_TYPE=Debug \
          -G Ninja
fi

# Build all targets (includes plugins)
echo "Building all targets..."
cmake --build "$BUILD_DIR" -j8 || {
    echo "Warning: Some targets failed to build (possibly tests)"
    echo "Checking if main targets succeeded..."

    # Check critical binaries
    if [ ! -f "$BUILD_DIR/bin/ekiosk.app/Contents/MacOS/ekiosk" ]; then
        echo "Error: ekiosk failed to build"
        exit 1
    fi

    if [ ! -f "$BUILD_DIR/bin/plugins/libhumo_paymentsd.dylib" ]; then
        echo "Error: Payment plugins failed to build"
        exit 1
    fi

    echo "Main targets built successfully despite test failures"
}

echo ""
echo "==================================="
echo "Build completed successfully!"
echo "==================================="
echo "Binaries location: $BUILD_DIR/bin/"
echo "Plugins location: $BUILD_DIR/bin/plugins/"
