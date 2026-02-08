/* @file Плагин сценария для тестирования оборудования */

#include "MainScenario.h"

#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QPair>
#include <QtCore/QSettings>

#include <Common/ExitCodes.h>

#include <SDK/Drivers/DeviceTypes.h>
#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/IDatabaseService.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>

#include <Crypt/ICryptEngine.h>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>

#include "ScenarioPlugin.h"

namespace {
/// Конструктор плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new Migrator3000::MainScenarioPlugin(aFactory, aInstancePath);
}
} // namespace

REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application,
                         PPSDK::CComponents::ScenarioFactory,
                         CScenarioPlugin::PluginName),
                &CreatePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                Migrator3000MainScenario);

namespace Migrator3000 {

//---------------------------------------------------------------------------
MainScenario::MainScenario(SDK::PaymentProcessor::ICore *aCore, ILog *aLog)
    : Scenario(CScenarioPlugin::PluginName, aLog), m_Core(aCore),
      m_NetworkService(aCore->getNetworkService()), m_TerminalService(aCore->getTerminalService()),
      m_SettingsService(aCore->getSettingsService()), m_CryptService(aCore->getCryptService()),
      m_DeviceService(aCore->getDeviceService()), m_MonitoringComandId(-1) {
    m_TerminalSettings = static_cast<PPSDK::TerminalSettings *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

    Q_ASSERT(m_TerminalSettings);

    m_ScriptingCore = static_cast<SDK::PaymentProcessor::Scripting::Core *>(
        dynamic_cast<SDK::GUI::IGraphicsHost *>(m_Core->getGUIService())
            ->getInterface<QObject>(SDK::PaymentProcessor::Scripting::CProxyNames::Core));

    Q_ASSERT(m_ScriptingCore);

    connect(&m_TaskWatcher, SIGNAL(finished()), SLOT(onTaskFinished()));
}

//---------------------------------------------------------------------------
MainScenario::~MainScenario() {}

//---------------------------------------------------------------------------
bool MainScenario::initialize(const QList<GUI::SScriptObject> & /*aScriptObjects*/) {
    return true;
}

//---------------------------------------------------------------------------
void MainScenario::start(const QVariantMap &aContext) {
    setStateTimeout(0);

    m_Context = aContext;
    QStringList args = m_Context["args"].toString().split(";");
    m_Kiosk2InstallPath = args.first().split("#").last();
    m_MonitoringComandId = args.last().split("#").last().toInt();

    QByteArray sign;
    QString error;

    // check keys
    if (!m_CryptService->getCryptEngine()->sign(0, "Hello Humo", sign, error)) {
        toLog(LogLevel::Error, QString("CHECK keys error, %1").arg(error));
        signalTriggered("abort", QVariantMap());
        return;
    }

    toLog(LogLevel::Normal, QString("CHECK keys OK"));

    // setup connection
    PPSDK::SConnection connection;

    using boost::property_tree::ptree;
    ptree pt;

    QString terminalConfig = m_Kiosk2InstallPath + "/config/terminal.xml";

    try {
        read_xml(terminalConfig.toStdString(), pt);

        BOOST_FOREACH (ptree::value_type const &v, pt.get_child("terminal")) {
            if (v.first == "connection") {
                connection.type = QString::from_StdString(
                                      v.second.get<std::string>("<xmlattr>.type", "")) == "local"
                                      ? EConnectionTypes::Unmanaged
                                      : EConnectionTypes::Dialup;
                connection.name =
                    QString::from_StdString(v.second.get<std::string>("<xmlattr>.name", ""));

                QNetworkProxy proxy;

                auto proxyType =
                    QString::from_StdString(v.second.get<std::string>("proxy.<xmlattr>.type", ""));

                if (proxyType == "http") {
                    proxy.setType(QNetworkProxy::HttpProxy);
                } else if (proxyType == "http_caching") {
                    proxy.setType(QNetworkProxy::HttpCachingProxy);
                } else if (proxyType == "socks5") {
                    proxy.setType(QNetworkProxy::Socks5Proxy);
                } else {
                    proxy.setType(QNetworkProxy::NoProxy);
                }

                if (proxy.type() != QNetworkProxy::NoProxy) {
                    proxy.setUser(QString::from_StdString(
                        v.second.get<std::string>("proxy.<xmlattr>.username", "")));
                    proxy.setPassword(QString::from_StdString(
                        v.second.get<std::string>("proxy.<xmlattr>.password", "")));
                    proxy.setHostName(QString::from_StdString(
                        v.second.get<std::string>("proxy.<xmlattr>.host", "")));
                    proxy.setPort(QString::from_StdString(
                                      v.second.get<std::string>("proxy.<xmlattr>.port", "0"))
                                      .toUShort());
                }

                connection.proxy = proxy;
            }
        }
    } catch (boost::property_tree::xml_parser_error &e) {
        toLog(LogLevel::Error,
              QString("PARSING '%1' error, %2")
                  .arg(terminalConfig)
                  .arg(QString::from_StdString(e.message())));
        signalTriggered("abort", QVariantMap());
        return;
    }

    m_NetworkService->setConnection(connection);
    m_TerminalSettings->setConnection(connection);

    // test connection
    if (!m_NetworkService->testConnection()) {
        toLog(LogLevel::Error,
              QString("CHECK connection error, %1")
                  .arg(m_NetworkService->getLastConnectionError().split(":").last()));
        signalTriggered("abort", QVariantMap());
        return;
    }

    toLog(LogLevel::Normal, QString("CHECK connection OK"));

    // start find devices
    connect(m_DeviceService,
            SIGNAL(deviceDetected(const QString &)),
            this,
            SLOT(onDeviceDetected(const QString &)));
    connect(m_DeviceService, SIGNAL(detectionStopped()), this, SLOT(onDetectionStopped()));

    toLog(LogLevel::Normal, QString("START autodetect."));

    m_FoundedDevices.clear();
    m_DeviceService->detect();
}

//---------------------------------------------------------------------------
void MainScenario::stop() {
    m_TimeoutTimer.stop();

    disconnect(m_DeviceService,
               SIGNAL(deviceDetected(const QString &)),
               this,
               SLOT(onDeviceDetected(const QString &)));
    disconnect(m_DeviceService, SIGNAL(detectionStopped()), this, SLOT(onDetectionStopped()));
}

//---------------------------------------------------------------------------
void MainScenario::pause() {
    m_TimeoutTimer.stop();
}

//---------------------------------------------------------------------------
void MainScenario::resume(const QVariantMap &aContext) {}

//---------------------------------------------------------------------------
void MainScenario::signalTriggered(const QString &aSignal, const QVariantMap & /*aArguments*/) {
    QVariantMap parameters;
    int returnCode = -1;

    if (aSignal == "abort") {
        parameters.insert("result", "abort");
        returnCode = ExitCode::Error;

        m_TimeoutTimer.stop();
        emit finished(parameters);
    } else if (aSignal == "finish") {
        returnCode = ExitCode::NoError;

        m_TimeoutTimer.stop();
        emit finished(parameters);
    }

    // abort/finish - завершаем сценарий, закрываем приложение
    if (returnCode == ExitCode::NoError || returnCode == ExitCode::Error) {
        QVariantMap parameters;
        parameters.insert("returnCode", returnCode);
        m_ScriptingCore->postEvent(PPSDK::EEventType::StopSoftware, parameters);
    }
}

//---------------------------------------------------------------------------
QString MainScenario::getState() const {
    return QString("main");
}

//---------------------------------------------------------------------------
void MainScenario::onTimeout() {
    if (m_TaskWatcher.isRunning()) {
        m_TaskWatcher.waitForFinished();
    }

    signalTriggered("finish", QVariantMap());
}

//---------------------------------------------------------------------------
void MainScenario::onTaskFinished() {
    signalTriggered("finish", QVariantMap());
}

//--------------------------------------------------------------------------
bool MainScenario::canStop() {
    return true;
}

//---------------------------------------------------------------------------
void MainScenario::onDeviceDetected(const QString &aConfigName) {
    toLog(LogLevel::Normal, QString("DETECT device %1").arg(aConfigName));
    m_FoundedDevices.append(aConfigName);
}

//---------------------------------------------------------------------------
void MainScenario::onDetectionStopped() {
    toLog(LogLevel::Normal, QString("STOP autodetect. WAIT device init."));

    // Подождем, чтобы все устройства успели проинициализироваться
    QTimer::singleShot(40000, this, SLOT(finishDeviceDetection()));
}

//---------------------------------------------------------------------------
void MainScenario::finishDeviceDetection() {
    toLog(LogLevel::Normal, QString("INIT devices done."));

    // update configs
    m_DeviceService->saveConfigurations(m_FoundedDevices);
    m_SettingsService->saveConfiguration();

    // todo check validator/printer config settings
    QStringList configurations = m_DeviceService->getConfigurations();

    auto isDeviceOK = [=](const QString &aDeviceType) -> bool {
        namespace DSDK = SDK::Driver;

        foreach (QString config, configurations) {
            if (config.section('.', 2, 2) == aDeviceType) {
                auto status = m_DeviceService->getDeviceStatus(config);

                return status && status->isMatched(DSDK::EWarningLevel::Warning);
            }
        }

        return false;
    };

    bool validatorOK = isDeviceOK(SDK::Driver::CComponents::BillAcceptor);

    if (!validatorOK) {
        toLog(LogLevel::Error, QString("BILL VALIDATOR error or not found."));
        signalTriggered("abort");
        return;
    }

    toLog(LogLevel::Normal, QString("BILL VALIDATOR is OK."));

    bool printerOK = isDeviceOK(SDK::Driver::CComponents::Printer) ||
                     isDeviceOK(SDK::Driver::CComponents::DocumentPrinter) ||
                     isDeviceOK(SDK::Driver::CComponents::FiscalRegistrator);

    bool blockTerminalByPrinter = m_TerminalSettings->getCommonSettings().blockOn(
        SDK::PaymentProcessor::SCommonSettings::PrinterError);

    if (!printerOK && blockTerminalByPrinter) {
        toLog(LogLevel::Error,
              QString("PRINTER %1, BLOCK terminal by printer = %2.")
                  .arg(printerOK ? "OK" : "error")
                  .arg(blockTerminalByPrinter ? "YES" : "NO"));
        signalTriggered("abort");
        return;
    }

    toLog(LogLevel::Error, QString("PRINTER is OK."));

    // fix standalone flag
    {
        QSettings ini("client.ini", QSettings::IniFormat);
        // Qt 6 uses UTF-8 by default, no need to set codec
        ini.setValue("common/standalone", false);

        if (ini.status() != QSettings::NoError) {
            toLog(LogLevel::Error,
                  QString("UPDATE standalone flag error: %1.")
                      .arg(ini.status() == QSettings::AccessError ? "An access error occurred"
                                                                  : "A format error occurred"));

            signalTriggered("abort");
            return;
        }

        toLog(LogLevel::Normal, QString("UPDATE standalone flag OK."));
    }

    auto queryStr =
        QString(
            "INSERT INTO `command` (`id`, `type`, `parameters`, `receive_date`, `status`, `last_update`, `on_monitoring`, `description`, `tag`) \
							VALUES(%1, 18, \""
            "\", \"%2\", 3, %1, 0, \"OK\", 10)")
            .arg(m_MonitoringComandId)
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));

    if (!m_Core->getDatabaseService()->execQuery(queryStr)) {
        toLog(LogLevel::Error, QString("UPDATE monitoring command error."));
        signalTriggered("abort");
        return;
    }

    toLog(LogLevel::Normal, QString("UPDATE monitoring command OK."));

    signalTriggered("finish", QVariantMap());
}

} // namespace Migrator3000

//---------------------------------------------------------------------------
