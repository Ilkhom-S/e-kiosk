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
)

# Translations
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(IOPortTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
    )
endif()

# Ensure EK::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(io_ports PRIVATE EK::LibUSB)
