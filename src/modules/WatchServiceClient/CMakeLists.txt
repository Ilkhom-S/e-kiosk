# WatchServiceClient module

file(GLOB WATCHSERVICECLIENT_SOURCES
    src/*.cpp
    src/*.h
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)

# Find Boost components used by this module
find_package(Boost REQUIRED COMPONENTS property_tree foreach)

ek_add_library(WatchServiceClient
    SOURCES ${WATCHSERVICECLIENT_SOURCES}
    QT_MODULES Core Network
    DEPENDS Boost::property_tree Boost::foreach Log MessageQueue
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)
