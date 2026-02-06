# BillAcceptor driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB BILLACCEPTOR_SOURCES
    src/*.cpp
    src/*.h
    ${EK_INCLUDES_DIR}/Common/CoreVersion.rc
    ../../../includes/Hardware/CashAcceptors/CashAcceptorStatusesDescriptions.h
)

ek_add_driver_plugin(bill_acceptors
    SOURCES ${BILLACCEPTOR_SOURCES}
    DEPENDS
    HardwareCommon
    CashAcceptors
)

# Translations - merge bill acceptor and common translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins/drivers)
    ek_add_translations(bill_acceptors
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/plugins/drivers
    )
endif()


