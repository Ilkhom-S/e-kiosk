# BillDispensers driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB BILLDISPENSERS_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(bill_dispensers
    FOLDER "plugins/Drivers/BillDispensers"
    SOURCES ${BILLDISPENSERS_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    CashDispensers
    CashDevices
    DriversSDK
    PluginsSDK
    ek_common
)
