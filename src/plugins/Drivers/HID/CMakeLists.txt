# HID driver plugin

file(GLOB HID_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(hid_devices
    SOURCES ${HID_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    HardwareScanners
    DriversSDK
    SysUtils
)
