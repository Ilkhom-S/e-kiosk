# Hardware modules

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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
)
# Exclude Creator directory (Windows-only USB devices)
list(FILTER CARDREADERS_SOURCES EXCLUDE REGEX ".*/Creator/.*")
if(WIN32)
    # Creator protocol is Windows-only (uses USB)
    file(GLOB_RECURSE CREATOR_SOURCES
        Cardreaders/src/Creator/*.cpp
        Cardreaders/src/Creator/*.h
    )
    list(APPEND CARDREADERS_SOURCES ${CREATOR_SOURCES})
endif()
# Exclude IDTech folder due to buggy SDK - find alternative later
list(FILTER CARDREADERS_SOURCES EXCLUDE REGEX ".*/IDTech/.*")
set(CARDREADERS_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/CardReaders/CardReaderStatusCodes.h
)
ek_add_library(Cardreaders
    FOLDER "modules/Hardware"
    SOURCES ${CARDREADERS_SOURCES} ${CARDREADERS_HEADERS}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS EK::LibUSB
    COMPILE_DEFINITIONS _UNICODE UNICODE
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
    ${CMAKE_SOURCE_DIR}/include/Hardware/CashAcceptors/BillTable.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/CashAcceptors/CashAcceptorBaseConstants.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/CashAcceptors/CashAcceptorStatusCodes.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/CashAcceptors/CurrencyDescriptions.h
)
ek_add_library(CashAcceptors
    FOLDER "modules/Hardware"
    SOURCES ${CASHACCEPTORS_SOURCES} ${CASHACCEPTORS_HEADERS}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS HardwareProtocols
    COMPILE_DEFINITIONS _UNICODE UNICODE
)

# CashDispensers
file(GLOB_RECURSE CASHDISPENSERS_SOURCES
    CashDispensers/src/*.cpp
    CashDispensers/src/*.h
)
set(CASHDISPENSERS_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/Dispensers/DispenserData.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/Dispensers/DispenserStatusCodes.h
)
ek_add_library(CashDispensers
    FOLDER "modules/Hardware"
    SOURCES ${CASHDISPENSERS_SOURCES} ${CASHDISPENSERS_HEADERS}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)

# CoinAcceptors
file(GLOB_RECURSE COINACCEPTORS_SOURCES
    CoinAcceptors/src/*.cpp
    CoinAcceptors/src/*.h
)
ek_add_library(CoinAcceptors
    FOLDER "modules/Hardware"
    SOURCES ${COINACCEPTORS_SOURCES}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)

# HardwareFR (Fiscal Registers)
if(EKIOSK_ENABLE_FR)
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
        ${CMAKE_SOURCE_DIR}/include/Hardware/FR/FRErrorDescription.h
        ${CMAKE_SOURCE_DIR}/include/Hardware/FR/FRStatusCodes.h
    )
    ek_add_library(HardwareFR
        FOLDER "modules/Hardware"
        SOURCES ${HARDWAREFR_SOURCES} ${HARDWAREFR_HEADERS}
        QT_MODULES Core
        INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS EK::LibUSB
        COMPILE_DEFINITIONS _UNICODE UNICODE
    )
    if(WIN32)
        target_link_libraries(HardwareFR PUBLIC Setupapi Advapi32)
    endif()
else()
    message(STATUS "FR (Fiscal Register) support is disabled (deprecated)")
endif()

# HardwareModems
file(GLOB HARDWAREMODEMS_SOURCES
    Modems/src/ATModem/*.cpp
    Modems/src/ATModem/*.h
)
set(HARDWAREMODEMS_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/Modems/ModemStatusCodes.h
)
ek_add_library(HardwareModems
    FOLDER "modules/Hardware"
    SOURCES ${HARDWAREMODEMS_SOURCES} ${HARDWAREMODEMS_HEADERS}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS EK::SmsMessage
    COMPILE_DEFINITIONS _UNICODE UNICODE
)


# HardwareProtocols
file(GLOB_RECURSE HARDWAREPROTOCOLS_SOURCES
    Protocols/CashAcceptor/*/src/*.cpp
    Protocols/CashAcceptor/*/src/*.h
    Protocols/Common/src/*.cpp
    Protocols/Common/src/*.h
    Protocols/Watchdog/*/src/*.cpp
    Protocols/Watchdog/*/src/*.h
)
set(HARDWAREPROTOCOLS_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/Protocols/Common/ProtocolNames.h
)
if(EKIOSK_ENABLE_FR)
    file(GLOB_RECURSE HARDWAREPROTOCOLS_FR_SOURCES
        Protocols/FR/Atol/src/*.*
        Protocols/FR/Atol2/src/*.*
        Protocols/FR/Atol3/src/*.*
        Protocols/FR/Base/src/*.*
        Protocols/FR/Common/src/*.*
        Protocols/FR/Prim/src/*.*
        Protocols/FR/Shtrih/src/*.*
        Protocols/FR/Spark/src/*.*
        Protocols/FR/Kasbi/src/*.*
        Protocols/FR/AFP/src/*.*
    )
    file(GLOB HARDWAREPROTOCOLS_FR_MAIN_SOURCES
        Protocols/FR/src/*.cpp
        Protocols/FR/src/*.h
    )
    list(APPEND HARDWAREPROTOCOLS_SOURCES ${HARDWAREPROTOCOLS_FR_SOURCES} ${HARDWAREPROTOCOLS_FR_MAIN_SOURCES})
    list(APPEND HARDWAREPROTOCOLS_HEADERS
        ${CMAKE_SOURCE_DIR}/include/Hardware/Protocols/FR/FiscalChequeStates.h
    )
endif()
ek_add_library(HardwareProtocols
    FOLDER "modules/Hardware"
    SOURCES ${HARDWAREPROTOCOLS_SOURCES} ${HARDWAREPROTOCOLS_HEADERS}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)


# HardwareScanners
# For now excluding ACR120, OPOS scanners. Need to find real devices to test on.
if(WIN32)
    file(GLOB_RECURSE HARDWARESCANNERS_SOURCES
        Scanners/src/Serial/*.cpp
        Scanners/src/Serial/*.h
    )
    set(HARDWARESCANNERS_HEADERS
        ${CMAKE_SOURCE_DIR}/include/Hardware/HID/HIDBase.h
    )
    ek_add_library(HardwareScanners
        FOLDER "modules/Hardware"
        SOURCES ${HARDWARESCANNERS_SOURCES} ${HARDWARESCANNERS_HEADERS}
        QT_MODULES Core
        INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
        COMPILE_DEFINITIONS _UNICODE UNICODE
    )
endif()
