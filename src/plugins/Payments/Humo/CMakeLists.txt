# Humo payments plugin
include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)



# Source and header file lists for humo_payments plugin (using globbing)
file(GLOB HUMO_PAYMENTS_SOURCES src/*.cpp src/*.cc src/*.cxx)
file(GLOB HUMO_PAYMENTS_HEADERS src/*.h src/*.hpp)

ek_add_plugin(humo_payments
    FOLDER "plugins/Payments"
    SOURCES ${HUMO_PAYMENTS_SOURCES} ${HUMO_PAYMENTS_HEADERS}
    QT_MODULES Core Xml Network
    DEPENDS
    PluginsSDK
    PPSDK
    NetworkTaskManager
    PaymentBase
    ek_boost
    INSTALL_DIR plugins
)
