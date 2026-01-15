# DeviceManager module

file(GLOB DEVICEMANAGER_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(DeviceManager
    SOURCES ${DEVICEMANAGER_SOURCES}
    QT_MODULES Core Concurrent
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
