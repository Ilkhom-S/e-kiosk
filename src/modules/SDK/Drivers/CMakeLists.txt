# DriversSDK module

file(GLOB DRIVERSSDK_SOURCES
    src/*.cpp
    src/*.h
)

# Include SDK headers from includes directory
file(GLOB DRIVERSSDK_HEADERS
    ${CMAKE_SOURCE_DIR}/include/SDK/Drivers/*.h
    ${CMAKE_SOURCE_DIR}/include/SDK/Drivers/*/*.h
)

ek_add_library(DriversSDK
    SOURCES ${DRIVERSSDK_SOURCES} ${DRIVERSSDK_HEADERS}
    QT_MODULES Core Widgets
    DEPENDS ek_common
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
