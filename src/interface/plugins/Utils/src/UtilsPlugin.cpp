/* @file Плагин для отрисовки кнопок логотипов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <Common/QtHeadersEnd.h>

// Project
#include "BarcodeProvider.h"
#include "Log.h"
#include "SkinProvider.h"
#include "Utils.h"
#include "UtilsPlugin.h"

void UtilsPlugin::registerTypes(const char * /*aUri*/) {
}

//------------------------------------------------------------------------------
void UtilsPlugin::initializeEngine(QQmlEngine *aEngine, const char * /*aUri*/) {
    QObject *application = aEngine->rootContext()->contextProperty("Core").value<QObject *>();
    QString logoPath = ".";
    QString userLogoPath = ".";
    QString interfacePath = ".";

    if (application) {
        Log::initialize(application);

        logoPath = application->property("environment")
                       .value<QObject *>()
                       ->property("terminal")
                       .value<QObject *>()
                       ->property("contentPath")
                       .toString();

        userLogoPath = application->property("environment")
                           .value<QObject *>()
                           ->property("terminal")
                           .value<QObject *>()
                           ->property("dataPath")
                           .toString();

        interfacePath = application->property("environment")
                            .value<QObject *>()
                            ->property("terminal")
                            .value<QObject *>()
                            ->property("interfacePath")
                            .toString();
    }

    Utils *utils = new Utils(aEngine, interfacePath, userLogoPath);

    aEngine->rootContext()->setContextProperty("Utils", utils);

    SkinProvider *sp = new SkinProvider(interfacePath, logoPath, userLogoPath, dynamic_cast<Skin *>(utils->getSkin()));

    aEngine->addImageProvider("ui", sp);

    aEngine->addImageProvider("barcode", new BarcodeProvider());
}

//------------------------------------------------------------------------------
