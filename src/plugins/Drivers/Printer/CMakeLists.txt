# Printer driver plugin

file(GLOB PRINTER_SOURCES
    src/*.cpp
    src/*.h
)

# Include shared parameters
set(PRINTER_PARAMS
    ${TC_SRC_ROOT}/plugins/Drivers/Parameters/PrinterPluginParameters.h
    ${TC_SRC_ROOT}/plugins/Drivers/Parameters/PrinterPluginParameterTranslations.h
    ${TC_SRC_ROOT}/plugins/Drivers/Parameters/PrinterPluginParameters.cpp
)

set(PRINTER_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/Printers/PrinterStatusesDescriptions.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(printers
    SOURCES ${PRINTER_SOURCES} ${PRINTER_PARAMS} ${PRINTER_HEADERS} ${VERSION_RC}
    QT_MODULES Core Gui
    DEPENDS
        DriversSDK
        HardwareCommon
        HardwarePrinters
        SysUtils
        HardwareIOPorts
)

if(WIN32)
    target_link_libraries(printers PRIVATE Advapi32 User32 Winspool)
endif()
