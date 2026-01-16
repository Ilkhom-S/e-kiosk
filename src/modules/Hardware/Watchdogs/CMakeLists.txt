# HardwareWatchdogs module

file(GLOB_RECURSE WATCHDOGS_SOURCES
    src/*.cpp
    src/*.h
)

set(WATCHDOGS_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/Watchdogs/WatchdogDevices.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/Watchdogs/WatchdogStatusCodes.h
)

ek_add_library(HardwareWatchdogs
    FOLDER "modules/Hardware/Watchdogs"
    SOURCES ${WATCHDOGS_SOURCES} ${WATCHDOGS_HEADERS}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
