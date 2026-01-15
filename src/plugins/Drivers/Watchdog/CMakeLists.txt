# Watchdog driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB WATCHDOG_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(watchdogs
    SOURCES ${WATCHDOG_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    HardwareWatchdogs
    DriversSDK
    HardwareIOPorts
    ek_common
)
