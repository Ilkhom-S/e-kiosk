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
    FOLDER "plugins/ScenarioBackends/Uniteller"
    SOURCES ${UNITELLER_SOURCES} ${UNITELLER_RESOURCES}
    QT_MODULES Core Network
    DEPENDS
    SysUtils
    Log
    NetworkTaskManager
    ScenarioEngine
    PaymentBase
)

# Translations
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(UnitellerTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
    )
endif()
