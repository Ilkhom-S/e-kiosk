
# Updater application for EKiosk

include(${CMAKE_SOURCE_DIR}/cmake/EKApplication.cmake)

file(GLOB_RECURSE UPDATER_SOURCES src/*.cpp)
file(GLOB_RECURSE UPDATER_HEADERS src/*.h)
file(GLOB_RECURSE UPDATER_UI_FILES src/*.ui)
file(GLOB UPDATER_RESOURCES src/*.qrc)
file(GLOB UPDATER_RC_FILES src/*.rc)

set(UPDATER_VERSION_RC ${EK_INCLUDES_DIR}/Common/CoreVersion.rc)

# XmlPatterns is deprecated in Qt6
if(QT_VERSION_MAJOR EQUAL 5)
    set(UPDATER_QT_MODULES Core Gui Widgets XmlPatterns)
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
    App
    Log
    SysUtils
    DebugUtils
    WatchServiceClient
    MessageQueue
    UpdateEngine
    Packer
    INCLUDE_DIRS src
)

if(WIN32)
    target_link_libraries(updater PRIVATE Advapi32 User32)
endif()

# Link TaskScheduler for Windows Task Scheduler integration
target_link_libraries(updater PRIVATE TaskSchedulerPhishMe)

# Link SingleApplication for single-instance app support (Qt5/Qt6 compatible)
if(TARGET SingleApplication::SingleApplication)
    target_link_libraries(updater PUBLIC SingleApplication::SingleApplication)
else()
    message(WARNING "SingleApplication not found - single-instance app support disabled")
endif()

# Translations
file(GLOB TS_FILES src/locale/*.ts)
if(TS_FILES)
    ek_add_translations(UpdaterTranslations
        SOURCES ${TS_FILES}
    )
    add_dependencies(updater UpdaterTranslations)
endif()
