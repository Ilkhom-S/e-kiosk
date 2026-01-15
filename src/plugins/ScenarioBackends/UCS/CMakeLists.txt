# UCS scenario backend plugin

file(GLOB UCS_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB UCS_RESOURCES
    src/*.qrc
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_scenario_backend(ucs
    SOURCES ${UCS_SOURCES} ${UCS_RESOURCES} ${VERSION_RC}
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
    tc_add_translations(UCSTranslations
        SOURCES ${TS_FILES}
    )
endif()
