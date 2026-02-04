
# Updater application for EKiosk

include(${CMAKE_SOURCE_DIR}/cmake/EKApplication.cmake)

file(GLOB_RECURSE UPDATER_SOURCES src/*.cpp)
file(GLOB_RECURSE UPDATER_HEADERS src/*.h)
file(GLOB_RECURSE UPDATER_UI_FILES src/*.ui)
file(GLOB UPDATER_RESOURCES src/*.qrc)
file(GLOB UPDATER_RC_FILES src/*.rc)

set(UPDATER_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)

# XmlPatterns is deprecated in Qt6
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 COMPONENTS XmlPatterns QUIET)
    if(TARGET Qt5::XmlPatterns)
        set(UPDATER_QT_MODULES Core Gui Widgets XmlPatterns)
    else()
        set(UPDATER_QT_MODULES Core Gui Widgets)
    endif()
else()
    set(UPDATER_QT_MODULES Core Gui Widgets)
endif()

ek_add_application(updater
    FOLDER "apps"
    SOURCES
    ${UPDATER_SOURCES}
    ${UPDATER_HEADERS}
    ${UPDATER_UI_FILES}
    ${UPDATER_RESOURCES}
    ${UPDATER_RC_FILES}
    ${UPDATER_VERSION_RC}
    QT_MODULES ${UPDATER_QT_MODULES}
    DEPENDS
    Log
    SysUtils
    DebugUtils
    WatchServiceClient
    MessageQueue
    UpdateEngine
    Packer
    INCLUDE_DIRS src
)

if(APPLE)
    # Configure as standard application that shows in dock
    set_target_properties(updater PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE updater-app-icon.icns
    )
    # Copy icon to bundle Resources
    add_custom_command(TARGET updater POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_BUNDLE_DIR:updater>/Contents/Resources
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/updater-app-icon.icns $<TARGET_BUNDLE_DIR:updater>/Contents/Resources/AppIcon.icns
    )
endif()

if(WIN32)
    target_link_libraries(updater PRIVATE Advapi32 User32)
endif()

# Translations
file(GLOB TS_FILES src/locale/updater_*.ts)
if(TS_FILES)
    # Ensure output directory exists
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale)
    ek_add_translations(UpdaterTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        INSTALL
    )
    if(TARGET UpdaterTranslations_translations)
        add_dependencies(updater UpdaterTranslations_translations)
        # Copy translation files to bin/locale for cross-platform compatibility
        add_custom_command(TARGET updater POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/locale
            ${CMAKE_BINARY_DIR}/bin/locale
            COMMENT "Copying translation files to bin/locale for cross-platform compatibility"
        )
    endif()
endif()

# Link SingleApplication for single-instance app support (Qt5/Qt6 compatible)
if(TARGET SingleApplication::SingleApplication)
    target_link_libraries(updater PUBLIC SingleApplication::SingleApplication)
else()
    message(WARNING "SingleApplication not found - single-instance app support disabled")
endif()

# Translations
file(GLOB TS_FILES src/locale/updater_*.ts)
if(TS_FILES)
    # Ensure output directory exists
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale)
    ek_add_translations(UpdaterTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        INSTALL
    )
    if(TARGET UpdaterTranslations_translations)
        add_dependencies(updater UpdaterTranslations_translations)
        # Copy translation files to bin/locale directory
        add_custom_command(TARGET updater POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/locale
            ${CMAKE_BINARY_DIR}/bin/locale
            COMMENT "Copying translation files to bin/locale directory"
        )
    endif()
endif()

# Генерация updater.ini для всех конфигураций
include(${CMAKE_SOURCE_DIR}/cmake/EKIniTemplate.cmake)
ek_generate_ini_for_all_configs(updater "${CMAKE_SOURCE_DIR}/runtimes/common/data/updater.ini.in")
