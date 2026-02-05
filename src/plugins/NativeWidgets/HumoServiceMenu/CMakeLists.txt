# HumoServiceMenu native widget plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

file(GLOB_RECURSE HUMOSERVICEMENU_SOURCES
    src/*.cpp
    src/*.h
)

set(HUMOSERVICEMENU_UI_FILES
    src/FirstSetup/WelcomeWizardPage.ui
    src/FirstSetup/NetworkWizardPage.ui
    src/FirstSetup/SaveSettingsWizardPage.ui
    src/FirstSetup/WizardFrame.ui
    src/AutoEncashment/AutoEncashmentWindow.ui
    src/VirtualKeyboard/KeyboardWindow.ui
    src/GUI/MainHumoServiceMenuWindow.ui
    src/GUI/MainServiceWindow.ui
    src/GUI/HardwareWindow.ui
    src/GUI/TokenWindow.ui
    src/GUI/KeysWindow.ui
    src/GUI/HardwareServiceWindow.ui
    src/GUI/NetworkServiceWindow.ui
    src/GUI/PaymentServiceWindow.ui
    src/GUI/DispenserServiceWindow.ui
    src/GUI/DialupConnectionWindow.ui
    src/GUI/DeviceSlot.ui
    src/GUI/KeysServiceWindow.ui
    src/GUI/SetupServiceWindow.ui
    src/GUI/EncashmentHistoryWindow.ui
    src/GUI/TokenServiceWindow.ui
    src/GUI/EncashmentServiceWindow.ui
    src/GUI/DiagnosticsServiceWindow.ui
    src/GUI/UnmanagedConnectionWindow.ui
    src/GUI/LogsServiceWindow.ui
    src/GUI/EditorPane.ui
    src/GUI/DeviceStatusWindow.ui
    src/GUI/InputBox.ui
    src/GUI/MessageBox/MessageWindow.ui
)

file(GLOB_RECURSE HUMOSERVICEMENU_RESOURCES
    src/*.qrc
)

set(VERSION_RC ${EK_INCLUDES_DIR}/Common/CoreVersion.rc)

ek_add_plugin(humo_service_menu
    FOLDER "plugins/NativeWidgets"
    SOURCES ${HUMOSERVICEMENU_SOURCES} ${HUMOSERVICEMENU_UI_FILES} ${HUMOSERVICEMENU_RESOURCES}
    QT_MODULES Core Gui Widgets Network
    DEPENDS
    BasicApplication
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

# Translations
file(GLOB TS_FILES src/Locale/*.ts)
if(TS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
    ek_add_translations(humo_service_menu
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/locale
    )
endif()
