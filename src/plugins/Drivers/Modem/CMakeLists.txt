# Modem driver plugin

file(GLOB MODEM_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(modems
    SOURCES ${MODEM_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
        HardwareCommon
        HardwareModems
        DriversSDK
        HardwareIOPorts
)
