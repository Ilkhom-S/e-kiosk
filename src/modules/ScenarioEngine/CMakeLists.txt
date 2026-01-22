# ScenarioEngine module
# Note: ScenarioEngine requires Qt State Machine framework which is not available in Qt6
# This module is only built for Qt5

if(QT_VERSION_MAJOR EQUAL 5)
    file(GLOB SCENARIOENGINE_SOURCES
        src/*.cpp
        src/*.h
    )

    # For AUTOM OC to process the public header with Q_OBJECT
    set(SCENARIOENGINE_PUBLIC_HEADERS
        ${CMAKE_SOURCE_DIR}/include/ScenarioEngine/Scenario.h
    )

    # Ensure Qt components are found for this module
    find_package(Qt5 COMPONENTS Core Concurrent Script REQUIRED)
    set(SCENARIOENGINE_QT_MODULES Core Concurrent Script)

    # Find Boost components used by this module
    find_package(Boost REQUIRED)

    ek_add_library(ScenarioEngine
        FOLDER "modules"
        SOURCES ${SCENARIOENGINE_SOURCES} ${SCENARIOENGINE_PUBLIC_HEADERS}
        QT_MODULES ${SCENARIOENGINE_QT_MODULES}
        INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
        COMPILE_DEFINITIONS _UNICODE UNICODE
    )
else()
    message(STATUS "ScenarioEngine module is disabled for Qt6 (Qt State Machine framework not available)")
endif()
