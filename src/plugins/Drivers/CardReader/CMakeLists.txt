# CardReader driver plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB CARDREADER_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(card_readers
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
