# Humo payments plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB HUMO_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(humo_payments
    SOURCES ${HUMO_SOURCES}
    QT_MODULES Core Xml Network
    DEPENDS
    PluginsSDK
    PPSDK
    NetworkTaskManager
    PaymentBase
    INSTALL_DIR plugins
)
