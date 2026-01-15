# VirtualDevices driver plugin

file(GLOB VIRTUALDEVICES_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(virtual_devices
    SOURCES ${VIRTUALDEVICES_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
        HardwareCommon
        DriversSDK
)
