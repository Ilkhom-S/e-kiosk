# Watchdog driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

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

# Translations - merge watchdog and common translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
    ek_add_translations(watchdogs
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/locale
    )
endif()
