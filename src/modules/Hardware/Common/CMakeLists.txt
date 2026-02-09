# HardwareCommon module

include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)


file(GLOB_RECURSE HARDWARECOMMON_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB PROTOCOLUTILS_SOURCES
    ../Protocols/Common/src/ProtocolUtils.cpp
    ../Protocols/Common/src/ProtocolUtils.h
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
    ${CMAKE_SOURCE_DIR}/include/Hardware/Common/*.tpp
    ${CMAKE_SOURCE_DIR}/include/Hardware/Protocols/Common/*.tpp
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
    FOLDER "modules/Hardware"
    SOURCES ${ALL_SOURCES}
    QT_MODULES Core
    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include/Hardware
    ${CMAKE_SOURCE_DIR}/include/SDK/Drivers
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ek_common
    COMPILE_DEFINITIONS _UNICODE UNICODE
)

# Add translations using the EK helper
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(HardwareCommonTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        INSTALL
        FOLDER "modules/Hardware"
    )
endif()

# Ensure EK::LibUSB target is available for this library
include(${CMAKE_SOURCE_DIR}/thirdparty/cmake/TCFindLibUSB.cmake)
target_link_libraries(HardwareCommon PRIVATE EK::LibUSB)
