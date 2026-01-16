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

# Translations
file(GLOB TS_FILES src/Locale/*.ts)
if(TS_FILES)
    ek_add_translations(UCSTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Locale
    )
endif()
