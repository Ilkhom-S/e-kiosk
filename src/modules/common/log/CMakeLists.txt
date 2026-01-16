cmake_minimum_required(VERSION 3.16)

# QFile-based logger (lightweight replacement for log4cpp)
include(${CMAKE_SOURCE_DIR}/cmake/EKLibrary.cmake)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

file(GLOB LOG_SOURCES
    "src/*.cpp"
    "src/*.h"
)

ek_add_library(Log
    FOLDER "modules/Common/Log"
    SOURCES ${LOG_SOURCES}
    DEPENDS ek_common SysUtils
)

target_link_libraries(Log PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(Log PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
