# CoinAcceptor driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB COINACCEPTOR_SOURCES
    src/*.cpp
    src/*.h
    ../../../includes/Common/CoreVersion.rc
)

ek_add_driver_plugin(coin_acceptors
    SOURCES ${COINACCEPTOR_SOURCES}
    DEPENDS
    DriversSDK
    HardwareCommon
    HardwareProtocols
    CashAcceptors
    CoinAcceptors
)

# Translations - reuse BillAcceptor translations (same content, easier maintenance)
file(GLOB TS_FILES_BILL_ACCEPTOR ../BillAcceptor/src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_BILL_ACCEPTOR} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
    ek_add_translations(coin_acceptors
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/locale
    )
endif()
