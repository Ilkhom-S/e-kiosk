# IOPort driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB IOPORT_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(io_ports
    SOURCES ${IOPORT_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    HardwareIOPorts
    DriversSDK
    PluginsSDK
    ek_common
)


# Ensure EK::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(io_ports PRIVATE EK::LibUSB)
