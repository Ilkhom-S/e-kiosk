/* @file Обработчик команд работы с устройствами/сервисами получения денег. */

#pragma once

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

#include <Common/ILogable.h>

#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/ICashAcceptorManager.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/Plugins/IPlugin.h>

// PP
#include "DatabaseUtils/IPaymentDatabaseUtils.h"

class IApplication;
class IHardwareDatabaseUtils;

namespace SDK {
namespace PaymentProcessor {
class IDeviceService;
class IChargeProvider;
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
class CashAcceptorManager : public SDK::PaymentProcessor::ICashAcceptorManager, public ILogable {
    Q_OBJECT

public:
    CashAcceptorManager(IApplication *aApplication);
    virtual ~CashAcceptorManager();

    /// Инициализация.
    virtual bool initialize(IPaymentDatabaseUtils *aDatabase);

    /// Остановка.
    virtual bool shutdown();

    /// Возвращает список доступных методов оплаты.
    virtual QStringList getPaymentMethods();

    /// Начать приём денег для указанного платежа.
    virtual bool enable(qint64 aPayment,
                        const QString &aPaymentMethod,
                        SDK::PaymentProcessor::TPaymentAmount aMaxAmount);

    /// Завершить приём денег для указанного платежа.
    virtual bool disable(qint64 aPayment);

    /// Значение счетчика непринятых купюр.
    int getRejectCount() const;

private:
    /// Увеличить значение счетчика непринятых купюр.
    void incrementRejectCount();

    /// Загрузка таблицы разрешённых номиналов
    void initWorkingParList();

    /// Разрешить ли принимать купюру/монету данного номинала в счет активного платежа
    bool allowMoreMoney(SDK::PaymentProcessor::TPaymentAmount aAmount);

    /// Определяет, имеет ли платеж фиксированную сумму
    bool isFixedAmountPayment(qint64 aPayment);

private slots:
    /// Валидатор распознал купюру.
    void onEscrow(const SDK::Driver::SPar &aPar);

    /// Валидатор распознал купюру (с контролем переполнения суммы платежа)
    void onEscrowChangeControl(const SDK::Driver::SPar &aPar);

    /// Изменение статуса валидатора.
    void onStatusChanged(SDK::Driver::EWarningLevel::Enum aLevel,
                         const QString &aTranslation,
                         int aStatus);

    /// Валидатор успешно уложил купюру.
    void onStacked(SDK::Driver::TParList aNotes);

    /// Обновление списка устройств.
    void updateHardwareConfiguration();

    /// Провайдер денег получил указанную сумму.
    void onChargeProviderStacked(const SDK::PaymentProcessor::SNote &aNote);

private:
    IApplication *mApplication;
    IPaymentDatabaseUtils *mDatabase;
    SDK::PaymentProcessor::IDeviceService *mDeviceService;
    bool mDisableAmountOverflow;

    struct SPaymentData {
        qint64 paymentId;
        SDK::PaymentProcessor::TPaymentAmount currentAmount;
        SDK::PaymentProcessor::TPaymentAmount maxAmount;
        QSet<SDK::Driver::ICashAcceptor *> validators;
        QSet<SDK::PaymentProcessor::IChargeProvider *> chargeProviders;

        explicit SPaymentData(qint64 aPaymentId)
            : paymentId(aPaymentId), currentAmount(0.0), maxAmount(0.0) {}

        bool maxAmountReached() const;
        bool chargeSourceEmpty() const;
    };

    /// Список всех валидаторов.
    typedef QList<SDK::Driver::ICashAcceptor *> TCashAcceptorList;
    TCashAcceptorList mDeviceList;

    QSharedPointer<SPaymentData> mPaymentData;

    /// Набор разрешенных номиналов.
    SDK::Driver::TParList mWorkingParList;

    /// Список всех провайдеров денежных средств
    QList<SDK::PaymentProcessor::IChargeProvider *> mChargeProviders;
};

//---------------------------------------------------------------------------
