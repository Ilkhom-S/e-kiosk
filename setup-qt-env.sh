#!/bin/bash
# Qt Environment Setup Script for EKiosk Project
# This script sets up environment variables for Qt development

echo "Setting up Qt environment variables for EKiosk..."

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Detected Linux system"

    # Common Qt installation paths on Linux
    QT5_PATHS=(
        "/opt/Qt/5.15.2/gcc_64"
        "/opt/Qt/5.15.2/clang_64"
        "/usr/lib/x86_64-linux-gnu/qt5"
        "/usr/local/Qt/5.15.2"
    )

    QT6_PATHS=(
        "/opt/Qt/6.10.1/gcc_64"
        "/opt/Qt/6.10.1/clang_64"
        "/usr/lib/x86_64-linux-gnu/qt6"
    )

elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Detected macOS system"

    QT5_PATHS=(
        "/opt/Qt/5.15.2/clang_64"
        "/usr/local/Qt/5.15.2"
    )

    QT6_PATHS=(
        "/opt/Qt/6.10.1/macos"
        "/usr/local/Qt/6.10.1"
    )
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

# Function to find first existing path
find_qt_path() {
    local paths=("$@")
    for path in "${paths[@]}"; do
        if [[ -d "$path" ]]; then
            echo "$path"
            return 0
        fi
    done
    return 1
}

# Set Qt5 directory
QT5_DIR=$(find_qt_path "${QT5_PATHS[@]}")
if [[ -n "$QT5_DIR" ]]; then
    export QT5_DIR="$QT5_DIR"
    echo "QT5_DIR set to: $QT5_DIR"

    # Add to shell profile
    if [[ -f "$HOME/.bashrc" ]]; then
        echo "export QT5_DIR=\"$QT5_DIR\"" >> "$HOME/.bashrc"
    fi
    if [[ -f "$HOME/.zshrc" ]]; then
        echo "export QT5_DIR=\"$QT5_DIR\"" >> "$HOME/.zshrc"
    fi
else
    echo "Warning: Qt5 installation not found in common locations."
    echo "Please set QT5_DIR manually to your Qt5 installation directory."
fi

# Set Qt6 directory
QT6_DIR=$(find_qt_path "${QT6_PATHS[@]}")
if [[ -n "$QT6_DIR" ]]; then
    export QT6_DIR="$QT6_DIR"
    echo "QT6_DIR set to: $QT6_DIR"

    # Add to shell profile
    if [[ -f "$HOME/.bashrc" ]]; then
        echo "export QT6_DIR=\"$QT6_DIR\"" >> "$HOME/.bashrc"
    fi
    if [[ -f "$HOME/.zshrc" ]]; then
        echo "export QT6_DIR=\"$QT6_DIR\"" >> "$HOME/.zshrc"
    fi
else
    echo "Qt6 installation not found in common locations."
fi

echo ""
echo "Environment variables set for current session."
echo "Please restart your terminal or run 'source ~/.bashrc' (or ~/.zshrc)."
echo "You can verify the variables with: echo \$QT5_DIR or echo \$QT6_DIR"
echo ""
echo "For VS Code CMake extension, you can now use the default preset."