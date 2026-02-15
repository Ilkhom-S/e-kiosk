#!/bin/bash
# Rebuild only plugins (faster than full rebuild)
# Use this when only plugin code changed

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/macos-qt6"

echo "==================================="
echo "Rebuild Plugins Only"
echo "==================================="

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found: $BUILD_DIR"
    echo "Run clean-rebuild-macos.sh first"
    exit 1
fi

cd "$BUILD_DIR"

# List of main plugin targets
PLUGIN_TARGETS=(
    "libhumo_paymentsd"
    "libservice_menud"
    "libhumo_service_menud"
    "libadd"
    "libqml_backendd"
    "libnative_backendd"
    "libscreen_maker_scenariosd"
    "libmigrator3000_scenariosd"
)

echo "Cleaning plugin build artifacts..."
for target in "${PLUGIN_TARGETS[@]}"; do
    if [ -f "bin/plugins/$target.dylib" ]; then
        echo "  Removing bin/plugins/$target.dylib"
        rm -f "bin/plugins/$target.dylib"
    fi
done

# Remove all .qm translation files to force rebuild
echo "Removing translation files..."
find bin/plugins -name "*.qm" -delete 2>/dev/null || true

echo ""
echo "Rebuilding all plugin targets..."
cmake --build "$BUILD_DIR" --target all -j8

echo ""
echo "==================================="
echo "Plugin rebuild completed!"
echo "==================================="
echo "Plugins location: $BUILD_DIR/bin/plugins/"
find "$BUILD_DIR/bin/plugins" -name "*.dylib" -type f | while read plugin; do
    echo "  - $(basename $plugin)"
done
