# Printer driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB PRINTER_SOURCES
    src/*.cpp
    src/*.h
    ../../../includes/Common/CoreVersion.rc
)

# Include shared parameters
set(PRINTER_PARAMS
    ${CMAKE_SOURCE_DIR}/src/plugins/Drivers/Parameters/PrinterPluginParameters.h
    ${CMAKE_SOURCE_DIR}/src/plugins/Drivers/Parameters/PrinterPluginParameterTranslations.h
    ${CMAKE_SOURCE_DIR}/src/plugins/Drivers/Parameters/PrinterPluginParameters.cpp
)

set(PRINTER_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/Printers/PrinterStatusesDescriptions.h
)

set(QT_MODULES Gui PrintSupport)
if(WIN32)
    list(APPEND QT_MODULES AxContainer)
endif()

ek_add_driver_plugin(printers
    SOURCES ${PRINTER_SOURCES} ${PRINTER_PARAMS} ${PRINTER_HEADERS}
    QT_MODULES ${QT_MODULES}
    DEPENDS
    DriversSDK
    HardwareCommon
    HardwarePrinters
    SysUtils
    HardwareIOPorts
    EK::LibUSB
    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/modules/Hardware/Printers/src
)

# Translations - merge printer and common translations
file(GLOB TS_FILES_PRINTER src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PRINTER} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
    ek_add_translations(printers
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/locale
    )
endif()

if(WIN32)
    target_link_libraries(printers PRIVATE Advapi32 User32 Winspool)
endif()
