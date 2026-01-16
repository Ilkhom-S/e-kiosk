# ServiceMenu native widget plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

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

ek_add_plugin(service_menu
    FOLDER "plugins"
    SOURCES ${SERVICEMENU_SOURCES} ${SERVICEMENU_UI_FILES} ${SERVICEMENU_RESOURCES}
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
    ek_boost
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    INSTALL_DIR plugins
)

if(WIN32)
    target_link_libraries(service_menu PRIVATE Advapi32 Rasapi32)
endif()

# Translations
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(ServiceMenuTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
    )
endif()
