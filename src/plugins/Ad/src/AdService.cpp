/* @file Сервис для работы с рекламой. */

#include "AdService.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QString>

#include <SDK/PaymentProcessor/Core/ICryptService.h>

#include <AdBackend/Campaign.h>
#include <AdBackend/Client.h>
#include <AdBackend/DatabaseUtils.h>
#include <Services/ServiceNames.h>
#include <SysUtils/ISysUtils.h>

#include "System/IApplication.h"
#include "System/SettingsConstants.h"

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
    return dynamic_cast<AdService *>(aApplication->getCore()->getService(CServices::AdService));
}

//---------------------------------------------------------------------------
AdService::AdService(IApplication *aApplication)
    : m_Application(aApplication), ILogable(CAdService::LogName), m_Settings(nullptr) {
    QString userPath = IApplication::toAbsolutePath(
        m_Application->getSettings().value(CSettings::UserDataPath).toString());
    m_Settings =
        new QSettings(ISysUtils::rm_BOM(userPath + QDir::separator() + CAdService::SettingsName),
                      QSettings::IniFormat,
                      this);
    // В Qt6 метод setIniCodec() удален, UTF-8 используется по умолчанию
    // В Qt5 необходимо явно установить кодировку UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_Settings->setIniCodec("utf-8");
#endif
}

//---------------------------------------------------------------------------
AdService::~AdService() = default;

//---------------------------------------------------------------------------
bool AdService::initialize() {
    QString userPath = IApplication::toAbsolutePath(
        m_Application->getSettings().value(CSettings::UserDataPath).toString());
    m_Database = QSharedPointer<Ad::DatabaseUtils>(new Ad::DatabaseUtils(userPath, getLog()));
    // m_Settings->value(CAdService::Settings::Url, CAdService::DefaultUrl).toUrl(),
    // 		m_Application->getWorkingDirectory() + QDir::separator() +
    // 			m_Settings->value(CAdService::Settings::ContendPath,
    // CAdService::DefaultContentPath).toString(), 		m_Database.data(),
    m_Client = QSharedPointer<Ad::Client>(new Ad::Client(m_Application->getCore(), getLog(), 0));

    m_Database->addStatisticRecord(-1, "terminal_started");

    return true;
}

//---------------------------------------------------------------------------
// Закончена инициализация всех сервисов
void AdService::finishInitialize() {}

//---------------------------------------------------------------------------
bool AdService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool AdService::shutdown() {
    m_Client.clear();
    m_Database.clear();

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
    return {};
}

//---------------------------------------------------------------------------
void AdService::resetParameters(const QSet<QString> & /*aParameters*/) {}

//---------------------------------------------------------------------------
QVariant AdService::getContent(const QString &aName) const {
    //  auto channel = m_Client->channel(aName); // if API is channel()
    // if (!channel || channel->isExpired())
    //     return QVariant();

    return m_Client->getContent(aName); // adjust to actual accessor
}

//---------------------------------------------------------------------------
void AdService::addEvent(const QString &aName) {
    // TODO
}

//---------------------------------------------------------------------------
