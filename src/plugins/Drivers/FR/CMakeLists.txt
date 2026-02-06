# FR (Fiscal Register) driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB FR_SOURCES
    src/*.cpp
    src/*.h
    src/Parameters/*.cpp
    src/Parameters/*.h
    ../../../includes/Common/CoreVersion.rc
    ../../../includes/Hardware/FR/FRStatusesDescriptions.h
    ../Parameters/PrinterPluginParameters.*
)

ek_add_driver_plugin(fiscal_registers
    SOURCES ${FR_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareProtocols
    HardwareCommon
    HardwarePrinters
    HardwareFR
    OPOSSDK
    SysUtils
    DriversSDK
    HardwareIOPorts
    ek_common
)

# Translations - merge FR, Printer, and common translations
file(GLOB TS_FILES_FR src/locale/*.ts)
file(GLOB TS_FILES_PRINTER ../Printer/src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_FR} ${TS_FILES_PRINTER} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins/drivers)
    ek_add_translations(fiscal_registers
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/plugins/drivers
    )
endif()

# Ensure EK::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(fiscal_registers PRIVATE EK::LibUSB)
