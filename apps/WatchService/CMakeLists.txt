# WatchService (guard) application

file(GLOB WATCHSERVICE_SOURCES
    src/*.cpp
    src/*.h
)

if(NOT WIN32)
    list(REMOVE_ITEM WATCHSERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/win_processenumerator.cpp)
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

if(APPLE)
    # Configure as GUI app with icon
    set_target_properties(guard PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE guard-app-icon.icns
    )
    target_sources(guard PRIVATE src/icons/guard-app-icon.icns)

    # Copy icon to bundle Resources
    add_custom_command(TARGET guard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_BUNDLE_DIR:guard>/Contents/Resources
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/guard-app-icon.icns $<TARGET_BUNDLE_DIR:guard>/Contents/Resources/AppIcon.icns
    )
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
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        INSTALL
    )
    if(TARGET WatchServiceTranslations_translations)
        add_dependencies(guard WatchServiceTranslations_translations)
        # Copy translation files to executable directory for debugging
        add_custom_command(TARGET guard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/locale
            $<TARGET_FILE_DIR:guard>/locale
            COMMENT "Copying translation files to executable directory"
        )
    endif()
endif()

# Генерация guard.ini для всех конфигураций
include(${CMAKE_SOURCE_DIR}/cmake/EKIniTemplate.cmake)
ek_generate_ini_for_all_configs(guard "${CMAKE_SOURCE_DIR}/runtimes/common/data/guard.ini.in")
