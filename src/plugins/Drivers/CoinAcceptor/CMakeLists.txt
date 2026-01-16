# CoinAcceptor driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB COINACCEPTOR_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(coin_acceptors
    FOLDER "plugins/Drivers/CoinAcceptor"
    SOURCES ${COINACCEPTOR_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    CoinAcceptors
    DriversSDK
    PluginsSDK
    ek_common
)
