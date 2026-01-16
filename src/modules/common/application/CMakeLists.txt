# BasicApplication library (modules/common/application)

cmake_minimum_required(VERSION 3.16)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Widgets)

# Use EK helper for libraries
include(${CMAKE_SOURCE_DIR}/cmake/EKLibrary.cmake)

ek_add_library(BasicApplication
    FOLDER "modules/Common/Application"
    SOURCES src/BasicApplication.cpp
    QT_MODULES Core Network Widgets
    DEPENDS ek_common Log DebugUtils SysUtils EK::SingleApplication
)

# Enable automoc for QObject-derived types
set_target_properties(BasicApplication PROPERTIES AUTOMOC ON)

target_compile_features(BasicApplication PUBLIC cxx_std_14)
