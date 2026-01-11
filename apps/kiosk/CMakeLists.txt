cmake_minimum_required(VERSION 3.16)

project(KioskApp LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
set(QT_VERSION_MAJOR ${QT_VERSION_MAJOR})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    Widgets
    Sql
    PrintSupport
    Network
    Multimedia
    Xml
    SerialPort
    WebSockets
)

set(QT_COMMON_COMPONENTS
    Core
    Widgets
    Sql
    PrintSupport
    Network
    Multimedia
    Xml
    SerialPort
    WebSockets
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMMON_COMPONENTS})

file(GLOB_RECURSE SOURCES
    src/*.cpp
)
file(GLOB_RECURSE HEADERS
    src/*.h
)
file(GLOB_RECURSE UI_FILES
    src/*.ui
)
set(RESOURCE_FILES
    ${CMAKE_SOURCE_DIR}/ekiosk.qrc
)

if(QT_VERSION_MAJOR EQUAL 5)
    qt5_add_resources(RESOURCE_SOURCES ${RESOURCE_FILES})
else()
    qt6_add_resources(RESOURCE_SOURCES ${RESOURCE_FILES})
endif()

add_executable(KioskApp
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_SOURCES}
)

target_include_directories(KioskApp PRIVATE src)

target_link_libraries(KioskApp
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::WebSockets
)

if(WIN32)
    target_link_libraries(KioskApp
        rasapi32
        winspool
        psapi
        user32
        Advapi32
    )
endif()

if(MSVC)
    target_compile_options(KioskApp PRIVATE /FS)
endif()

set_target_properties(KioskApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../EKiosk
)
