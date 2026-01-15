# CoinAcceptor driver plugin

file(GLOB COINACCEPTOR_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(coin_acceptors
    SOURCES ${COINACCEPTOR_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
    HardwareProtocols
    HardwareCommon
    CoinAcceptors
    DriversSDK
    PluginsSDK
)
