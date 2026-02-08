/* @file Плагин для проигрывателя flash */

#pragma once

#include <QtDeclarative/QDeclarativeExtensionPlugin>

#include "FlashPlayerItem.h"

//--------------------------------------------------------------------------
class FlashPlayerItem_Plugin : public QDeclarativeExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Humo.GraphicsItems.FlashPlayer")

public:
    virtual void registerTypes(const char *aUri) {
        qmlRegisterType<FlashPlayerItem>(aUri, 1, 0, "FlashPlayer");
    }
};

//--------------------------------------------------------------------------
