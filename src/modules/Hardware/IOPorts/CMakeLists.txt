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
    list(APPEND IOPORTS_HEADERS ${CMAKE_SOURCE_DIR}/include/Hardware/IOPorts/DeviceWinProperties.h)
elseif(UNIX AND NOT APPLE)
    file(GLOB IOPORTS_LINUX_SOURCES
        src/COM/linux/*.cpp
        src/COM/linux/*.h
    )
    list(APPEND IOPORTS_SOURCES ${IOPORTS_LINUX_SOURCES})
endif()

ek_add_library(HardwareIOPorts
    SOURCES ${IOPORTS_SOURCES}
    QT_MODULES Core Network
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS SysUtils EK::LibUSB
    COMPILE_DEFINITIONS _UNICODE UNICODE
)




if(WIN32)
    target_link_libraries(HardwareIOPorts PUBLIC Setupapi Advapi32)
endif()
