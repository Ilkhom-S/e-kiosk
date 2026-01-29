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

set(VERSION_RC ${EK_INCLUDES_DIR}/Common/CoreVersion.rc)

ek_add_plugin(service_menu
    FOLDER "plugins"
    SOURCES ${SERVICEMENU_SOURCES} ${SERVICEMENU_UI_FILES} ${SERVICEMENU_RESOURCES}
    QT_MODULES Core Gui Widgets Network
    DEPENDS
    BasicApplication
    Connection
    PluginsSDK
    DriversSDK
    PPSDK
    PPSDKImpl
    NetworkTaskManager
    KeysUtils
    Log
    SysUtils
    ek_boost
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    INSTALL_DIR plugins
)

# Alternative approach: Move UI files to same directories as headers
# This would eliminate the need for AUTOUIC_SEARCH_PATHS but requires reorganizing files

# Or use a more specific glob pattern:
# file(GLOB SERVICEMENU_UI_FILES
#     src/*.ui
#     src/*/*.ui
#     src/*/*/*.ui
#     src/*/*/*/*.ui
# )

# Or set globally for all targets (in main CMakeLists.txt):
# set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/src/plugins/NativeWidgets/ServiceMenu/src/AutoEncashment" CACHE STRING "")

# Set AUTOUIC search paths for UI files in subdirectories
set_target_properties(service_menu PROPERTIES
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/AutoEncashment;${CMAKE_CURRENT_SOURCE_DIR}/src/FirstSetup/UI;${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/MessageBox;${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/UI;${CMAKE_CURRENT_SOURCE_DIR}/src/VirtualKeyboard"
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
