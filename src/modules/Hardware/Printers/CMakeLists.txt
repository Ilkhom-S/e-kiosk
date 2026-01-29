# HardwarePrinters module

file(GLOB_RECURSE PRINTERS_SOURCES
    src/*.cpp
    src/*.h
)

# Add explicit template instantiation file
list(APPEND PRINTERS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PrinterTemplates.cpp
)

# Exclude .cpp files that are included in PrinterTemplates.cpp for template implementations
list(REMOVE_ITEM PRINTERS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/POSPrinters/Common/POSPrinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/POSPrinters/EjectorPOS/EjectorPOS.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/POSPrinters/Custom/CustomVKP/CustomVKP80.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/POSPrinters/Custom/CustomVKP/CustomVKP80III.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/POSPrinters/Citizen/CitizenPPU700/CitizenPPU700.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/POSPrinters/Custom/CustomPrinters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Base/Port/PortPrinterBase.cpp
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


