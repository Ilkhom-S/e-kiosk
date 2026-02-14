/* @file Реализация менеджера плагинов. */

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFileInfo>

#include <iostream>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <Common/Version.h>

#include <SysUtils/ISysUtils.h>
#include <WatchServiceClient/Constants.h>

// Plugin SDK
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/PluginLoader.h>

#include "Services/EventService.h"
#include "Services/PluginService.h"
#include "Services/ServiceNames.h"
#include "System/SettingsConstants.h"

namespace CPluginService {
const QString HumoSignerName = "HUMO";
} // namespace CPluginService

//------------------------------------------------------------------------------
PluginService *PluginService::instance(IApplication *aApplication) {
    return dynamic_cast<PluginService *>(
        aApplication->getCore()->getService(CServices::PluginService));
}

//------------------------------------------------------------------------------
PluginService::PluginService(IApplication *aApplication)
    : ILogable("Plugins"), m_Application(aApplication), m_PluginLoader(nullptr) {}

//------------------------------------------------------------------------------
PluginService::~PluginService() = default;

//------------------------------------------------------------------------------
bool PluginService::initialize() {
    m_PluginLoader = new SDK::Plugin::PluginLoader(this);

    m_PluginLoader->addDirectory(m_Application->getPluginPath());
    m_PluginLoader->addDirectory(m_Application->getUserPluginPath());

#ifndef _DEBUG
    // Запускаем background verification для проверки digital signature плагинов.
    m_PluginVerifierSynchronizer.addFuture(QtConcurrent::run([this]() {
        verifyPlugins();
        return;
    }));
#endif

    return true;
}

//------------------------------------------------------------------------------
void PluginService::finishInitialize() {}

//---------------------------------------------------------------------------
bool PluginService::canShutdown() {
    return true;
}

//------------------------------------------------------------------------------
#ifndef SM_SHUTTINGDOWN
constexpr int KSystemMetricShuttingDown = 0x2000;
#endif

//------------------------------------------------------------------------------
bool PluginService::shutdown() {
    // Не выгружаем libraries на выходе из ПО в процессе перезагрузки системы. #48972
#ifdef Q_OS_WIN
    if (GetSystemMetrics(KSystemMetricShuttingDown) == 0) {
#endif
        toLog(LogLevel::Debug, "Destroy plugins loader...");

        delete dynamic_cast<SDK::Plugin::PluginLoader *>(m_PluginLoader);
#ifdef Q_OS_WIN
    }
#endif

    m_PluginLoader = nullptr;

    toLog(LogLevel::Debug, "Plugin service shutdown OK.");

    return true;
}

//------------------------------------------------------------------------------
QString PluginService::getName() const {
    return CServices::PluginService;
}

//------------------------------------------------------------------------------
const QSet<QString> &PluginService::getRequiredServices() const {
    static QSet<QString> requiredResources;
    return requiredResources;
}

//------------------------------------------------------------------------------
QVariantMap PluginService::getParameters() const {
    return {};
}

//------------------------------------------------------------------------------
void PluginService::resetParameters(const QSet<QString> & /*aParameters*/) {}

//------------------------------------------------------------------------------
QString PluginService::getState() const {
    QStringList result;

    if (m_UnsignedPlugins.count() != 0) {
        result << QString("Unsigned : {%1}").arg(m_UnsignedPlugins.join(";"));
    }

    QStringList signedKeys = m_SignedPlugins.keys();
    signedKeys.removeDuplicates();

    foreach (QString signerName, signedKeys) {
        QStringList pluginsForSigner;
        for (auto it = m_SignedPlugins.constBegin(); it != m_SignedPlugins.constEnd(); ++it) {
            if (it.key() == signerName) {
                pluginsForSigner << it.value();
            }
        }
        result << QString("Signed by %1 : {%2}").arg(signerName).arg(pluginsForSigner.join(";"));
    }

    return result.join(";");
}

//------------------------------------------------------------------------------
SDK::Plugin::IPluginLoader *PluginService::getPluginLoader() {
    return m_PluginLoader;
}

//------------------------------------------------------------------------------
ILog *PluginService::getLog(const QString &aName) const {
    return aName.isEmpty() ? ILogable::getLog() : ILog::getInstance(aName);
}

//------------------------------------------------------------------------------
QString PluginService::getVersion() const {
    return Humo::getVersion();
}

//------------------------------------------------------------------------------
QString PluginService::getDirectory() const {
    return IApplication::getWorkingDirectory();
}

//------------------------------------------------------------------------------
QString PluginService::getDataDirectory() const {
    return m_Application->getUserDataPath();
}

//------------------------------------------------------------------------------
QString PluginService::getLogsDirectory() const {
    return IApplication::getWorkingDirectory() + "/logs";
}

//------------------------------------------------------------------------------
QString PluginService::getPluginDirectory() const {
    return m_Application->getPluginPath();
}

//------------------------------------------------------------------------------
bool PluginService::canConfigurePlugin(const QString & /*aInstancePath*/) const {
    // Not implemented.
    return false;
}

//------------------------------------------------------------------------------
QVariantMap PluginService::getPluginConfiguration(const QString & /*aInstancePath*/) const {
    // Not implemented.
    return {};
}

//------------------------------------------------------------------------------
bool PluginService::canSavePluginConfiguration(const QString & /*aInstancePath*/) const {
    // Not implemented.
    return false;
}

//------------------------------------------------------------------------------
bool PluginService::savePluginConfiguration(const QString & /*aInstancePath*/,
                                            const QVariantMap & /*aParameters*/) {
    // Not implemented.
    return false;
}

//------------------------------------------------------------------------------
SDK::Plugin::IExternalInterface *PluginService::getInterface(const QString &aInterface) {
    toLog(LogLevel::Debug, QString("getInterface called for: %1").arg(aInterface));

    if (aInterface == SDK::PaymentProcessor::CInterfaces::ICore) {
        if (!m_Application) {
            toLog(LogLevel::Error, "m_Application is null");
            return nullptr;
        }

        auto *core = m_Application->getCore();
        if (!core) {
            toLog(LogLevel::Error, "getCore() returned nullptr");
            return nullptr;
        }

        // ServiceController наследует ICore и IExternalInterface из разных hierarchies.
        // Используем reinterpret_cast для конвертации между ними (safe, так как object type
        // известен).
        auto *result = reinterpret_cast<SDK::Plugin::IExternalInterface *>(core);
        return result;
    }

    toLog(LogLevel::Warning, QString("Interface not supported: %1").arg(aInterface));
    throw SDK::PaymentProcessor::ServiceIsNotImplemented(aInterface);
}

//------------------------------------------------------------------------------
void PluginService::verifyPlugins() {
#ifdef Q_OS_WIN
    m_SignedPlugins.clear();
    m_UnsignedPlugins.clear();

    auto shortPath = [=](const QString &aFullPath) -> QString {
        // Удаляем extension.
        QString result = aFullPath.left(aFullPath.length() - 4).toLower();

        // Удаляем path к plugin/instance.
        return result.contains(m_Application->getUserPluginPath().toLower())
                   ? result.mid(m_Application->getUserPluginPath().length()) + ".u"
                   : result.mid(QString(IApplication::getWorkingDirectory() + QDir::separator() +
                                        (result.contains("plugins") ? "plugins" : ""))
                                    .length());
    };

    QStringList modules = m_PluginLoader->getPluginPathList(QRegularExpression(".*"));

    // Добавляем проверку executable файлов.

    QStringList exeModules = QStringList() << CWatchService::Modules::WatchService
                                           << CWatchService::Modules::PaymentProcessor
                                           << CWatchService::Modules::Updater
                                           << CWatchService::Modules::WatchServiceController;

    foreach (QString module, exeModules) {
        QString file = QString("%1%2%3.exe")
                           .arg(IApplication::getWorkingDirectory())
                           .arg(QDir::separator())
                           .arg(module);

        if (QFileInfo(file).exists()) {
            modules.append(file);
        }
    }

    foreach (QString fullPath, modules) {
        QString plugin = QDir::toNativeSeparators(fullPath).split(QDir::separator()).last();
        qlonglong status = ISysUtils::verifyTrust(QDir::toNativeSeparators(fullPath));

        toLog(LogLevel::Normal, QString("Verifying %1...").arg(fullPath));

        if (status == ERROR_SUCCESS) {
            ISysUtils::SSignerInfo signer;
            bool result = ISysUtils::getSignerInfo(QDir::toNativeSeparators(fullPath), signer);

            if (result) {
                if (signer.name != CPluginService::HumoSignerName) {
                    m_SignedPlugins.insertMulti(signer.name, shortPath(fullPath));
                }

                toLog(LogLevel::Normal, QString("Signed. Subject name: %1").arg(signer.name));
            } else {
                toLog(LogLevel::Warning, QString("Signed. Subject name is unknown."));

                m_UnsignedPlugins.append(shortPath(fullPath));
            }
        } else {
            toLog(LogLevel::Warning,
                  QString("%1").arg(status == TRUST_E_NOSIGNATURE
                                        ? "No signature was present in the subject."
                                        : "Could not verify signer in the subject."));

            m_UnsignedPlugins.append(shortPath(fullPath));
        }
    }

    try {
        auto *eventService = EventService::instance(m_Application);

        if (m_UnsignedPlugins.count()) {
            eventService->sendEvent(SDK::PaymentProcessor::Event(
                SDK::PaymentProcessor::EEventType::Warning,
                getName(),
                QString("Unsigned : {%1}").arg(m_UnsignedPlugins.join(";"))));
        }

        QStringList signedKeys = m_SignedPlugins.keys();
        signedKeys.removeDuplicates();

        foreach (QString signerName, signedKeys) {
            eventService->sendEvent(SDK::PaymentProcessor::Event(
                SDK::PaymentProcessor::EEventType::Warning,
                getName(),
                QString("Signed by %1 : {%2}")
                    .arg(signerName)
                    .arg(QStringList(m_SignedPlugins.values(signerName)).join(";"))));
        }
    } catch (SDK::PaymentProcessor::ServiceIsNotImplemented &e) {
        toLog(LogLevel::Error, "Exception occurred while verify plugins.");
    }

#else
#pragma warning "PluginService::verifyPlugins not implemented on this platform."
#endif
}

//------------------------------------------------------------------------------
