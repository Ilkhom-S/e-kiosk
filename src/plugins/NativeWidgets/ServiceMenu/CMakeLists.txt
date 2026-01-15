# ServiceMenu native widget plugin

file(GLOB_RECURSE SERVICEMENU_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB_RECURSE SERVICEMENU_UI_FILES
    src/*.ui
)

file(GLOB_RECURSE SERVICEMENU_RESOURCES
    src/*.qrc
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_plugin(service_menu
    SOURCES ${SERVICEMENU_SOURCES} ${SERVICEMENU_UI_FILES} ${SERVICEMENU_RESOURCES} ${VERSION_RC}
    QT_MODULES Core Gui Widgets Network
    DEPENDS
        App
        Connection
        PluginsSDK
        DriversSDK
        PPSDK
        NetworkTaskManager
        KeysUtils
        Log
        SysUtils
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    INSTALL_DIR plugins
)

if(WIN32)
    target_link_libraries(service_menu PRIVATE Advapi32 Rasapi32)
endif()

# Translations
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    tc_add_translations(ServiceMenuTranslations
        SOURCES ${TS_FILES}
    )
endif()
