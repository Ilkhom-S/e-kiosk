# Watchdog driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB WATCHDOG_SOURCES
    src/*.cpp
    src/*.h
)

# Add CoreVersion.rc like in QBS file
set(WATCHDOG_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)
list(APPEND WATCHDOG_SOURCES ${WATCHDOG_VERSION_RC})

ek_add_driver_plugin(watchdogs
    SOURCES ${WATCHDOG_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    HardwareWatchdogs
    HardwareProtocols
    PluginsSDK
    DriversSDK
    HardwareIOPorts
    ek_common
)

# Translations - match QBS WatchdogTranslations product
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(WatchdogTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
    )
endif()
