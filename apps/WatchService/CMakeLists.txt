# WatchService (watchdog) application

file(GLOB WATCHSERVICE_SOURCES
    src/*.cpp
    src/*.h
)

# Remove platform-specific process enumerator implementations
list(REMOVE_ITEM WATCHSERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/win_processenumerator.cpp)
list(REMOVE_ITEM WATCHSERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/linux_processenumerator.cpp)
list(REMOVE_ITEM WATCHSERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/mac_processenumerator.cpp)

# Add the appropriate platform-specific implementation
if(WIN32)
    list(APPEND WATCHSERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/win_processenumerator.cpp)
elseif(APPLE)
    list(APPEND WATCHSERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/mac_processenumerator.cpp)
else()
    list(APPEND WATCHSERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/linux_processenumerator.cpp)
endif()

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

ek_add_application(watchdog
    FOLDER "apps"
    SOURCES
    ${WATCHSERVICE_SOURCES}
    ${WATCHSERVICE_UI_FILES}
    ${WATCHSERVICE_RESOURCES}
    ${WATCHSERVICE_RC_FILES}
    ${WS_VERSION_RC}
    QT_MODULES Gui Widgets
    DEPENDS
    Log
    SysUtils
    DebugUtils
    SettingsManager
    MessageQueue
    PPSDK
)

if(WIN32)
    target_link_libraries(watchdog PRIVATE Advapi32 User32 Winspool)
endif()

if(APPLE)
    # Configure as GUI app with custom Info.plist
    set_target_properties(watchdog PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE AppIcon
    )
    target_sources(watchdog PRIVATE src/icons/watchdog-app-icon.icns)

    # Copy icon to bundle Resources
    add_custom_command(TARGET watchdog POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_BUNDLE_DIR:watchdog>/Contents/Resources
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/watchdog-app-icon.icns $<TARGET_BUNDLE_DIR:watchdog>/Contents/Resources/AppIcon.icns
    )
endif()

# Link SingleApplication for single-instance app support (Qt5/Qt6 compatible)
# Uses itay-grudev/SingleApplication: https://github.com/itay-grudev/SingleApplication
if(TARGET SingleApplication::SingleApplication)
    target_link_libraries(watchdog PUBLIC SingleApplication::SingleApplication)
else()
    message(WARNING "SingleApplication not found - single-instance app support disabled")
endif()

# Translations
file(GLOB TS_FILES src/locale/watchdog_*.ts)
if(TS_FILES)
    # Ensure output directory exists
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale)
    ek_add_translations(WatchServiceTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        INSTALL
    )
    if(TARGET WatchServiceTranslations_translations)
        add_dependencies(watchdog WatchServiceTranslations_translations)
        # Copy translation files to bin/locale directory
        add_custom_command(TARGET watchdog POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/locale
            ${CMAKE_BINARY_DIR}/bin/locale
            COMMENT "Copying translation files to bin/locale directory"
        )
    endif()
endif()

# Генерация watchdog.ini для всех конфигураций
include(${CMAKE_SOURCE_DIR}/cmake/EKIniTemplate.cmake)
ek_generate_ini_for_all_configs(watchdog "${CMAKE_SOURCE_DIR}/runtimes/common/data/watchdog.ini.in")
