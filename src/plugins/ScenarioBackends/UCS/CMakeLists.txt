# UCS scenario backend plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB UCS_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB UCS_RESOURCES
    src/*.qrc
)

ek_add_plugin(ucs
    FOLDER "plugins/ScenarioBackends"
    SOURCES ${UCS_SOURCES} ${UCS_RESOURCES}
    QT_MODULES Core Network
    DEPENDS
    SysUtils
    Log
    NetworkTaskManager
    ScenarioEngine
    PaymentBase
)

# Translations - merge UCS and common translations
file(GLOB TS_FILES_PLUGIN src/Locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
    ek_add_translations(ucs
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/locale
    )
endif()
