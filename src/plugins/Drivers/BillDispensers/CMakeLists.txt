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

ek_add_driver_plugin(bill_dispensers
    FOLDER "plugins/Drivers"
    SOURCES ${BILLDISPENSERS_SOURCES}
    DEPENDS
    HardwareProtocols
    HardwareCommon
    CashDispensers
)

# Translations - merge bill dispensers and common translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins/drivers)
    ek_add_translations(bill_dispensers
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/plugins/drivers
    )
endif()
