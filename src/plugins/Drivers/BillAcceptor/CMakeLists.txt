# BillAcceptor driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB BILLACCEPTOR_SOURCES
    src/*.cpp
    src/*.h
    ${EK_INCLUDES_DIR}/Common/CoreVersion.rc
)

ek_add_driver_plugin(bill_acceptors
    SOURCES ${BILLACCEPTOR_SOURCES}
    DEPENDS
    HardwareCommon
    CashAcceptors
)

# Translations - merge common and plugin-specific translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    ek_add_translations(BillAcceptorTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
    )
endif()


