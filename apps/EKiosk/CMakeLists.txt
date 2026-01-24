

cmake_minimum_required(VERSION 3.16)
project(EKiosk LANGUAGES CXX C)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql PrintSupport Network Multimedia Xml SerialPort WebSockets WebEngineWidgets Core5Compat)

# Use EKiosk CMake helpers
include(${CMAKE_SOURCE_DIR}/cmake/EKApplication.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKStaticAnalysis.cmake)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UI_FILES src/*.ui)

# Exclude Windows-only connection files on non-Windows platforms
if(NOT WIN32)
    list(REMOVE_ITEM SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/connection/RasConnection.cpp
    )
    list(REMOVE_ITEM HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/connection/RasConnection.h
    )
endif()

set(RESOURCE_FILES ${CMAKE_SOURCE_DIR}/ekiosk.qrc)

# Application target with Qt modules and resources
ek_add_application(client
    FOLDER "apps"
    SOURCES ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCE_FILES}
    QT_MODULES Core Widgets Sql PrintSupport Network Multimedia Xml SerialPort WebSockets WebEngineWidgets Core5Compat
    LIBRARIES
    BasicApplication
    EK::QZint
    $<$<PLATFORM_ID:Windows>:rasapi32;winspool;psapi;user32;Advapi32>
)

# Link common include INTERFACE target so `include/` headers are available
target_link_libraries(client PUBLIC ek_common)

# Link Boost includes for SDK headers that use Boost
# Link Boost includes for SDK headers that use Boost
target_link_libraries(client PUBLIC ek_boost)

# Option to enable test mode (disables kiosk restrictions, e.g. killing explorer.exe)
option(EKIOSK_TEST_MODE "Enable test mode (no kiosk restrictions)" ON)
if(EKIOSK_TEST_MODE)
    target_compile_definitions(client PRIVATE EKIOSK_TEST_MODE)
endif()

# Static analysis (optional, but recommended)
# ek_enable_static_analysis(client CPPCHECK CLANG_TIDY)
