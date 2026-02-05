# CardReader driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB CARDREADER_SOURCES
    src/*.cpp
    src/*.h
    ../../../includes/Common/CoreVersion.rc
    ../../../includes/Hardware/CardReaders/CardReaderStatusesDescriptions.h
)

# Exclude Windows-only USB card readers on non-Windows platforms
if(NOT WIN32)
    list(REMOVE_ITEM CARDREADER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MifareReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MifareReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MifareReaderModelData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PCSCReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PCSCReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PCSCReaderStatusCodes.h
    )
endif()

ek_add_driver_plugin(card_readers
    SOURCES ${CARDREADER_SOURCES}
    DEPENDS
    HardwareCommon
    Cardreaders
    HardwareIOPorts
    SysUtils
    # IDTech_SDK # Uncomment if IDTech SDK is used
)

if(WIN32)
    target_link_libraries(card_readers PRIVATE Setupapi)
endif()

# Translations - merge common and plugin-specific translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
    ek_add_translations(card_readers
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/locale
    )
endif()
