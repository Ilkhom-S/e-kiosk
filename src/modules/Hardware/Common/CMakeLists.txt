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
    ${CMAKE_SOURCE_DIR}/include/SDK/Drivers/HardwareConstants.h
    ${CMAKE_SOURCE_DIR}/include/SDK/Drivers/DetectingPriority.h
)

file(GLOB HARDWARECOMMON_PLUGIN_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/Plugins/CommonParameters.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/Plugins/DevicePluginBase.h
)

file(GLOB HARDWARECOMMON_PROTO_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/CardReaders/ProtoMifareReader.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/FR/ProtoFR.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/HID/ProtoHID.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/HID/ProtoOPOSScanner.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/CashAcceptors/ProtoCashAcceptor.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/Dispensers/ProtoDispenser.h
    ${CMAKE_SOURCE_DIR}/include/Hardware/Watchdogs/ProtoWatchdog.h
)

file(GLOB HARDWARECOMMON_COMMON_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Hardware/Common/*.h
)

set(ALL_SOURCES
    ${HARDWARECOMMON_SOURCES}
    ${PROTOCOLUTILS_SOURCES}
    ${HARDWARECOMMON_SDK_HEADERS}
    ${HARDWARECOMMON_PLUGIN_HEADERS}
    ${HARDWARECOMMON_PROTO_HEADERS}
    ${HARDWARECOMMON_COMMON_HEADERS}
)

ek_add_library(HardwareCommon
    SOURCES ${ALL_SOURCES}
    QT_MODULES Core
    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include/Hardware
    ${CMAKE_SOURCE_DIR}/include/SDK/Drivers
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS LibUSB
    COMPILE_DEFINITIONS _UNICODE UNICODE
)

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
