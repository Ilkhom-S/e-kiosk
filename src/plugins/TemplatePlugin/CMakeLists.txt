# TemplatePlugin - minimal plugin template for development

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB TEMPLATEPLUGIN_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(template_plugin
    FOLDER "plugins/Template"
    SOURCES ${TEMPLATEPLUGIN_SOURCES}
    QT_MODULES Core
    DEPENDS PluginsSDK PPSDK
)
