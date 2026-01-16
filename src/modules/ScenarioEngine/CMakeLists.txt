# ScenarioEngine module

file(GLOB SCENARIOENGINE_SOURCES
    src/*.cpp
    src/*.h
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Concurrent REQUIRED)

# Find Boost components used by this module
find_package(Boost REQUIRED)

ek_add_library(ScenarioEngine
    FOLDER "modules"
    SOURCES ${SCENARIOENGINE_SOURCES}
    QT_MODULES Core Concurrent
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
