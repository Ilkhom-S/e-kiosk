/* @file Обработчик команд работы с устройствами выдачи наличных. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QSet>

#include <Common/ILogable.h>

#include <SDK/Drivers/IDispenser.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>

// PP
#include "DatabaseUtils/IPaymentDatabaseUtils.h"

class IApplication;
class IHardwareDatabaseUtils;

namespace SDK {
namespace PaymentProcessor {
class IDeviceService;
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
class CashDispenserManager : public SDK::PaymentProcessor::ICashDispenserManager, public ILogable {
    Q_OBJECT

    /// Данные выдаваемых объектов.
    struct SItem_Data {
        SDK::Driver::IDispenser *dispenser;
        int unit;
        int count;

        SItem_Data() : dispenser(nullptr), unit(0), count(0) {}
        SItem_Data(SDK::Driver::IDispenser *aDispenser, int aUnit, int aCount)
            : dispenser(aDispenser), unit(aUnit), count(aCount) {}
    };

    typedef QList<SItem_Data> TItem_Data;
    typedef QMap<int, TItem_Data> TItem_DataSet;
    typedef TItem_DataSet::iterator TItem_DataSetIt;

    struct SAmounts {
        SDK::PaymentProcessor::TPaymentAmount toDispensing; /// к выдаче
        SDK::PaymentProcessor::TPaymentAmount dispensed;    /// выданное

        SAmounts() : toDispensing(0), dispensed(0) {}
        SAmounts(SDK::PaymentProcessor::TPaymentAmount aToDispensing,
                 SDK::PaymentProcessor::TPaymentAmount aDispensed)
            : toDispensing(aToDispensing), dispensed(aDispensed) {}

        SAmounts &operator+=(const SAmounts &aAmounts) {
            toDispensing += aAmounts.toDispensing;
            dispensed += aAmounts.dispensed;

            return *this;
        }
    };

public:
    CashDispenserManager(IApplication *aApplication);

    /// Инициализация
    virtual bool initialize(IPaymentDatabaseUtils *aDatabase);

    /// Проверка, возможна ли выдача наличных средств
    virtual SDK::PaymentProcessor::TPaymentAmount
    canDispense(SDK::PaymentProcessor::TPaymentAmount aRequiredAmount);

    /// Выдать указанную сумму (асинхронная операция)
    virtual void dispense(SDK::PaymentProcessor::TPaymentAmount aAmount);

    /// Получить список номиналов и их кол-ва для всех устройств выдачи денег
    virtual SDK::PaymentProcessor::TCashUnitsState getCashUnitsState();

    /// Сохранить список номиналов и их кол-ва для всех устройств выдачи денег
    virtual bool setCashUnitsState(const QString &aDeviceConfigurationName,
                                   const SDK::PaymentProcessor::TCashUnitList &aCashUnitList);

public slots:
    void shutdown();

private slots:
    void updateHardwareConfiguration();

    void onUnitsDefined();
    void onDispensed(int aUnit, int aItems);
    void onRejected(int aUnit, int aItems);
    void onUnitEmpty(int aUnit);

    /// Изменение статуса диспенсера
    void onStatusChanged(SDK::Driver::EWarningLevel::Enum aLevel,
                         const QString &aTranslation,
                         int aStatus);

private:
    /// Загрузить содержимое диспенсеров из БД
    void loadCashList();

    /// Сохранить содержимое диспенсеров в БД
    void saveCashCount();

    /// Получить данные объектов для выдачи
    TItem_DataSet getItem_DataSet(SDK::PaymentProcessor::TPaymentAmount aAmount);

    /// Получить данные объекта для выдачи
    static bool getItem_Data(SDK::PaymentProcessor::TPaymentAmount aAmount,
                             TItem_DataSet &aItemData,
                             TItem_DataSetIt &aItemDataSetIt);

    /// Проверить сигнал о результате выдачи денег
    SDK::PaymentProcessor::SCashUnit *
    checkSignal(QObject *aSender, const QString &aSignalName, int aUnit);

    /// Сохранить в базе информацию о выданных купюрах
    bool storeNotes(QObject *aSender, int aUnit, int aItems);

    /// Проверить и обработать сигнал о результате выдачи денег
    bool handleSignal(QObject *aSender,
                      const QString &aSignalName,
                      int aUnit,
                      int aItems,
                      SDK::PaymentProcessor::TPaymentAmount &aAmount);

    /// Установить конфигурацию кассет
    void setCashList(SDK::Driver::IDispenser *aDispenser);

    /// Валюта
    QString m_CurrencyName;

    /// Приложение
    IApplication *m_Application;

    /// Девайс-сервис
    SDK::PaymentProcessor::IDeviceService *m_DeviceService;

    /// Экземпляр девайс-сервиса
    IHardwareDatabaseUtils *m_Database;
    IPaymentDatabaseUtils *m_PaymentDatabase;

    /// Сумма для выдачи/выданная
    SAmounts m_Amounts;

    /// Список всех устройств с их GUID
    QMap<SDK::Driver::IDispenser *, QString> m_Dispensers;

    /// Список сломанных устройств
    typedef QSet<SDK::Driver::IDispenser *> TDispensers;
    TDispensers m_FailedDispensers;

    /// Актуальный список номиналов в каждом диспенсере
    SDK::PaymentProcessor::TCashUnitsState m_CurrencyCashList;
};

//---------------------------------------------------------------------------
