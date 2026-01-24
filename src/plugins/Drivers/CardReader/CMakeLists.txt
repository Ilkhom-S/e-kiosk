# CardReader driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB CARDREADER_SOURCES
    src/*.cpp
    src/*.h
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

ek_add_plugin(card_readers
    FOLDER "plugins/Drivers"
    SOURCES ${CARDREADER_SOURCES}
    QT_MODULES Core
    DEPENDS
    HardwareCommon
    Cardreaders
    DriversSDK
    HardwareIOPorts
    PluginsSDK
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)
