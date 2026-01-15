# IOPort driver plugin

file(GLOB IOPORT_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(io_ports
    SOURCES ${IOPORT_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    HardwareIOPorts
    DriversSDK
    PluginsSDK
)


# Ensure TC::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(io_ports PRIVATE TC::LibUSB)
