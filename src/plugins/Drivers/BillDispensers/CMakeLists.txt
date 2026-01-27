# BillDispensers driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB BILLDISPENSERS_SOURCES
    src/*.cpp
    src/*.h
    ../../../includes/Common/CoreVersion.rc
    ../../../modules/Hardware/CashDevices/src/*.* # CCTalk device base
    ../../../modules/Hardware/CashDevices/src/*/*.* # CCTalk subdirectory files
    ../../../includes/Hardware/Dispensers/DispenserStatusesDescriptions.h
)

ek_add_plugin(bill_dispensers
    FOLDER "plugins/Drivers"
    SOURCES ${BILLDISPENSERS_SOURCES}
    QT_MODULES Core
    DEPENDS
    DriversSDK
    HardwareProtocols
    HardwareCommon
    CashDispensers
    PluginsSDK
    ek_common
)

# Translations - merge common and plugin-specific translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    ek_add_translations(BillDispensersTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
    )
endif()
