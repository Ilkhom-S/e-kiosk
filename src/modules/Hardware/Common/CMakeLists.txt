# Ensure TC::LibUSB target is available
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
# HardwareCommon module


file(GLOB_RECURSE HARDWARECOMMON_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB PROTOCOLUTILS_SOURCES
    ../Protocols/Common/ProtocolUtils.cpp
    ../Protocols/Common/ProtocolUtils.h
)

file(GLOB HARDWARECOMMON_SDK_HEADERS
    ${TC_INCLUDES_DIR}/SDK/Drivers/HardwareConstants.h
    ${TC_INCLUDES_DIR}/SDK/Drivers/DetectingPriority.h
)

file(GLOB HARDWARECOMMON_PLUGIN_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/Plugins/CommonParameters.h
    ${TC_INCLUDES_DIR}/Hardware/Plugins/DevicePluginBase.h
)

file(GLOB HARDWARECOMMON_PROTO_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/CardReaders/ProtoMifareReader.h
    ${TC_INCLUDES_DIR}/Hardware/FR/ProtoFR.h
    ${TC_INCLUDES_DIR}/Hardware/HID/ProtoHID.h
    ${TC_INCLUDES_DIR}/Hardware/HID/ProtoOPOSScanner.h
    ${TC_INCLUDES_DIR}/Hardware/CashAcceptors/ProtoCashAcceptor.h
    ${TC_INCLUDES_DIR}/Hardware/Dispensers/ProtoDispenser.h
    ${TC_INCLUDES_DIR}/Hardware/Watchdogs/ProtoWatchdog.h
)

file(GLOB HARDWARECOMMON_COMMON_HEADERS
    ${TC_INCLUDES_DIR}/Hardware/Common/*.h
)

set(ALL_SOURCES
    ${HARDWARECOMMON_SOURCES}
    ${PROTOCOLUTILS_SOURCES}
    ${HARDWARECOMMON_SDK_HEADERS}
    ${HARDWARECOMMON_PLUGIN_HEADERS}
    ${HARDWARECOMMON_PROTO_HEADERS}
    ${HARDWARECOMMON_COMMON_HEADERS}
)

tc_add_library(HardwareCommon
    SOURCES ${ALL_SOURCES}
    QT_MODULES Core
    INCLUDE_DIRS
    ${TC_INCLUDES_DIR}/Hardware
    ${TC_INCLUDES_DIR}/SDK/Drivers
)



# Use TC::LibUSB target from thirdparty helper
target_link_libraries(HardwareCommon PRIVATE TC::LibUSB)

# Add translations target if Qt LinguistTools is available
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 COMPONENTS LinguistTools QUIET)
    if(Qt5LinguistTools_FOUND)
        file(GLOB TS_FILES src/locale/*.ts)
        if(TS_FILES)
            qt5_add_translation(QM_FILES ${TS_FILES})
            add_custom_target(HardwareCommonTranslations ALL DEPENDS ${QM_FILES})
            install(FILES ${QM_FILES} DESTINATION locale)
        endif()
    endif()
else()
    find_package(Qt6 COMPONENTS LinguistTools QUIET)
    if(Qt6LinguistTools_FOUND)
        file(GLOB TS_FILES src/locale/*.ts)
        if(TS_FILES)
            qt6_add_translation(QM_FILES ${TS_FILES})
            add_custom_target(HardwareCommonTranslations ALL DEPENDS ${QM_FILES})
            install(FILES ${QM_FILES} DESTINATION locale)
        endif()
    endif()
endif()
