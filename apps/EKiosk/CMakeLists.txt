

cmake_minimum_required(VERSION 3.16)
project(EKiosk LANGUAGES CXX C)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql PrintSupport Network Multimedia Xml SerialPort WebSockets WebEngineWidgets Core5Compat)

# Use EKiosk CMake helpers
include(${CMAKE_SOURCE_DIR}/cmake/EKApplication.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKStaticAnalysis.cmake)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UI_FILES src/*.ui)
file(GLOB RESOURCE_FILES src/*.qrc)
file(GLOB RC_FILES src/*.rc)

# Exclude Windows-only connection files on non-Windows platforms
if(NOT WIN32)
    list(REMOVE_ITEM SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/connection/RasConnection.cpp
    )
    list(REMOVE_ITEM HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/connection/RasConnection.h
    )
endif()

# Application target with Qt modules and resources
ek_add_application(client
    FOLDER "apps"
    SOURCES ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCE_FILES} ${RC_FILES}
    QT_MODULES Core Widgets Sql PrintSupport Network Multimedia Xml SerialPort WebSockets WebEngineWidgets Core5Compat
    LIBRARIES
    EK::QZint
    PPSDK
    PPSDK
    GUISDK
    PluginsSDK
    DriversSDK
    GraphicsEngine
    ScenarioEngine
    SettingsManager
    DatabaseProxy
    NetworkTaskManager
    Connection
    CryptEngine
    DeviceManager
    PaymentBase
    WatchServiceClient
    UpdateEngine
    MessageQueue
    Packer
    AdBackend
    SysUtils
    KeysUtils
    $<$<PLATFORM_ID:Windows>:rasapi32;winspool;psapi;user32;Advapi32>
)

if(APPLE)
    # Configure as standard application that shows in dock
    set_target_properties(client PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE AppIcon
    )
    # Copy icon to bundle Resources
    add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_BUNDLE_DIR:client>/Contents/Resources
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/client-app-icon.icns $<TARGET_BUNDLE_DIR:client>/Contents/Resources/AppIcon.icns
    )
endif()

# Link common include INTERFACE target so `include/` headers are available
target_link_libraries(client PUBLIC ek_common)

# Add zint include directory for qzint.h header
target_include_directories(client PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/zint/backend_qt")

# Link Boost includes for SDK headers that use Boost
# target_link_libraries(client PUBLIC ek_boost)  # Removed to avoid linking issues

# Option to enable test mode (disables kiosk restrictions, e.g. killing explorer.exe)
option(EKIOSK_TEST_MODE "Enable test mode (no kiosk restrictions)" ON)
if(EKIOSK_TEST_MODE)
    target_compile_definitions(client PRIVATE EKIOSK_TEST_MODE)
endif()

# Static analysis (optional, but recommended)
# ek_enable_static_analysis(client CPPCHECK CLANG_TIDY)

# Translations
file(GLOB TS_FILES src/locale/client_*.ts)
if(TS_FILES)
    # Ensure output directory exists
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale)
    ek_add_translations(EKioskTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        INSTALL
    )
    if(TARGET EKioskTranslations_translations)
        add_dependencies(client EKioskTranslations_translations)
        # Copy translation files to bin/locale directory
        add_custom_command(TARGET client POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/locale
            ${CMAKE_BINARY_DIR}/bin/locale
            COMMENT "Copying translation files to bin/locale directory"
        )
    endif()
endif()
