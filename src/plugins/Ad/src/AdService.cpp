/* @file Сервис для работы с рекламой. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICryptService.h>

// System
#include <AdBackend/Campaign.h>
#include <AdBackend/Client.h>
#include <AdBackend/DatabaseUtils.h>
#include <Services/ServiceNames.h>
#include <SysUtils/ISysUtils.h>
#include "System/IApplication.h"
#include "System/SettingsConstants.h"

// Project
#include "AdService.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CAdService {
    /// Название лога.
    const QString LogName = "Ad";

    /// Путь к данным рекламы по умолчанию
    const QString DefaultContentPath = "user/ad";

    /// Путь к данным рекламы по умолчанию
    const QString DefaultUrl = "https://service.humo.tj/ad";

    /// Имя файла настроек
    const QString SettingsName = "ad.ini";

    /// Имена параметров в ini, описывающие рекламную кампанию
    namespace Settings {
        const QString Url = "ad/Url";
        const QString ContendPath = "ad/path";
    } // namespace Settings
} // namespace CAdService

//---------------------------------------------------------------------------
AdService *AdService::instance(IApplication *aApplication) {
    return static_cast<AdService *>(aApplication->getCore()->getService(CServices::AdService));
}

//---------------------------------------------------------------------------
AdService::AdService(IApplication *aApplication)
    : mApplication(aApplication), ILogable(CAdService::LogName), mSettings(nullptr) {
    QString userPath =
        IApplication::toAbsolutePath(mApplication->getSettings().value(CSettings::UserDataPath).toString());
    mSettings = new QSettings(ISysUtils::rmBOM(userPath + QDir::separator() + CAdService::SettingsName),
                              QSettings::IniFormat, this);
    mSettings->setIniCodec("utf-8");
}

//---------------------------------------------------------------------------
AdService::~AdService() {
}

//---------------------------------------------------------------------------
bool AdService::initialize() {
    QString userPath =
        IApplication::toAbsolutePath(mApplication->getSettings().value(CSettings::UserDataPath).toString());
    mDatabase = QSharedPointer<Ad::DatabaseUtils>(new Ad::DatabaseUtils(userPath, getLog()));
    // mSettings->value(CAdService::Settings::Url, CAdService::DefaultUrl).toUrl(),
    // 		mApplication->getWorkingDirectory() + QDir::separator() +
    // 			mSettings->value(CAdService::Settings::ContendPath, CAdService::DefaultContentPath).toString(),
    // 		mDatabase.data(),
    mClient = QSharedPointer<Ad::Client>(new Ad::Client(mApplication->getCore(), getLog(), 0));

    mDatabase->addStatisticRecord(-1, "terminal_started");

    return true;
}

//------------------------------------------------------------------------------
void AdService::finishInitialize() {
}

//---------------------------------------------------------------------------
bool AdService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool AdService::shutdown() {
    mClient.clear();
    mDatabase.clear();

    return true;
}

//---------------------------------------------------------------------------
QString AdService::getName() const {
    return CServices::AdService;
}

//---------------------------------------------------------------------------
const QSet<QString> &AdService::getRequiredServices() const {
    static QSet<QString> requiredServices = QSet<QString>() << CServices::SettingsService;

    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap AdService::getParameters() const {
    return QVariantMap();
}

//---------------------------------------------------------------------------
void AdService::resetParameters(const QSet<QString> &) {
}

//---------------------------------------------------------------------------
QVariant AdService::getContent(const QString &aName) const {
    //  auto channel = mClient->channel(aName); // if API is channel()
    // if (!channel || channel->isExpired())
    //     return QVariant();

    return mClient->getContent(aName); // adjust to actual accessor
}

//---------------------------------------------------------------------------
void AdService::addEvent(const QString &aName) {
    // TODO
}

//---------------------------------------------------------------------------
