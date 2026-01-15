# BillDispensers driver plugin

file(GLOB BILLDISPENSERS_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(bill_dispensers
    SOURCES ${BILLDISPENSERS_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
    HardwareProtocols
    HardwareCommon
    CashDispensers
    CashDevices
    DriversSDK
    PluginsSDK
)
