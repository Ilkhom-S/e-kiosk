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
cmake -S "$PROJECT_ROOT" \
      -B "$BUILD_DIR" \
      -DCMAKE_PREFIX_PATH=/usr/local/opt/qt6 \
      -DCMAKE_BUILD_TYPE=Debug \
      -G Ninja

# Build all targets (includes plugins)
echo "Building all targets..."
cmake --build "$BUILD_DIR" -j8

echo ""
echo "==================================="
echo "Build completed successfully!"
echo "==================================="
echo "Binaries location: $BUILD_DIR/bin/"
echo "Plugins location: $BUILD_DIR/bin/plugins/"
