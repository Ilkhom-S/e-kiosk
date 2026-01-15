# Watchdog driver plugin

file(GLOB WATCHDOG_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(watchdogs
    SOURCES ${WATCHDOG_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
        HardwareCommon
        HardwareWatchdogs
        DriversSDK
        HardwareIOPorts
)
