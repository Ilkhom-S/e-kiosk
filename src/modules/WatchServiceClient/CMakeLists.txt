# WatchServiceClient module

file(GLOB WATCHSERVICECLIENT_SOURCES
    src/*.cpp
    src/*.h
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)

# Find Boost components used by this module
find_package(Boost REQUIRED)

ek_add_library(WatchServiceClient
    SOURCES ${WATCHSERVICECLIENT_SOURCES}
    QT_MODULES Core Network
    DEPENDS Log MessageQueue
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
