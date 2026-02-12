# WatchServiceController (controller) application - refactored to EKiosk CMake helpers

include(${CMAKE_SOURCE_DIR}/cmake/EKStaticAnalysis.cmake)

include_directories(${CMAKE_SOURCE_DIR}/thirdparty/SingleApplication)
# Генерация ini-файлов для всех конфигураций через хелпер
ek_generate_ini_for_all_configs(controller "${CMAKE_SOURCE_DIR}/runtimes/common/data/controller/controller.ini.in")

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
ek_add_application(controller
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

# Enable static analysis
ek_enable_static_analysis(controller CLANG_TIDY)

if(APPLE)
    # Configure as background app that only shows in menu bar, not dock
    set_target_properties(controller PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE controller-app-icon.icns
    )
    # No dock icon for background apps
    target_sources(controller PRIVATE src/icons/controller-app-icon.icns)
    set_source_files_properties(src/icons/controller-app-icon.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    # Compile WatchServiceController.cpp as Objective-C++ for macOS theme support
    set_source_files_properties(src/WatchServiceController.cpp PROPERTIES LANGUAGE OBJCXX)
endif()

# Make controller depend on ini file generation (only built when controller is built)
add_dependencies(controller controller_ini)

# Optional single-instance support via project SingleApplication wrapper (EK::SingleApplication)
if(TARGET EK::SingleApplication)
    target_link_libraries(controller PUBLIC EK::SingleApplication)
else()
    message(WARNING "EK::SingleApplication not found - single-instance app support disabled")
endif()

# Translations
file(GLOB TS_FILES src/locale/controller_*.ts)
if(TS_FILES)
    # Ensure output directory exists
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale)
    ek_add_translations(WatchServiceControllerTranslations
        SOURCES ${TS_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        INSTALL
    )
    if(TARGET WatchServiceControllerTranslations_translations)
        add_dependencies(controller WatchServiceControllerTranslations_translations)
        # Copy translation files to bin/locale for cross-platform compatibility
        add_custom_command(TARGET controller POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/locale
            ${CMAKE_BINARY_DIR}/bin/locale
            COMMENT "Copying translation files to bin/locale for cross-platform compatibility"
        )
    endif()
endif()
