# FR (Fiscal Register) driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB FR_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(fiscal_registers
    FOLDER "plugins/Drivers/FR"
    SOURCES ${FR_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    HardwareFR
    DriversSDK
    HardwareIOPorts
    ek_common
)

# Ensure EK::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(fiscal_registers PRIVATE EK::LibUSB)
