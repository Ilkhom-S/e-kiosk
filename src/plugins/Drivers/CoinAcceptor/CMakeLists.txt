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

# Translations - reuse BillAcceptor translations (as per QBS)
file(GLOB TS_FILES_BILLACCEPTOR ../BillAcceptor/src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_BILLACCEPTOR} ${TS_FILES_COMMON})
if(TS_FILES)
    ek_add_translations(CoinAcceptorTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
    )
endif()
