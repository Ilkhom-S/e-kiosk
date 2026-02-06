# IOPort driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB IOPORT_SOURCES
    src/*.cpp
    src/*.h
    ../../../includes/Common/CoreVersion.rc
    ../../../includes/Hardware/IOPorts/IOPortStatusesDescriptions.h
)

ek_add_driver_plugin(io_ports
    SOURCES ${IOPORT_SOURCES}
    QT_MODULES Network
    DEPENDS
    DriversSDK
    HardwareCommon
    HardwareIOPorts
    SysUtils
    EK::LibUSB
)

# Translations - merge IO port and common translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins/drivers)
    ek_add_translations(io_ports
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/plugins/drivers
    )
endif()


