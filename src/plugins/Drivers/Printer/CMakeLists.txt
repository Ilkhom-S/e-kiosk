# Printer driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB PRINTER_SOURCES
    src/*.cpp
    src/*.h
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

ek_add_plugin(printers
    SOURCES ${PRINTER_SOURCES} ${PRINTER_PARAMS} ${PRINTER_HEADERS}
    QT_MODULES Core Gui
    DEPENDS
    DriversSDK
    HardwareCommon
    HardwarePrinters
    SysUtils
    HardwareIOPorts
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)

if(WIN32)
    target_link_libraries(printers PRIVATE Advapi32 User32 Winspool)
endif()
