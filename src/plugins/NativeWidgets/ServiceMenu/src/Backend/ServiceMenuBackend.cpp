/* @file Бэкэнд для сервисного меню; обеспечивает доступ к основным сервисам ядра */

#include "ServiceMenuBackend.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QCryptographicHash>

#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/IRemoteService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "GUI/MessageBox/MessageBox.h"
#include "GUI/ServiceTags.h"
#include "HardwareManager.h"
#include "KeysManager.h"
#include "NetworkManager.h"
#include "PaymentManager.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
ServiceMenuBackend::ServiceMenuBackend(SDK::Plugin::IEnvironment *aFactory, ILog *aLog)
    : m_Factory(aFactory), m_Log(aLog), m_AutoEncashmentEnabled(false),
      m_AuthorizationEnabled(true) {
    m_Core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        m_Factory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    GUI::MessageBox::initialize();

    m_TerminalSettings = static_cast<SDK::PaymentProcessor::TerminalSettings *>(
        m_Core->getSettingsService()->getAdapter(
            SDK::PaymentProcessor::CAdapterNames::TerminalAdapter));

    connect(&m_HeartbeatTimer, SIGNAL(timeout()), this, SLOT(sendHeartbeat()));
}

//------------------------------------------------------------------------
ServiceMenuBackend::~ServiceMenuBackend() {
    GUI::MessageBox::shutdown();

    foreach (SDK::Plugin::IPlugin *plugin, m_WidgetPluginList) {
        m_Factory->getPluginLoader()->destroyPlugin(plugin);
    }

    m_WidgetPluginList.clear();
}

//------------------------------------------------------------------------
HardwareManager *ServiceMenuBackend::getHardwareManager() {
    if (m_HardwareManager.isNull()) {
        m_HardwareManager = QSharedPointer<HardwareManager>(new HardwareManager(m_Factory, m_Core));
        m_ConfigList << m_HardwareManager.data();
    }

    return m_HardwareManager.data();
}

//------------------------------------------------------------------------
KeysManager *ServiceMenuBackend::getKeysManager() {
    if (m_KeysManager.isNull()) {
        m_KeysManager = QSharedPointer<KeysManager>(new KeysManager(m_Core));
        m_ConfigList << m_KeysManager.data();
    }

    return m_KeysManager.data();
}

//------------------------------------------------------------------------
NetworkManager *ServiceMenuBackend::getNetworkManager() {
    if (m_NetworkManager.isNull()) {
        m_NetworkManager = QSharedPointer<NetworkManager>(new NetworkManager(m_Core));
        m_ConfigList << m_NetworkManager.data();
    }

    return m_NetworkManager.data();
}

//------------------------------------------------------------------------
PaymentManager *ServiceMenuBackend::getPaymentManager() {
    if (m_PaymentManager.isNull()) {
        m_PaymentManager = QSharedPointer<PaymentManager>(new PaymentManager(m_Core));
    }

    return m_PaymentManager.data();
}

//------------------------------------------------------------------------
void ServiceMenuBackend::toLog(const QString &aMessage) {
    toLog(LogLevel::Normal, aMessage);
}

//------------------------------------------------------------------------
void ServiceMenuBackend::toLog(LogLevel::Enum aLevel, const QString &aMessage) {
    m_Log->write(aLevel, aMessage);
}

//------------------------------------------------------------------------
SDK::PaymentProcessor::ICore *ServiceMenuBackend::getCore() const {
    return m_Core;
}

//------------------------------------------------------------------------
void ServiceMenuBackend::getTerminalInfo(QVariantMap &aTerminalInfo) {
    aTerminalInfo.clear();
    aTerminalInfo[CServiceTags::TerminalNumber] = m_TerminalSettings->getKeys()[0].ap;
    aTerminalInfo[CServiceTags::SoftwareVersion] = m_TerminalSettings->getAppEnvironment().version;

    // TODO Исправить на константу
    aTerminalInfo[CServiceTags::TerminalLocked] =
        dynamic_cast<SDK::PaymentProcessor::ITerminalService *>(
            m_Core->getService("TerminalService"))
            ->isLocked();
}

//------------------------------------------------------------------------
void ServiceMenuBackend::sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType) {
    SDK::PaymentProcessor::Event e(aEventType, "");
    m_Core->getEventService()->sendEvent(e);
}

//------------------------------------------------------------------------
void ServiceMenuBackend::sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType,
                                   const QVariantMap &aParameters) {
    SDK::PaymentProcessor::Event e(aEventType, "", QVariant::fromValue(aParameters));
    m_Core->getEventService()->sendEvent(e);
}

//------------------------------------------------------------------------
bool ServiceMenuBackend::isConfigurationChanged() {
    bool result = false;

    foreach (IConfigManager *manager, m_ConfigList) {
        if (manager->isConfigurationChanged()) {
            manager->resetConfiguration();
            result = true;
        }
    }

    return result;
}

//------------------------------------------------------------------------
bool ServiceMenuBackend::saveConfiguration() {
    if (isConfigurationChanged()) {
        try {
            m_Core->getSettingsService()->saveConfiguration();
        } catch (const std::exception &e) {
            toLog(QString("Save configuration error (%1)").arg(e.what()));
        }
    }

    return true;
}

//------------------------------------------------------------------------
void ServiceMenuBackend::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//------------------------------------------------------------------------
QVariantMap ServiceMenuBackend::getConfiguration() const {
    return m_Parameters;
}

//------------------------------------------------------------------------
void ServiceMenuBackend::saveDispenserUnitState() {
    m_CashUnitsState = m_Core->getFundsService()->getDispenser()->getCashUnitsState();
}

//------------------------------------------------------------------------
void ServiceMenuBackend::printDispenserDiffState() {
    PPSDK::TCashUnitsState curCashUnitsState =
        m_Core->getFundsService()->getDispenser()->getCashUnitsState();
    if (m_CashUnitsState != curCashUnitsState) {
        QVariantMap parameters;

        QStringList units;
        units << "FIRST" << "SECOND";

        for (int i = 0; i < units.count(); i++) {
            PPSDK::SCashUnit beforeUnit, afterUnit;

            const auto &beforeList = m_CashUnitsState.values();
            if (beforeList.size()) {
                if (beforeList.first().size() > i) {
                    beforeUnit = beforeList.first()[i];
                }
            }

            const auto &afterList = curCashUnitsState.values();
            if (afterList.size()) {
                if (afterList.first().size() > i) {
                    afterUnit = afterList.first()[i];
                }
            }

            parameters.insert(QString("CU_%1").arg(units[i]), i + 1);
            parameters.insert(QString("CU_%1_NOMINAL_BEFORE").arg(units[i]), beforeUnit.nominal);
            parameters.insert(QString("CU_%1_COUNT_BEFORE").arg(units[i]), beforeUnit.count);
            parameters.insert(QString("CU_%1_AMOUNT_BEFORE").arg(units[i]), beforeUnit.amount());
            parameters.insert(QString("CU_%1_NOMINAL_AFTER").arg(units[i]), afterUnit.nominal);
            parameters.insert(QString("CU_%1_COUNT_AFTER").arg(units[i]), afterUnit.count);
            parameters.insert(QString("CU_%1_AMOUNT_AFTER").arg(units[i]), afterUnit.amount());
            parameters.insert(QString("CU_%1_DIFF").arg(units[i]),
                              afterUnit.amount() - beforeUnit.amount());

            toLog("-------------------------------------------");
            toLog(QString("Cash unit %1").arg(i + 1));
            toLog(QString("Before: %1 * %2 = %3")
                      .arg(beforeUnit.nominal)
                      .arg(beforeUnit.count)
                      .arg(beforeUnit.amount()));
            toLog(QString("After: %1 * %2 = %3")
                      .arg(afterUnit.nominal)
                      .arg(afterUnit.count)
                      .arg(afterUnit.amount()));

            toLog(QString("Diff: %1").arg(afterUnit.amount() - beforeUnit.amount()));
        }

        QtConcurrent::run([=]() {
            m_Core->getPrinterService()->printReceipt(QString(""),
                                                      parameters,
                                                      QString("dispenser_diff"),
                                                      DSDK::EPrintingModes::None,
                                                      true);
        });
    } else {
        toLog("Dispenser cash units state are not changed.");
    }
}

//------------------------------------------------------------------------
void ServiceMenuBackend::needUpdateConfigs() {
    // TODO: убрать константу.
    dynamic_cast<SDK::PaymentProcessor::ITerminalService *>(m_Core->getService("TerminalService"))
        ->needUpdateConfigs();
}

//------------------------------------------------------------------------
bool ServiceMenuBackend::hasAnyPassword() const {
    SDK::PaymentProcessor::SServiceMenuPasswords serviceMenuSettings =
        m_TerminalSettings->getServiceMenuPasswords();
    bool admin =
        serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Service]
            .isEmpty();
    bool tech =
        serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Technician]
            .isEmpty();
    bool encash =
        serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Collection]
            .isEmpty();

    return !(admin && tech && encash);
}

//---------------------------------------------------------------------------
bool ServiceMenuBackend::authorize(const QString &aPassword) {
    bool result = true;
    m_AccessRights.clear();

    QString hash = QString::fromUtf8(
        QCryptographicHash::hash(aPassword.toLatin1(), QCryptographicHash::Md5).toHex());
    SDK::PaymentProcessor::SServiceMenuPasswords serviceMenuSettings =
        m_TerminalSettings->getServiceMenuPasswords();

    // Роль администратора
    if (hash == serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Service]
                    .toLower()) {
        m_AccessRights << ServiceMenuBackend::Diagnostic << ServiceMenuBackend::SetupHardware
                       << ServiceMenuBackend::SetupNetwork << ServiceMenuBackend::SetupKeys
                       << ServiceMenuBackend::ViewPaymentSummary << ServiceMenuBackend::ViewPayments
                       << ServiceMenuBackend::PrintReceipts << ServiceMenuBackend::Encash
                       << ServiceMenuBackend::StopApplication << ServiceMenuBackend::RebootTerminal
                       << ServiceMenuBackend::LockTerminal;

        m_UserRole = CServiceTags::UserRole::RoleAdministrator;
    }
    // Роль техника
    else if (hash ==
             serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Technician]
                 .toLower()) {
        m_AccessRights << ServiceMenuBackend::Diagnostic << ServiceMenuBackend::SetupHardware
                       << ServiceMenuBackend::SetupNetwork << ServiceMenuBackend::SetupKeys
                       << ServiceMenuBackend::StopApplication << ServiceMenuBackend::RebootTerminal
                       << ServiceMenuBackend::LockTerminal;

        m_UserRole = CServiceTags::UserRole::RoleTechnician;
    }
    // Роль инкассатора
    else if (hash ==
             serviceMenuSettings.passwords[SDK::PaymentProcessor::CServiceMenuPasswords::Collection]
                 .toLower()) {
        m_AccessRights << ServiceMenuBackend::Encash;
        m_UserRole = CServiceTags::UserRole::RoleCollector;
    } else {
        result = false;
    }

    return result;
}

//---------------------------------------------------------------------------
ServiceMenuBackend::TAccessRights ServiceMenuBackend::getAccessRights() const {
    return m_AccessRights;
}

//---------------------------------------------------------------------------
bool ServiceMenuBackend::isAuthorizationEnabled() const {
    return m_AuthorizationEnabled;
}

//------------------------------------------------------------------------
QList<QWidget *> ServiceMenuBackend::getExternalWidgets(bool aReset) {
    QStringList plugins = m_Factory->getPluginLoader()->getPluginList(
        QRegularExpression("PaymentProcessor\\.GraphicsItem\\..*\\ServiceMenuWidget"));

    if (m_WidgetPluginList.isEmpty()) {
        foreach (const QString &widget, plugins) {
            SDK::Plugin::IPlugin *plugin = m_Factory->getPluginLoader()->createPlugin(widget);
            m_WidgetPluginList << plugin;
        }
    }

    QList<QWidget *> widgetList;

    foreach (SDK::Plugin::IPlugin *plugin, m_WidgetPluginList) {
        SDK::GUI::IGraphicsItem *item_Object = dynamic_cast<SDK::GUI::IGraphicsItem *>(plugin);
        if (item_Object) {
            if (aReset) {
                item_Object->reset(QVariantMap());
            }

            widgetList << item_Object->getNativeWidget();
        }
    }

    return widgetList;
}

//------------------------------------------------------------------------
void ServiceMenuBackend::startHeartbeat() {
    m_HeartbeatTimer.start(CServiceMenuBackend::HeartbeatTimeout);
}

//------------------------------------------------------------------------
void ServiceMenuBackend::stopHeartbeat() {
    m_HeartbeatTimer.stop();
}

//------------------------------------------------------------------------
void ServiceMenuBackend::sendHeartbeat() {
    m_Core->getRemoteService()->sendHeartbeat();
}

//------------------------------------------------------------------------