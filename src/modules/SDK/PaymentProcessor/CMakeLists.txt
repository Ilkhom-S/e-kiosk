# PPSDK (PaymentProcessor SDK) module

file(GLOB_RECURSE PPSDK_SOURCES
    src/*.cpp
    src/*.h
)

# Public headers live under EK includes dir; let ek_common provide ${EK_INCLUDES_DIR}
# ek_add_library will export the include dir passed in INCLUDE_DIRS as PUBLIC.
ek_add_library(PPSDK
    FOLDER "modules/SDK"
    SOURCES ${PPSDK_SOURCES}
    QT_MODULES Core Widgets Qml
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

# Add IDeviceTest interface header for AUTOMOC processing
if(EXISTS "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/IDeviceTest.h")
    target_sources(PPSDK PRIVATE "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/IDeviceTest.h")
else()
    # Fall back to include path in source tree during development
    target_sources(PPSDK PRIVATE "${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor/IDeviceTest.h")
endif()

# Add EventTypes header for AUTOMOC processing (contains Q_ENUM)
if(EXISTS "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/Core/EventTypes.h")
    target_sources(PPSDK PRIVATE "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/Core/EventTypes.h")
else()
    # Fall back to include path in source tree during development
    target_sources(PPSDK PRIVATE "${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor/Core/EventTypes.h")
endif()

# Add scripting headers with Q_OBJECT for AUTOMOC processing
set(SCRIPTING_HEADERS
    IBackendScenarioObject.h
    Settings.h
    ScriptArray.h
    PrinterService.h
    NetworkService.h
    PaymentService.h
    HIDService.h
    GUIService.h
    FundsService.h
    DeviceService.h
    Core.h
    AdService.h
)

foreach(HEADER ${SCRIPTING_HEADERS})
    if(EXISTS "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/Scripting/${HEADER}")
        target_sources(PPSDK PRIVATE "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/Scripting/${HEADER}")
    else()
        # Fall back to include path in source tree during development
        target_sources(PPSDK PRIVATE "${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor/Scripting/${HEADER}")
    endif()
endforeach()

# Add payment headers with Q_OBJECT for AUTOMOC processing
if(EXISTS "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/Payment/Step.h")
    target_sources(PPSDK PRIVATE "${EK_INCLUDES_DIR}/SDK/PaymentProcessor/Payment/Step.h")
else()
    # Fall back to include path in source tree during development
    target_sources(PPSDK PRIVATE "${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor/Payment/Step.h")
endif()
