# WatchServiceController (tray) application - refactored to EKiosk CMake helpers

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

# Use EK_INCLUDES_DIR when available, fall back to legacy TC_INCLUDES_DIR if present
set(WSC_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)

ek_add_application(tray
    FOLDER "apps"
    SOURCES
    ${WSC_SOURCES}
    ${WSC_RESOURCES}
    ${WSC_RC_FILES}
    ${WSC_VERSION_RC}
    QT_MODULES Widgets
    DEPENDS
    App
    Log
    SysUtils
    DebugUtils
    WatchServiceClient
    MessageQueue
)

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
    )
    if(TARGET WatchServiceControllerTranslations)
        add_dependencies(tray WatchServiceControllerTranslations)
    endif()
endif()
