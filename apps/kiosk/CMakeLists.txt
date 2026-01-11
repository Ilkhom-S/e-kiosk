

cmake_minimum_required(VERSION 3.16)
project(KioskApp LANGUAGES CXX C)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql PrintSupport Network Multimedia Xml SerialPort WebSockets)

# Use EKiosk CMake helpers
include(${CMAKE_SOURCE_DIR}/cmake/EKApplication.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKStaticAnalysis.cmake)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UI_FILES src/*.ui)
set(RESOURCE_FILES ${CMAKE_SOURCE_DIR}/ekiosk.qrc)

# Application target with Qt modules and resources
ek_add_application(KioskApp
    SOURCES ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCE_FILES}
    QT_MODULES Core Widgets Sql PrintSupport Network Multimedia Xml SerialPort WebSockets
    INCLUDE_DIRS src
    LIBRARIES
    $<$<PLATFORM_ID:Windows>:rasapi32;winspool;psapi;user32;Advapi32>
)

# Static analysis (optional, but recommended)
ek_enable_static_analysis(KioskApp CPPCHECK CLANG_TIDY)
