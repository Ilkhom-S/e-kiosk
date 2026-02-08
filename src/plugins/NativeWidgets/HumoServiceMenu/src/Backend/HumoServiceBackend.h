/* @file Бэкэнд для HumoService; обеспечивает доступ к основным сервисам ядра */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>
#include <QtCore/QVariantMap>

#include <Common/ILog.h>

#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ICashDispenserManager.h>

namespace SDK {
namespace PaymentProcessor {
class ICore;
class INetworkService;
class TerminalSettings;
} // namespace PaymentProcessor
namespace Plugin {
class IEnvironment;
class IPlugin;
} // namespace Plugin
} // namespace SDK

class IConfigManager;
class HardwareManager;
class KeysManager;
class NetworkManager;
class PaymentManager;

//------------------------------------------------------------------------
namespace CHumoServiceBackend {
const QString LogName = "HumoService";
const int HeartbeatTimeout = 60 * 1000;
} // namespace CHumoServiceBackend

//------------------------------------------------------------------------
class HumoServiceBackend : public QObject {
    Q_OBJECT

public:
    HumoServiceBackend(SDK::Plugin::IEnvironment *aFactory, ILog *aLog);
    ~HumoServiceBackend();

public:
    enum AccessRights {
        Diagnostic,
        ViewLogs,

        SetupHardware,
        SetupNetwork,
        SetupKeys,

        ViewPaymentSummary,
        ViewPayments,
        ProcessPayments,
        PrintReceipts,

        Encash,

        StopApplication,
        RebootTerminal,
        LockTerminal
    };

    typedef QSet<AccessRights> TAccessRights;

    enum HandlerType { Info = 0, Hardware, Encashment, Payment, System, Keys };

public:
    /// Авторизация и получение прав доступа.
    virtual bool authorize(const QString &aPassword);

    /// Возвращает текущие права системы.
    virtual TAccessRights getAccessRights() const;

    /// Бэкэнд поддерживает авторизацию?
    virtual bool isAuthorizationEnabled() const;

    // Изменились ли настройки
    static bool isConfigurationChanged();

public:
    HardwareManager *getHardwareManager();
    KeysManager *getKeysManager();
    NetworkManager *getNetworkManager();
    PaymentManager *getPaymentManager();

public:
    void toLog(const QString &aMessage);
    void toLog(LogLevel::Enum aLevel, const QString &aMessage);
    SDK::PaymentProcessor::ICore *getCore() const;

public:
    void getTerminalInfo(QVariantMap &aTerminalInfo);

    void sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType);
    void sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType,
                   const QVariantMap &aParameters);

    /// Сохранить полную конфигурацию
    static bool saveConfiguration();
    void setConfiguration(const QVariantMap &aParameters);
    QVariantMap getConfiguration() const;

    /// Сохранить состояние кассет диспенсера
    void saveDispenserUnitState();

    /// Вывести в лог и на печать разницу между сохраненным состоянием диспенсера и текущим
    void printDispenserDiffState();

    /// Вызываем, если требуется обновить config.xml
    void needUpdateConfigs();

    static bool hasAnyPassword();

    /// С какими правами зашли в сервисное меню
    QString getUserRole() const { return m_UserRole; }

    QList<QWidget *> getExternalWidgets(bool aReset = true);

    void startHeartbeat();
    void stopHeartbeat();

private slots:
    void sendHeartbeat();

private:
    SDK::PaymentProcessor::ICore *m_Core;
    SDK::Plugin::IEnvironment *m_Factory;
    QSharedPointer<HardwareManager> m_HardwareManager;
    QSharedPointer<KeysManager> m_KeysManager;
    QSharedPointer<NetworkManager> m_NetworkManager;
    QSharedPointer<PaymentManager> m_PaymentManager;

    QList<IConfigManager *> m_ConfigList;

    QList<SDK::Plugin::IPlugin *> m_WidgetPluginList;

    ILog *m_Log;
    SDK::PaymentProcessor::TerminalSettings *m_TerminalSettings;

    TAccessRights m_AccessRights;
    QString m_UserRole;

    QVariantMap m_Parameters;

    bool m_AutoEncashmentEnabled;
    bool m_AuthorizationEnabled;

    /// Состояние кассет диспенсера на момент входа в СМ
    SDK::PaymentProcessor::TCashUnitsState m_CashUnitsState;

    /// Таймер отправки харбитов
    QTimer m_HeartbeatTimer;
};

//---------------------------------------------------------------------------