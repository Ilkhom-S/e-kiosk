# WatchServiceController (tray) application - refactored to EKiosk CMake helpers


include_directories(${CMAKE_SOURCE_DIR}/thirdparty/SingleApplication)
# Генерация ini-файлов для всех конфигураций через хелпер
ek_generate_ini_for_all_configs(tray "${CMAKE_SOURCE_DIR}/runtimes/common/data/tray.ini.in")

file(GLOB WSC_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB WSC_RESOURCES
    src/*.qrc
)

file(GLOB WSC_RC_FILES
    src/*.rc
)

set(WSC_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)


# App includes sources directly
ek_add_application(tray
    FOLDER "apps"
    SOURCES
    ${WSC_SOURCES}
    src/main.cpp
    ${WSC_RESOURCES}
    ${WSC_RC_FILES}
    ${WSC_VERSION_RC}
    QT_MODULES Widgets
    DEPENDS WatchServiceClient MessageQueue SysUtils DebugUtils
)

if(APPLE)
    # Configure as background app that only shows in menu bar, not dock
    set_target_properties(tray PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE tray-app-icon.icns
    )
    # No dock icon for background apps
    target_sources(tray PRIVATE src/icons/tray-app-icon.icns)

    # Compile WatchServiceController.cpp as Objective-C++ for macOS theme support
    set_source_files_properties(src/WatchServiceController.cpp PROPERTIES LANGUAGE OBJCXX)

    # Copy icon to bundle Resources
    add_custom_command(TARGET tray POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_BUNDLE_DIR:tray>/Contents/Resources
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/tray-app-icon.icns $<TARGET_BUNDLE_DIR:tray>/Contents/Resources/AppIcon.icns
    )
endif()

# Optional single-instance support via project SingleApplication wrapper (EK::SingleApplication)
if(TARGET EK::SingleApplication)
    set(QAPPLICATION_CLASS QGuiApplication CACHE STRING "Inheritance class for SingleApplication")
    target_link_libraries(tray PUBLIC EK::SingleApplication)
else()
    message(WARNING "EK::SingleApplication not found - single-instance app support disabled")
endif()

# Translations
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(WatchServiceControllerTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        INSTALL
    )
    if(TARGET WatchServiceControllerTranslations_translations)
        add_dependencies(tray WatchServiceControllerTranslations_translations)
        # Copy translation files to executable directory for debugging
        add_custom_command(TARGET tray POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/locale
            $<TARGET_FILE_DIR:tray>/locale
            COMMENT "Copying translation files to executable directory"
        )
    endif()
endif()
