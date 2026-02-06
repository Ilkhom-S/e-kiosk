/* @file Фабрика платежей. */

#pragma once

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QPair>
#include <QtCore/QPointer>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include <SDK/PaymentProcessor/Humo/RequestSender.h>
#include <SDK/PaymentProcessor/Settings/Provider.h>

#include "PaymentFactoryBase.h"

namespace PPSDK = SDK::PaymentProcessor;

using namespace PPSDK::Humo;

//------------------------------------------------------------------------------
class PaymentFactory : public PaymentFactoryBase {
    Q_OBJECT

public:
    //---------------------------------------------------------------------------
    // Конструктор фабрики платежей
    PaymentFactory(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath);

#pragma region SDK::Plugin::IPlugin interface

    //---------------------------------------------------------------------------
    // Возвращает название плагина
    virtual QString getPluginName() const;

#pragma endregion

#pragma region SDK::PaymentProcessor::IPaymentFactory interface

    //---------------------------------------------------------------------------
    // Инициализирует фабрику
    virtual bool initialize();

    //---------------------------------------------------------------------------
    // Завершает работу фабрики
    virtual void shutdown();

    //---------------------------------------------------------------------------
    // Возвращает поддерживаемые типы платежей
    virtual QStringList getSupportedPaymentTypes() const;

    //---------------------------------------------------------------------------
    // Создание экземпляра платежа, который экспортирует фабрика
    virtual SDK::PaymentProcessor::IPayment *createPayment(const QString &aType);

    //---------------------------------------------------------------------------
    // Удаление экземпляра платежа
    virtual void releasePayment(PPSDK::IPayment *aPayment);

    //---------------------------------------------------------------------------
    // Возвращает уточнённое описание для провайдера, который должен проводиться с помощью этой
    // фабрики платежей Если фабрика не может уточнить описание (или оператор сейчас использовать
    // нельзя), то она возвращает невалидную структуру. Пример: пиновая фабрика постепенно загружает
    // списки номиналов карт и обновляет их в настройках провайдера при вызове этого метода
    virtual PPSDK::SProvider getProviderSpecification(const PPSDK::SProvider &aProvider);

#pragma endregion

private:
    QMutex mMutex;
};

//------------------------------------------------------------------------------
