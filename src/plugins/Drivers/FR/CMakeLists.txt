# FR (Fiscal Register) driver plugin

file(GLOB FR_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_driver(fiscal_registers
    SOURCES ${FR_SOURCES} ${VERSION_RC}
    QT_MODULES Core
    DEPENDS
    HardwareProtocols
    HardwareCommon
    HardwareFR
    DriversSDK
    HardwareIOPorts
)

# Ensure TC::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(fiscal_registers PRIVATE TC::LibUSB)
