# CardReader driver plugin

file(GLOB CARDREADER_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(card_readers
    SOURCES ${CARDREADER_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
    HardwareProtocols
    HardwareCommon
    Cardreaders
    DriversSDK
    HardwareIOPorts
    PluginsSDK
)
