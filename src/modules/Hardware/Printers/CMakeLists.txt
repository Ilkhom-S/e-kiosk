# HardwarePrinters module

file(GLOB_RECURSE PRINTERS_SOURCES
    src/*.cpp
    src/*.h
)

set(PRINTERS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/Printers/POSPrinterData.h
    ${TC_INCLUDES_DIR}/Hardware/Printers/PrinterConstants.h
    ${TC_INCLUDES_DIR}/Hardware/Printers/PrinterDevices.h
    ${TC_INCLUDES_DIR}/Hardware/Printers/PrinterStatusCodes.h
)

tc_add_library(HardwarePrinters
    SOURCES ${PRINTERS_SOURCES} ${PRINTERS_HEADERS}
    QT_MODULES Core
)

# Ensure TC::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(HardwarePrinters PRIVATE TC::LibUSB)
