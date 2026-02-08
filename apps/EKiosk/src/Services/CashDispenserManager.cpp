/* @file Обработчик команд работы с устройствами выдачи наличных. */

#include "CashDispenserManager.h"

#include <QtCore/QVector>
#include <QtCore/QtAlgorithms>

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/IDispenser.h>
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <algorithm>
#include <math.h>
#include <numeric>

#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "FundsService.h"
#include "Services/DatabaseService.h"
#include "Services/DeviceService.h"
#include "Services/PaymentService.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
CashDispenserManager::CashDispenserManager(IApplication *aApplication)
    : ILogable(CFundsService::LogName), m_Application(aApplication), m_DeviceService(nullptr),
      m_Database(nullptr), m_PaymentDatabase(nullptr) {}

//---------------------------------------------------------------------------
bool CashDispenserManager::initialize(IPaymentDatabaseUtils *aDatabase) {
    m_DeviceService = DeviceService::instance(m_Application);
    m_Database =
        DatabaseService::instance(m_Application)->getDatabaseUtils<IHardwareDatabaseUtils>();
    m_PaymentDatabase = aDatabase;

    m_Dispensers.clear();

    // Получаем настройки терминала
    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();

    m_CurrencyName = settings->getCurrencySettings().name;

    if (m_CurrencyName.isEmpty()) {
        toLog(LogLevel::Error, "Currency is not set for funds service!");

        return false;
    }

    updateHardwareConfiguration();

    connect(
        m_DeviceService, SIGNAL(configurationUpdated()), this, SLOT(updateHardwareConfiguration()));

    return true;
}

//---------------------------------------------------------------------------
void CashDispenserManager::updateHardwareConfiguration() {
    m_Dispensers.clear();

    // Получаем настройки терминала
    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();

    // Получаем список всех доступных устройств.
    QStringList deviceList = settings->getDeviceList().filter(
        QRegularExpression(QString("(%1)").arg(DSDK::CComponents::Dispenser)));

    foreach (const QString &configurationName, deviceList) {
        auto *device =
            dynamic_cast<DSDK::IDispenser *>(m_DeviceService->acquireDevice(configurationName));

        if (device) {
            // Подписываемся на сигналы.
            device->subscribe(
                SDK::Driver::IDispenser::UnitsDefinedSignal, this, SLOT(onUnitsDefined()));
            device->subscribe(
                SDK::Driver::IDispenser::DispensedSignal, this, SLOT(onDispensed(int, int)));
            device->subscribe(
                SDK::Driver::IDispenser::RejectedSignal, this, SLOT(onRejected(int, int)));
            device->subscribe(
                SDK::Driver::IDispenser::UnitEmptySignal, this, SLOT(onUnitEmpty(int)));

            device->subscribe(
                SDK::Driver::IDevice::StatusSignal,
                this,
                SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

            m_Dispensers.insert(device, configurationName);
        } else {
            toLog(LogLevel::Error,
                  QString("Failed to acquire cash dispenser %1.").arg(configurationName));
        }
    }

    // Грузим список загруженных купюр
    loadCashList();

    onUnitsDefined();
}

//---------------------------------------------------------------------------
void CashDispenserManager::shutdown() {
    // Сохраняем состояние по купюрам
    saveCashCount();

    foreach (DSDK::IDispenser *dispenser, m_Dispensers.keys()) {
        m_DeviceService->releaseDevice(dispenser);
    }

    m_Dispensers.clear();
}

//---------------------------------------------------------------------------
void CashDispenserManager::onStatusChanged(DSDK::EWarningLevel::Enum aLevel,
                                           const QString &aTranslation,
                                           int /*aStatus*/) {
    auto *dispenser = dynamic_cast<DSDK::IDispenser *>(sender());

    if (!dispenser) {
        return;
    }

    if (aLevel == DSDK::EWarningLevel::Error) {
        m_FailedDispensers.insert(dispenser);

        emit error(aTranslation);
    } else {
        m_FailedDispensers.remove(dispenser);

        emit activity();
    }
}

//---------------------------------------------------------------------------
PPSDK::SCashUnit *
CashDispenserManager::checkSignal(QObject *aSender, const QString &aSignalName, int aUnit) {
    auto *dispenser = dynamic_cast<DSDK::IDispenser *>(aSender);

    if (!dispenser) {
        toLog(LogLevel::Error,
              QString("Receive %1 signal, but sender not have DSDK::IDispenser interface.")
                  .arg(aSignalName));
        return nullptr;
    }

    QString configurationName = m_Dispensers.value(dispenser);
    int unitCount = m_CurrencyCashList[configurationName].size();

    if (unitCount <= aUnit) {
        toLog(LogLevel::Error,
              QString("Wrong unit number = %1, need max %2.").arg(aUnit).arg(unitCount - 1));
        return nullptr;
    }

    return &m_CurrencyCashList[configurationName][aUnit];
}

//---------------------------------------------------------------------------
bool CashDispenserManager::handleSignal(QObject *aSender,
                                        const QString &aSignalName,
                                        int aUnit,
                                        int aItems,
                                        PPSDK::TPaymentAmount &aAmount) {
    PPSDK::SCashUnit *cashUnit = checkSignal(aSender, aSignalName, aUnit);

    if (!cashUnit) {
        return false;
    }

    cashUnit->count -= qMin(aItems, cashUnit->count);
    int nominal = cashUnit->nominal;
    aAmount = nominal * aItems;

    toLog(LogLevel::Normal,
          QString("%1 %2 notes (nominal %3) AMOUNT = %4")
              .arg(aSignalName)
              .arg(aItems)
              .arg(nominal)
              .arg(aAmount, 0, 'f', 2));

    if (cashUnit->count == 0) {
        auto *dispenser = dynamic_cast<DSDK::IDispenser *>(aSender);
        setCashList(dispenser);
    }

    saveCashCount();

    return true;
}

//---------------------------------------------------------------------------
void CashDispenserManager::setCashList(DSDK::IDispenser *aDispenser) {
    DSDK::TUnitData unitData;

    foreach (auto cashUnit, m_CurrencyCashList[m_Dispensers[aDispenser]]) {
        unitData << cashUnit.count;
    }

    aDispenser->setUnitList(unitData);
}

//---------------------------------------------------------------------------
void CashDispenserManager::onUnitsDefined() {
    auto *dispenser = dynamic_cast<DSDK::IDispenser *>(sender());
    TDispensers dispensers;

    if (dispenser) {
        dispensers << dispenser;
    } else {
        dispensers =
            QSet<DSDK::IDispenser *>(m_Dispensers.keys().begin(), m_Dispensers.keys().end());
    }

    /// Проверяем и обновляем список доступных купюр
    /// если загруженных купюр нет - создаем и сохраняем в БД пустой список

    foreach (DSDK::IDispenser *dispenser, dispensers) {
        QString configPath = m_Dispensers.value(dispenser);
        int units = dispenser->units();

        if (units != 0) {
            int currentUnits =
                m_CurrencyCashList.contains(configPath) ? m_CurrencyCashList[configPath].size() : 0;

            if (currentUnits < units) {
                PPSDK::TCashUnitList cashUnitList = PPSDK::TCashUnitList(
                    units - currentUnits, PPSDK::SCashUnit(m_CurrencyName, 0, 0));
                m_CurrencyCashList[configPath] << cashUnitList;
            } else if (currentUnits > units) {
                PPSDK::TCashUnitList &cashUnitList = m_CurrencyCashList[configPath];
                cashUnitList = cashUnitList.mid(0, units);
            }

            if (currentUnits != units) {
                saveCashCount();
            }
        }
    }
}

//---------------------------------------------------------------------------
void CashDispenserManager::onDispensed(int aUnit, int aItems) {
    PPSDK::TPaymentAmount amount = 0;

    if (handleSignal(sender(), "Dispensed", aUnit, aItems, amount)) {
        storeNotes(sender(), aUnit, aItems);

        m_Amounts += SAmounts(-amount, amount);

        if (canDispense(m_Amounts.toDispensing) != 0.0) {
            emit activity();

            dispense(m_Amounts.toDispensing);
        } else {
            toLog(LogLevel::Normal,
                  QString("Send dispensed total amount = %1").arg(m_Amounts.dispensed, 0, 'f', 2));

            emit dispensed(m_Amounts.dispensed);

            m_Amounts = SAmounts(0, 0);
        }
    } else {
        toLog(LogLevel::Warning,
              "Send dispensed total amount = 0 due to error in handling notes info");

        emit dispensed(0);

        m_Amounts = SAmounts(0, 0);
    }
}

//---------------------------------------------------------------------------
void CashDispenserManager::onRejected(int aUnit, int aItems) {
    PPSDK::TPaymentAmount amount = NAN;
    handleSignal(sender(), "Rejected", aUnit, aItems, amount);
}

//---------------------------------------------------------------------------
void CashDispenserManager::onUnitEmpty(int aUnit) {
    PPSDK::SCashUnit *cashUnit = checkSignal(sender(), "unitEmpty", aUnit);

    if (cashUnit) {
        cashUnit->count = 0;
    }
}

//---------------------------------------------------------------------------
CashDispenserManager::TItem_DataSet
CashDispenserManager::getItem_DataSet(PPSDK::TPaymentAmount aAmount) {
    CashDispenserManager::TItem_DataSet result;

    foreach (auto dispenser, m_Dispensers.keys()) {
        if (!m_FailedDispensers.contains(dispenser)) {
            auto cashList = m_CurrencyCashList[m_Dispensers[dispenser]];

            for (int i = 0; i < cashList.size(); i++) {
                if (dispenser->isDeviceReady(i) && (cashList[i].count != 0) &&
                    (cashList[i].nominal != 0) && (cashList[i].nominal <= aAmount)) {
                    int count = (cashList[i].count > 0) ? cashList[i].count : 0;
                    result[cashList[i].nominal] << SItem_Data(dispenser, i, count);
                }
            }
        }
    }

    return result;
}

//---------------------------------------------------------------------------
void CashDispenserManager::saveCashCount() {
    foreach (auto configName, m_CurrencyCashList.keys()) {
        QStringList parameterValueList;

        foreach (auto cash, m_CurrencyCashList.value(configName)) {
            parameterValueList
                << QString("%1:%2:%3").arg(cash.currencyName).arg(cash.nominal).arg(cash.count);
        }

        m_Database->setDeviceParam(configName,
                                   PPSDK::CDatabaseConstants::Parameters::CashUnits,
                                   parameterValueList.join(";"));
    }
}

//---------------------------------------------------------------------------
void CashDispenserManager::loadCashList() {
    m_CurrencyCashList.clear();

    for (auto it = m_Dispensers.begin(); it != m_Dispensers.end(); ++it) {
        const QString &configPath = *it;
        QString cashUnitData =
            m_Database->getDeviceParam(configPath, PPSDK::CDatabaseConstants::Parameters::CashUnits)
                .toString();
        QStringList cashUnits = cashUnitData.split(";", Qt::SkipEmptyParts);

        for (const auto &i : cashUnits) {
            QStringList unit = i.split(":");
            int count = unit[2].toInt();
            PPSDK::SCashUnit cashUnit(unit[0], unit[1].toInt(), count);
            m_CurrencyCashList[configPath] << cashUnit;
        }

        setCashList(it.key());
    }
}

//---------------------------------------------------------------------------
bool CashDispenserManager::getItem_Data(SDK::PaymentProcessor::TPaymentAmount aAmount,
                                        TItem_DataSet &aItemData,
                                        TItem_DataSetIt &aItemDataSetIt) {
    if (aItemData.isEmpty()) {
        return false;
    }

    QList<int> nominals = aItemData.keys();

    if (*std::min_element(nominals.begin(), nominals.end()) > aAmount) {
        return false;
    }

    int nominal = *std::max_element(nominals.begin(), nominals.end());

    if (nominal > aAmount) {
        std::sort(nominals.begin(), nominals.end());
        QList<int>::iterator it = std::lower_bound(nominals.begin(), nominals.end(), aAmount);

        nominal = *it;

        if (nominal > aAmount && it != nominals.begin()) {
            it--;
            nominal = *it;
        }
    }

    aItemDataSetIt = TItem_DataSetIt(aItemData.find(nominal));

    return true;
}

//---------------------------------------------------------------------------
PPSDK::TPaymentAmount CashDispenserManager::canDispense(PPSDK::TPaymentAmount aRequiredAmount) {
    TItem_DataSet itemDataSet = getItem_DataSet(aRequiredAmount);
    TItem_DataSetIt itemDataSetIt;
    PPSDK::TPaymentAmount dispensingAmount = 0;

    while (getItem_Data(aRequiredAmount - dispensingAmount, itemDataSet, itemDataSetIt)) {
        int nominal = itemDataSetIt.key();
        int requiredCount = int(aRequiredAmount - dispensingAmount) / nominal;
        int availableCount = std::accumulate(
            itemDataSetIt->begin(),
            itemDataSetIt->end(),
            0,
            [](int aCount, const SItem_Data &data) -> int { return aCount + data.count; });
        int count = qMin(requiredCount, availableCount);
        dispensingAmount += count * nominal;

        if (count == availableCount) {
            itemDataSet.erase(itemDataSetIt);
        }
    }

    return dispensingAmount;
}

//---------------------------------------------------------------------------
void CashDispenserManager::dispense(PPSDK::TPaymentAmount aAmount) {
    if (qFuzzyIsNull(m_Amounts.toDispensing) && qFuzzyIsNull(m_Amounts.dispensed)) {
        m_Amounts.toDispensing = aAmount;
        toLog(LogLevel::Normal, QString("Amount to dispensing = %1").arg(aAmount, 0, 'f', 2));
    }

    TItem_DataSet itemDataSet = getItem_DataSet(aAmount);
    TItem_DataSetIt itemDataSetIt;

    if (!getItem_Data(aAmount, itemDataSet, itemDataSetIt)) {
        m_Amounts = SAmounts(0, 0);

        toLog(LogLevel::Warning,
              "Send dispensed total amount = 0 due to absence of available dispensing resources");

        emit dispensed(0);
    } else {
        int requiredCount = int(aAmount) / itemDataSetIt.key();
        SItem_Data &data = *itemDataSetIt->begin();
        int count = qMin(data.count, requiredCount);

        data.dispenser->dispense(data.unit, count);
    }
}

//---------------------------------------------------------------------------
PPSDK::TCashUnitsState CashDispenserManager::getCashUnitsState() {
    return m_CurrencyCashList;
}

//---------------------------------------------------------------------------
bool CashDispenserManager::setCashUnitsState(const QString &aDeviceConfigurationName,
                                             const PPSDK::TCashUnitList &aCashUnitList) {
    if (!m_CurrencyCashList.contains(aDeviceConfigurationName)) {
        toLog(LogLevel::Error,
              QString("Unknown cash dispenser device %1.").arg(aDeviceConfigurationName));

        return false;
    }

    if (m_CurrencyCashList.value(aDeviceConfigurationName).size() != aCashUnitList.size()) {
        toLog(LogLevel::Error,
              QString("Incorrect cash unit count for device %1.").arg(aDeviceConfigurationName));

        return false;
    }

    m_CurrencyCashList[aDeviceConfigurationName] = aCashUnitList;
    setCashList(m_Dispensers.key(aDeviceConfigurationName));

    saveCashCount();

    return true;
}

//---------------------------------------------------------------------------
bool CashDispenserManager::storeNotes(QObject *aSender, int aUnit, int aItems) {
    auto *dispenser = dynamic_cast<DSDK::IDispenser *>(aSender);

    if (!dispenser) {
        toLog(LogLevel::Error,
              "Receive dispense signal, but sender not have DSDK::IDispenser interface.");
        return false;
    }

    QString configurationName = m_Dispensers.value(dispenser);

    PPSDK::SCashUnit *cashUnit = &m_CurrencyCashList[configurationName][aUnit];

    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();

    QList<PPSDK::SNote> notes;

    for (int i = 0; i < aItems; i++) {
        notes.push_back(PPSDK::SNote(
            PPSDK::EAmountType::Bill, cashUnit->nominal, settings->getCurrencySettings().id, ""));
    }

    return m_PaymentDatabase->addChangeNote(
        PaymentService::instance(m_Application)->getChangeSessionRef(), notes);
}

//---------------------------------------------------------------------------
