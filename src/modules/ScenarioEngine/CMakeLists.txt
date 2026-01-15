# ScenarioEngine module

file(GLOB SCENARIOENGINE_SOURCES
    src/*.cpp
    src/*.h
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Concurrent REQUIRED)

# Find Boost components used by this module
find_package(Boost REQUIRED COMPONENTS property_tree foreach)

ek_add_library(ScenarioEngine
    SOURCES ${SCENARIOENGINE_SOURCES}
    QT_MODULES Core Concurrent
    DEPENDS Boost::property_tree Boost::foreach
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
