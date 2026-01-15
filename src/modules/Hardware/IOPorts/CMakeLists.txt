# HardwareIOPorts module

file(GLOB IOPORTS_COMMON_SOURCES
    src/COM/common/*.cpp
    src/COM/common/*.h
)

set(IOPORTS_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/IOPorts/IOPortStatusCodes.h
)

set(IOPORTS_SOURCES ${IOPORTS_COMMON_SOURCES} ${IOPORTS_HEADERS})

# Platform-specific sources
if(WIN32)
    file(GLOB IOPORTS_WIN_SOURCES
        src/COM/windows/*.cpp
        src/COM/windows/*.h
        src/Base/*.cpp
        src/Base/*.h
        src/USB/*.cpp
        src/USB/*.h
        src/LibUSB/*.cpp
        src/LibUSB/*.h
        src/TCP/*.cpp
        src/TCP/*.h
    )
    list(APPEND IOPORTS_SOURCES ${IOPORTS_WIN_SOURCES})
    list(APPEND IOPORTS_HEADERS ${TC_INCLUDES_DIR}/Hardware/IOPorts/DeviceWinProperties.h)
elseif(UNIX AND NOT APPLE)
    file(GLOB IOPORTS_LINUX_SOURCES
        src/COM/linux/*.cpp
        src/COM/linux/*.h
    )
    list(APPEND IOPORTS_SOURCES ${IOPORTS_LINUX_SOURCES})
endif()

tc_add_library(HardwareIOPorts
    SOURCES ${IOPORTS_SOURCES}
    QT_MODULES Core Network
    DEPENDS SysUtils
)




if(WIN32)
    target_link_libraries(HardwareIOPorts PUBLIC Setupapi Advapi32)
endif()

# Ensure TC::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(HardwareIOPorts PRIVATE TC::LibUSB)
