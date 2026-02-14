# PPSDK (PaymentProcessor SDK) module

# Create single PPSDK library (equivalent to QBS structure)
file(GLOB_RECURSE PPSDK_SOURCES
    src/*.cpp
    src/*.h
    src/Humo/*.cpp
    src/Humo/*.h
    src/Scripting/*.cpp
    src/Scripting/*.h
    src/Security/*.*
)

# Include interface headers for moc generation (equivalent to QBS files list)
file(GLOB_RECURSE PPSDK_INTERFACE_HEADERS
    ${EK_INCLUDES_DIR}/SDK/PaymentProcessor/**/*.h
)

include(${CMAKE_SOURCE_DIR}/cmake/EKLibrary.cmake)
ek_add_library(PPSDK
    FOLDER "modules/SDK"
    SOURCES ${PPSDK_SOURCES} ${PPSDK_INTERFACE_HEADERS}
    QT_MODULES Core Widgets Qml
    INCLUDE_DIRS ${EK_INCLUDES_DIR}
    DEPENDS ek_boost ek_common NetworkTaskManager
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
