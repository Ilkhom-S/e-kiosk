cmake_minimum_required(VERSION 3.16)

# QFile-based logger (lightweight replacement for log4cpp)
include(${CMAKE_SOURCE_DIR}/cmake/EKLibrary.cmake)

ek_add_library(Log
    SOURCES src/QFileLogger.cpp
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(Log PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(Log PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
