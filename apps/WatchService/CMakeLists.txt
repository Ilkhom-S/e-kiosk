# WatchService (guard) application

file(GLOB WATCHSERVICE_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB WATCHSERVICE_UI_FILES
    src/*.ui
)

file(GLOB WATCHSERVICE_RESOURCES
    src/*.qrc
)

file(GLOB WATCHSERVICE_RC_FILES
    src/*.rc
)

set(WS_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)

ek_add_application(guard
    FOLDER "apps"
    SOURCES
    ${WATCHSERVICE_SOURCES}
    ${WATCHSERVICE_UI_FILES}
    ${WATCHSERVICE_RESOURCES}
    ${WATCHSERVICE_RC_FILES}
    ${WS_VERSION_RC}
    QT_MODULES Gui Widgets
    DEPENDS
    App
    Log
    SysUtils
    DebugUtils
    SettingsManager
    MessageQueue
    PPSDK
)

if(WIN32)
    target_link_libraries(guard PRIVATE Advapi32 User32 Winspool)
endif()

# Link SingleApplication for single-instance app support (Qt5/Qt6 compatible)
# Uses itay-grudev/SingleApplication: https://github.com/itay-grudev/SingleApplication
if(TARGET SingleApplication::SingleApplication)
    target_link_libraries(guard PUBLIC SingleApplication::SingleApplication)
else()
    message(WARNING "SingleApplication not found - single-instance app support disabled")
endif()

# Translations
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(WatchServiceTranslations
        SOURCES ${TS_FILES}
    )
    if(TARGET WatchServiceTranslations)
        add_dependencies(guard WatchServiceTranslations)
    endif()
endif()

# Генерация guard.ini для всех конфигураций
include(${CMAKE_SOURCE_DIR}/cmake/EKIniTemplate.cmake)
ek_generate_ini_for_all_configs(guard "${CMAKE_SOURCE_DIR}/runtimes/common/data/guard.ini.in")
