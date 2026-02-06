# Modem driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB MODEM_SOURCES
    src/*.cpp
    src/*.h
    ../../../includes/Common/CoreVersion.rc
    ../../../includes/Hardware/Modems/ModemStatusesDescriptions.h
)

ek_add_driver_plugin(modems
    SOURCES ${MODEM_SOURCES}
    DEPENDS
    DriversSDK
    HardwareCommon
    HardwareModems
)

# Translations - merge modem and common translations
file(GLOB TS_FILES_MODEM src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_MODEM} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
    ek_add_translations(modems
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/plugins/drivers
    )
endif()
