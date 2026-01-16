# AdBackend module

file(GLOB ADBACKEND_SOURCES
    src/*.cpp
    src/*.h
    src/*.qrc
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Xml Network Sql REQUIRED)

# If Boost is needed in future, add find_package(Boost REQUIRED COMPONENTS ...)

# Find Boost components used by SDK headers
find_package(Boost REQUIRED)

ek_add_library(AdBackend
    FOLDER "modules"
    SOURCES ${ADBACKEND_SOURCES}
    QT_MODULES Core Xml Network Sql
    DEPENDS NetworkTaskManager SysUtils
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
