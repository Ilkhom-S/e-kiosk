# UpdaterSplashScreen application

include(${CMAKE_SOURCE_DIR}/cmake/EKApplication.cmake)

file(GLOB_RECURSE SPLASH_SOURCES src/*.cpp)
file(GLOB_RECURSE SPLASH_HEADERS src/*.h)
file(GLOB_RECURSE SPLASH_UI_FILES src/*.ui)
file(GLOB SPLASH_RESOURCES src/*.qrc)

if(QT_VERSION_MAJOR EQUAL 5)
    set(SPLASH_QT_MODULES Core Gui Widgets)
else()
    set(SPLASH_QT_MODULES Core Gui Widgets)
endif()

ek_add_application(updatersplash
    FOLDER "apps"
    SOURCES
    ${SPLASH_SOURCES}
    ${SPLASH_HEADERS}
    ${SPLASH_UI_FILES}
    ${SPLASH_RESOURCES}
    QT_MODULES ${SPLASH_QT_MODULES}
    DEPENDS
    App
    ek_common
    INCLUDE_DIRS src
)

if(WIN32)
    target_link_libraries(updatersplash PRIVATE User32)
endif()

# Генерация updatersplash.ini для всех конфигураций
include(${CMAKE_SOURCE_DIR}/cmake/EKIniTemplate.cmake)
ek_generate_ini_for_all_configs(updatersplash "${CMAKE_SOURCE_DIR}/runtimes/common/data/updatersplash/updatersplash.ini.in")

# Make updatersplash depend on ini file generation (only built when updatersplash is built)
add_dependencies(updatersplash updatersplash_ini)
