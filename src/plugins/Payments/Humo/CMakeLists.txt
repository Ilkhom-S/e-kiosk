# Humo payments plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB HUMO_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(humo_payments
    FOLDER "plugins/Payments/Humo"
    SOURCES ${HUMO_SOURCES}
    QT_MODULES Core Xml Network
    DEPENDS
    PluginsSDK
    PPSDK
    NetworkTaskManager
    PaymentBase
    ek_boost
    INSTALL_DIR plugins
)
