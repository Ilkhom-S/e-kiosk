# PPSDK (PaymentProcessor SDK) module

file(GLOB_RECURSE PPSDK_SOURCES
    src/*.cpp
    src/*.h
)

# Public headers live under EK includes dir; let ek_common provide ${EK_INCLUDES_DIR}
# ek_add_library will export the include dir passed in INCLUDE_DIRS as PUBLIC.
ek_add_library(PPSDK
    FOLDER "modules/SDK/PaymentProcessor"
    SOURCES ${PPSDK_SOURCES}
    QT_MODULES Core Widgets
    INCLUDE_DIRS ${EK_INCLUDES_DIR}
    DEPENDS ek_boost ek_common
    COMPILE_DEFINITIONS _UNICODE UNICODE
)

# Ensure automoc picks up Q_OBJECT in public headers (IRemoteClient lives in ${EK_INCLUDES_DIR})
# Add the public header to target sources so CMake's AUTOMOC processes it.
# Use a full path constructed from EK_INCLUDES_DIR to be explicit about location.
if(EXISTS "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/Core/IRemoteClient.h")
    target_sources(PPSDK PRIVATE "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/Core/IRemoteClient.h")
else()
    # Fall back to include path in source tree during development
    target_sources(PPSDK PRIVATE "${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor/Core/IRemoteClient.h")
endif()
