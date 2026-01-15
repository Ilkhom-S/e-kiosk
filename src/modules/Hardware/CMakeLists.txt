# Hardware modules



# Core hardware submodules
add_subdirectory(Common)
add_subdirectory(IOPorts)
add_subdirectory(Watchdogs)
add_subdirectory(Printers)

# Main hardware libraries (defined here, not in subdirectories)

# Cardreaders
file(GLOB_RECURSE CARDREADERS_SOURCES
    Cardreaders/src/*.cpp
    Cardreaders/src/*.h
    Cardreaders/Protocols/Creator/src/*.cpp
    Cardreaders/Protocols/Creator/src/*.h
)
set(CARDREADERS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/CardReaders/CardReaderStatusCodes.h
)
tc_add_library(Cardreaders
    SOURCES ${CARDREADERS_SOURCES} ${CARDREADERS_HEADERS}
    QT_MODULES Core
    DEPENDS IDTech_SDK
)

# CashAcceptors
file(GLOB_RECURSE CASHACCEPTORS_SOURCES
    CashAcceptors/src/*.cpp
    CashAcceptors/src/*.h
    CoinAcceptors/src/CCTalk/*.cpp
    CoinAcceptors/src/CCTalk/*.h
    Acceptors/src/*.cpp
    Acceptors/src/*.h
    CashDevices/src/*.cpp
    CashDevices/src/*.h
)
set(CASHACCEPTORS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/CashAcceptors/BillTable.h
    ${TC_INCLUDES_DIR}/Hardware/CashAcceptors/CashAcceptorBaseConstants.h
    ${TC_INCLUDES_DIR}/Hardware/CashAcceptors/CashAcceptorStatusCodes.h
    ${TC_INCLUDES_DIR}/Hardware/CashAcceptors/CurrencyDescriptions.h
)
tc_add_library(CashAcceptors
    SOURCES ${CASHACCEPTORS_SOURCES} ${CASHACCEPTORS_HEADERS}
    QT_MODULES Core
)

# CashDispensers
file(GLOB_RECURSE CASHDISPENSERS_SOURCES
    CashDispensers/src/*.cpp
    CashDispensers/src/*.h
)
set(CASHDISPENSERS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/Dispensers/DispenserData.h
    ${TC_INCLUDES_DIR}/Hardware/Dispensers/DispenserStatusCodes.h
)
tc_add_library(CashDispensers
    SOURCES ${CASHDISPENSERS_SOURCES} ${CASHDISPENSERS_HEADERS}
    QT_MODULES Core
)

# CoinAcceptors
file(GLOB_RECURSE COINACCEPTORS_SOURCES
    CoinAcceptors/src/*.cpp
    CoinAcceptors/src/*.h
)
tc_add_library(CoinAcceptors
    SOURCES ${COINACCEPTORS_SOURCES}
    QT_MODULES Core
)

# HardwareFR (Fiscal Registers)
file(GLOB HARDWAREFR_SOURCES
    FR/src/Atol/*.*
    FR/src/Atol/*/*.*
    FR/src/Atol/*/*/*.*
    FR/src/Base/*.*
    FR/src/Base/Port/*.*
    # FR/src/OPOSMStarTUPK/*.*
    FR/src/Prim/*.*
    FR/src/Prim/*/*.*
    FR/src/Shtrih/*.*
    FR/src/Shtrih/*/*.*
    FR/src/Shtrih/*/*/*.*
    FR/src/Spark/*.*
    FR/src/Kasbi/*.*
    FR/src/MStar/Online/*.*
)
set(HARDWAREFR_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/FR/FRErrorDescription.h
    ${TC_INCLUDES_DIR}/Hardware/FR/FRStatusCodes.h
)
tc_add_library(HardwareFR
    SOURCES ${HARDWAREFR_SOURCES} ${HARDWAREFR_HEADERS}
    QT_MODULES Core
)
if(WIN32)
    target_link_libraries(HardwareFR PUBLIC Setupapi Advapi32)
endif()

# HardwareModems
file(GLOB HARDWAREMODEMS_SOURCES
    Modems/src/ATModem/*.cpp
    Modems/src/ATModem/*.h
)
set(HARDWAREMODEMS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/Modems/ModemStatusCodes.h
)
tc_add_library(HardwareModems
    SOURCES ${HARDWAREMODEMS_SOURCES} ${HARDWAREMODEMS_HEADERS}
    QT_MODULES Core
    DEPENDS SmsMessage
)


# HardwareProtocols
file(GLOB_RECURSE HARDWAREPROTOCOLS_SOURCES
    Protocols/FR/Atol2/src/*.*,
    Protocols/FR/Atol3/src/*.*,
    Protocols/FR/Common/src/*.*,
    Protocols/FR/Prim/src/*.*,
    Protocols/FR/Shtrih/src/*.*,
    Protocols/FR/Spark/src/*.*,
    Protocols/FR/Kasbi/src/*.*,
    Protocols/FR/AFP/src/*.*,
    Protocols/CashAcceptor/*/src/*.cpp
    Protocols/CashAcceptor/*/src/*.h
    Protocols/Watchdog/*/src/*.cpp
    Protocols/Watchdog/*/src/*.h
)
set(HARDWAREPROTOCOLS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/Protocols/Common/ProtocolNames.h
    ${TC_INCLUDES_DIR}/Hardware/Protocols/FR/FiscalChequeStates.h
)
tc_add_library(HardwareProtocols
    SOURCES ${HARDWAREPROTOCOLS_SOURCES} ${HARDWAREPROTOCOLS_HEADERS}
    QT_MODULES Core
)


# HardwareScanners
# For now excluding ACR120, OPOS scanners. Need to find real devices to test on.
file(GLOB_RECURSE HARDWARESCANNERS_SOURCES
    Scanners/src/Serial/*.cpp
    Scanners/src/Serial/*.h
)
set(HARDWARESCANNERS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/HID/HIDBase.h
)
tc_add_library(HardwareScanners
    SOURCES ${HARDWARESCANNERS_SOURCES} ${HARDWARESCANNERS_HEADERS}
    QT_MODULES Core
)

# Ensure TC::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(HardwareFR PRIVATE TC::LibUSB)
