# VirtualDevices driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB VIRTUALDEVICES_SOURCES
    src/*.cpp
    src/*.h
    src/**/*.cpp
    src/**/*.h
)

# Add CoreVersion.rc like in QBS file
set(VIRTUALDEVICES_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)
list(APPEND VIRTUALDEVICES_SOURCES ${VIRTUALDEVICES_VERSION_RC})

ek_add_plugin(virtual_devices
    FOLDER "plugins/Drivers"
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

# Translations - match QBS VirtualDevicesTranslations product
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(VirtualDevicesTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
    )
endif()
