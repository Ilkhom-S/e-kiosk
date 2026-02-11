# Deprecated EKiosk Source Code
# This folder contains deprecated modules from the old monolithic EKiosk application
# These are kept for reference only and are not built as part of the main project

# List of deprecated modules (not compiled)
set(DEPRECATED_MODULES
    db
    devices
    modules
    other
    ui
    updater
    connection
)

message(STATUS "Deprecated EKiosk modules: ${DEPRECATED_MODULES}")
message(STATUS "Deprecated files: mainwindow.h, mainwindow.cpp, mainwindow.ui")
