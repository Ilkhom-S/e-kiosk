# Humo payments plugin

file(GLOB HUMO_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_plugin(humo_payments
    SOURCES ${HUMO_SOURCES} ${VERSION_RC}
    QT_MODULES Core Xml Network
    DEPENDS
    PluginsSDK
    PPSDK
    NetworkTaskManager
    PaymentBase
    INSTALL_DIR plugins
)
