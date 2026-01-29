# PPSDK (PaymentProcessor SDK) module

# Create interface library for headers
add_library(PPSDK INTERFACE)
set_target_properties(PPSDK PROPERTIES FOLDER "modules/SDK")

# Add include directories
target_include_directories(PPSDK INTERFACE
    ${EK_INCLUDES_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt modules
target_link_libraries(PPSDK INTERFACE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
    ek_boost
    ek_common
)

# Add compile definitions
target_compile_definitions(PPSDK INTERFACE _UNICODE UNICODE)

# Create implementation library
file(GLOB_RECURSE PPSDK_SOURCES
    src/*.cpp
    src/*.h
    src/moc_*.cpp
)

# Also include interface headers for moc generation
file(GLOB_RECURSE PPSDK_INTERFACE_HEADERS
    ${EK_INCLUDES_DIR}/SDK/PaymentProcessor/**/*.h
)

include(${CMAKE_SOURCE_DIR}/cmake/EKLibrary.cmake)
ek_add_library(PPSDKImpl
    FOLDER "modules/SDK"
    SOURCES ${PPSDK_SOURCES} ${PPSDK_INTERFACE_HEADERS}
    QT_MODULES Core Widgets Qml
    DEPENDS PPSDK
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
