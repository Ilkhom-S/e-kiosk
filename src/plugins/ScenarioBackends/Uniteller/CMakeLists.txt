# Uniteller scenario backend plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB UNITELLER_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB UNITELLER_RESOURCES
    src/*.qrc
)

ek_add_plugin(uniteller
    FOLDER "plugins/ScenarioBackends"
    SOURCES ${UNITELLER_SOURCES} ${UNITELLER_RESOURCES}
    QT_MODULES Core Network
    DEPENDS
    SysUtils
    Log
    NetworkTaskManager
    ScenarioEngine
    PaymentBase
)

# Translations - merge Uniteller and common translations
file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
    ek_add_translations(uniteller
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/plugins
    )
endif()
