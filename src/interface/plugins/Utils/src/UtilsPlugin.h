/* @file Плагин для отрисовки кнопок логотипов. */

#pragma once

#include <QtQml/QQmlExtensionPlugin>

class QQmlEngine;

//------------------------------------------------------------------------------
class UtilsPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.humo.graphics.utils")

public:
    virtual void registerTypes(const char *aUri);
    virtual void initializeEngine(QQmlEngine *aEngine, const char *aUri);
};

//------------------------------------------------------------------------------
