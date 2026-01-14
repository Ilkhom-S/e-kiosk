# WatchServiceClient module

file(GLOB WATCHSERVICECLIENT_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(WatchServiceClient
    SOURCES ${WATCHSERVICECLIENT_SOURCES}
    QT_MODULES Core Network
    DEPENDS boost
)
