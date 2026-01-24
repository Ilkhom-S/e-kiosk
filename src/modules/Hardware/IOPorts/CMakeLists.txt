# HardwareIOPorts module

file(GLOB IOPORTS_COMMON_SOURCES
    src/COM/common/*.cpp
    src/COM/common/*.h
)

set(IOPORTS_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/IOPorts/IOPortStatusCodes.h
)

set(IOPORTS_SOURCES ${IOPORTS_COMMON_SOURCES} ${IOPORTS_HEADERS})

# Platform-specific sources
if(WIN32)
    file(GLOB IOPORTS_WIN_SOURCES
        src/COM/windows/*.cpp
        src/COM/windows/*.h
        src/USB/*.cpp
        src/USB/*.h
        src/LibUSB/*.cpp
        src/LibUSB/*.h
        src/TCP/*.cpp
        src/TCP/*.h
    )
    list(APPEND IOPORTS_SOURCES ${IOPORTS_WIN_SOURCES})
    list(APPEND IOPORTS_HEADERS ${CMAKE_SOURCE_DIR}/include/Hardware/IOPorts/DeviceWinProperties.h)
elseif(UNIX AND NOT APPLE)
    file(GLOB IOPORTS_UNIX_SOURCES
        src/COM/linux/*.cpp
        src/COM/linux/*.h
    )
    list(APPEND IOPORTS_SOURCES ${IOPORTS_UNIX_SOURCES})
endif()

# Cross-platform base classes (needed by AsyncSerialPort on all platforms)
file(GLOB IOPORTS_BASE_SOURCES
    src/Base/*.cpp
    src/Base/*.h
)
list(APPEND IOPORTS_SOURCES ${IOPORTS_BASE_SOURCES})

# Cross-platform AsyncSerialPort implementation
list(APPEND IOPORTS_SOURCES src/COM/AsyncSerialPort.cpp)

ek_add_library(HardwareIOPorts
    FOLDER "modules/Hardware"
    SOURCES ${IOPORTS_SOURCES}
    QT_MODULES Core Network
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS HardwareCommon SysUtils
    $<$<PLATFORM_ID:Windows>:EK::LibUSB>
    COMPILE_DEFINITIONS _UNICODE UNICODE
)




if(WIN32)
    target_link_libraries(HardwareIOPorts PUBLIC Setupapi Advapi32)
endif()
