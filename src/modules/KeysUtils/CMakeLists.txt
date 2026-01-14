# KeysUtils module

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB KEYSUTILS_SOURCES
    src/*.cpp
    src/*.h
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

ek_add_library(KeysUtils
    SOURCES ${KEYSUTILS_SOURCES}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
)
