# HardwareWatchdogs module

file(GLOB_RECURSE WATCHDOGS_SOURCES
    src/*.cpp
    src/*.h
)

set(WATCHDOGS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/Watchdogs/WatchdogDevices.h
    ${TC_INCLUDES_DIR}/Hardware/Watchdogs/WatchdogStatusCodes.h
)

tc_add_library(HardwareWatchdogs
    SOURCES ${WATCHDOGS_SOURCES} ${WATCHDOGS_HEADERS}
    QT_MODULES Core
)
