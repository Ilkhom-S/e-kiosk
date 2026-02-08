/* @file Плагин сценария для оплаты картами */
#pragma once

#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

// Plugin SDK
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IFactory.h>
#include <SDK/Plugins/PluginInitializer.h>

// PP SDK
#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Scripting/IBackendScenarioObject.h>

#include "Uniteller.h"

class IApplication;

//--------------------------------------------------------------------------
namespace SDK {
namespace PaymentProcessor {
class ICore;
} // namespace PaymentProcessor

namespace Plugin {
class IEnvironment;
} // namespace Plugin
} // namespace SDK

namespace Uniteller {
class API;
} // namespace Uniteller

namespace PPSDK = SDK::PaymentProcessor;

namespace CUnitellerBackend {
const QString PluginName = "CardBackend";
const QString ScriptObjectName = "CardPOS";

//--------------------------------------------------------------------------
class UnitellerCore : public SDK::PaymentProcessor::Scripting::IBackendScenarioObject,
                      public ILogable {
    Q_OBJECT

public:
    UnitellerCore(SDK::PaymentProcessor::ICore *aCore,
                  ILog *aLog,
                  QSharedPointer<Uniteller::API> aAPI);

public:
    virtual QString getName() const { return CUnitellerBackend::ScriptObjectName; }

signals:
    void onTimeout();

    /// Ждет поднесения карты (прикладывай или вставляй)
    void onReadyToCard();

    /// Показать сообщение пользователю
    void onShowMessage(const QString &aMessage);

    /// Сообщить пользователю об ошибке
    void onError(const QString &aMessage);

    void onPINRequired();

    /// Сообщение о вводе пина, может передаваться от 0 до 4х звездочек
    void onEnterPin(const QString &aPinString);

    /// Сообщение о переходе к обмену с процессингом после ввода пина
    void onOnlineRequired();

    void cardInserted();
    void cardOut();

    // Дерни за веревочкУ, карта выскочит
    void ejected();

public slots:
    void ejectCard();

private slots:
    void onDeviceEvent(Uniteller::DeviceEvent::Enum aEvent, Uniteller::KeyCode::Enum aKeyCode);
    void onPrintReceipt(const QStringList &aLines);

private:
    SDK::PaymentProcessor::ICore *m_Core;

    QTimer m_DummyTimer;
    int m_CountPINNumbers;
};

//--------------------------------------------------------------------------
class UnitellerBackendPlugin
    : public SDK::Plugin::IFactory<SDK::PaymentProcessor::Scripting::IBackendScenarioObject> {
public:
    UnitellerBackendPlugin(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath);

public:
    /// Возвращает название плагина.
    virtual QString getPluginName() const { return CUnitellerBackend::PluginName; }

    /// Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const { return QVariantMap(); }

    /// Настраивает плагин.
    virtual void setConfiguration(const QVariantMap &aParameters) { Q_UNUSED(aParameters); }

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    virtual QString getConfigurationName() const { return m_InstancePath; }

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной
    /// программы).
    virtual bool saveConfiguration() {
        return m_Environment->saveConfiguration(CUnitellerBackend::PluginName, getConfiguration());
    }

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const { return true; }

    /// Возвращает список имен классов, которые создает фабрика.
    virtual QStringList getClassNames() const { return QStringList(CUnitellerBackend::PluginName); }

    /// Создает класс c заданным именем.
    virtual PPSDK::Scripting::IBackendScenarioObject *create(const QString &aClassName) const;

private:
    QString m_InstancePath;
    SDK::Plugin::IEnvironment *m_Environment;

private:
    QSharedPointer<PPSDK::Scripting::IBackendScenarioObject> m_Object;
};

} // namespace CUnitellerBackend