# DeviceManager module

file(GLOB DEVICEMANAGER_SOURCES
    src/*.cpp
    src/*.h
)

tc_add_library(DeviceManager
    SOURCES ${DEVICEMANAGER_SOURCES}
    QT_MODULES Core Concurrent
)
