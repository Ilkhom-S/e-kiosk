# HID driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB HID_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(hid_devices
    FOLDER "plugins/Drivers/HID"
    SOURCES ${HID_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    HardwareScanners
    DriversSDK
    SysUtils
    ek_common
)
