/* @file Реализация плагина списания с карты UCS. */

#pragma once

#include <QtCore/QObject>

#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IChargeProvider.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/Plugins/IPlugin.h>

#include "API.h"

namespace SDK {
namespace PaymentProcessor {
class DealerSettings;
} // namespace PaymentProcessor
} // namespace SDK

namespace {
extern const char Param_RuntimePath[];
} // namespace

//------------------------------------------------------------------------------
class UcsChargeProvider : public QObject,
                          public SDK::PaymentProcessor::IChargeProvider,
                          public SDK::Plugin::IPlugin,
                          public ILogable {
    Q_OBJECT

public:
    UcsChargeProvider(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath);
    ~UcsChargeProvider();

    //////////////////////////////////////////////////////////////////////////
    /// Возвращает название плагина.
    virtual QString getPluginName() const;

    /// Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const;

    /// Настраивает плагин.
    virtual void setConfiguration(const QVariantMap &aParameters);

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    virtual QString getConfigurationName() const;

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной
    /// программы).
    virtual bool saveConfiguration();

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const;

    //////////////////////////////////////////////////////////////////////////
    virtual bool subscribe(const char *aSignal, QObject *aReceiver, const char *aSlot);
    virtual bool unsubscribe(const char *aSignal, QObject *aReceiver);

    /// Возвращает метод оплаты, поддерживаемый провайдером
    virtual QString getMethod();

    /// Включить приём средств
    virtual bool enable(SDK::PaymentProcessor::TPaymentAmount aMaxAmount);

    /// Выключение провайдера
    virtual bool disable();

signals:
    void stacked(SDK::PaymentProcessor::SNote);

public slots:
    void onEvent(const SDK::PaymentProcessor::Event &aEvent);

private slots:
    void onSaleComplete(double aAmount,
                        int aCurrency,
                        const QString &aRRN,
                        const QString &aConfirmationCode);
    void onEncashmentComplete();

private:
    QString m_InstancePath;
    QVariantMap m_Parameters;
    SDK::Plugin::IEnvironment *m_Factory;
    SDK::PaymentProcessor::ICore *m_Core;
    SDK::PaymentProcessor::DealerSettings *m_DealerSettings;
    QSharedPointer<Ucs::API> m_Api;
    QSharedPointer<Ucs::API> m_DatabaseUtils;
};

//------------------------------------------------------------------------------
