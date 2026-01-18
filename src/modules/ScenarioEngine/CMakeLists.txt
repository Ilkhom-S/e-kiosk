# ScenarioEngine module

file(GLOB SCENARIOENGINE_SOURCES
    src/*.cpp
    src/*.h
)

# For AUTOM OC to process the public header with Q_OBJECT
set(SCENARIOENGINE_PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/ScenarioEngine/Scenario.h
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Concurrent Script REQUIRED)

# Find Boost components used by this module
find_package(Boost REQUIRED)

ek_add_library(ScenarioEngine
    FOLDER "modules"
    SOURCES ${SCENARIOENGINE_SOURCES} ${SCENARIOENGINE_PUBLIC_HEADERS}
    QT_MODULES Core Concurrent Script
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
