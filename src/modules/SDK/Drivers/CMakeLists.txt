# DriversSDK module

file(GLOB DRIVERSSDK_SOURCES
    src/*.cpp
    src/*.h
)

# Include SDK headers from includes directory
file(GLOB DRIVERSSDK_HEADERS
    ${TC_INCLUDES_DIR}/SDK/Drivers/*.h
    ${TC_INCLUDES_DIR}/SDK/Drivers/*/*.h
)

tc_add_library(DriversSDK
    SOURCES ${DRIVERSSDK_SOURCES} ${DRIVERSSDK_HEADERS}
    QT_MODULES Core Widgets
    INCLUDE_DIRS ${TC_INCLUDES_DIR}/SDK/Drivers
)
