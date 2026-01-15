# BillAcceptor driver plugin

file(GLOB BILLACCEPTOR_SOURCES
    src/*.cpp
    src/*.h
)

set(BILLACCEPTOR_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/CashAcceptors/CashAcceptorStatusesDescriptions.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(bill_acceptors
    SOURCES ${BILLACCEPTOR_SOURCES} ${BILLACCEPTOR_HEADERS} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
        HardwareProtocols
        HardwareCommon
        CashAcceptors
        DriversSDK
        PluginsSDK
)

# Translations - merge common and plugin-specific translations
file(GLOB TS_FILES_RU src/locale/bill_acceptors_ru.ts)
file(GLOB TS_FILES_EN src/locale/bill_acceptors_en.ts)
file(GLOB TS_FILES_TG src/locale/bill_acceptors_tg.ts)
file(GLOB TS_FILES_UZ src/locale/bill_acceptors_uz.ts)

# TODO: Add multi-locale translation handling
