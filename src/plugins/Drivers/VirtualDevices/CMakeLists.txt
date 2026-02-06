# VirtualDevices driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB VIRTUALDEVICES_SOURCES
    src/*.cpp
    src/*.h
    src/**/*.cpp
    src/**/*.h
)

# Add CoreVersion.rc like in QBS file
set(VIRTUALDEVICES_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)
list(APPEND VIRTUALDEVICES_SOURCES ${VIRTUALDEVICES_VERSION_RC})

ek_add_driver_plugin(virtual_devices
    SOURCES ${VIRTUALDEVICES_SOURCES}
    QT_MODULES Gui
    DEPENDS
    DriversSDK
    HardwareCommon
    CashAcceptors
    CashDispensers
    HardwarePrinters
    PluginsSDK
    ek_common
)

# Translations - merge virtual devices and common translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins/drivers)
    ek_add_translations(virtual_devices
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/plugins/drivers
    )
endif()
