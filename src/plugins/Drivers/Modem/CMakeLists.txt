# Modem driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB MODEM_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(modems
    SOURCES ${MODEM_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    HardwareModems
    DriversSDK
    HardwareIOPorts
)
