# Uniteller scenario backend plugin

file(GLOB UNITELLER_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB UNITELLER_RESOURCES
    src/*.qrc
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_scenario_backend(uniteller
    SOURCES ${UNITELLER_SOURCES} ${UNITELLER_RESOURCES} ${VERSION_RC}
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
    tc_add_translations(UnitellerTranslations
        SOURCES ${TS_FILES}
    )
endif()
