# BillAcceptor driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB BILLACCEPTOR_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(bill_acceptors
    FOLDER "plugins/Drivers"
    SOURCES ${BILLACCEPTOR_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    CashAcceptors
    DriversSDK
    PluginsSDK
    ek_common
)

# Translations - merge common and plugin-specific translations
file(GLOB TS_FILES_RU src/locale/bill_acceptors_ru.ts)
file(GLOB TS_FILES_EN src/locale/bill_acceptors_en.ts)
file(GLOB TS_FILES_TG src/locale/bill_acceptors_tg.ts)
file(GLOB TS_FILES_UZ src/locale/bill_acceptors_uz.ts)


