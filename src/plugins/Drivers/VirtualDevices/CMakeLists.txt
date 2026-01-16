# VirtualDevices driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB VIRTUALDEVICES_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(virtual_devices
    FOLDER "plugins/Drivers/VirtualDevices"
    SOURCES ${VIRTUALDEVICES_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    DriversSDK
    ek_common
)
