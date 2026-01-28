# HardwarePrinters module

file(GLOB_RECURSE PRINTERS_SOURCES
    src/*.cpp
    src/*.h
)

set(PRINTERS_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/Printers/POSPrinterData.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/Printers/PrinterConstants.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/Printers/PrinterDevices.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/Printers/PrinterStatusCodes.h
)

ek_add_library(HardwarePrinters
    FOLDER "modules/Hardware"
    SOURCES ${PRINTERS_SOURCES} ${PRINTERS_HEADERS}
    QT_MODULES Core PrintSupport
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS EK::LibUSB
    COMPILE_DEFINITIONS _UNICODE UNICODE
)


