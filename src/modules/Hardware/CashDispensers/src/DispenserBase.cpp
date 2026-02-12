/* @file Диспенсер. */

#include <QtCore/QWriteLocker>
#include <QtCore/qmath.h>

#include <Hardware/Dispensers/DispenserBase.h>

#include "Hardware/CashAcceptors/CashAcceptorStatusesDescriptions.h"
#include "Hardware/Dispensers/DispenserStatusesDescriptions.h"

using namespace SDK::Driver;

//---------------------------------------------------------------------------
template <class T>
DispenserBase<T>::DispenserBase()
    : m_units(0), m_needGetUnits(false), m_unitError(false), m_resetIsPossible(true) {
    // описатель статус-кодов
    this->m_StatusCodesSpecification =
        DeviceStatusCode::PSpecifications(new DispenserStatusCode::CSpecifications());
    DeviceStatusCode::PSpecifications billAcceptorStatusCodeSpecification =
        DeviceStatusCode::PSpecifications(new BillAcceptorStatusCode::CSpecifications());
    QMap<int, SStatusCodeSpecification> billAcceptorStatusCodeData =
        billAcceptorStatusCodeSpecification->data();

    for (auto it = billAcceptorStatusCodeData.begin(); it != billAcceptorStatusCodeData.end();
         ++it) {
        this->m_StatusCodesSpecification->data().insert(it.key(), it.value());
    }

    // восстановимые ошибки
    this->m_RecoverableErrors.insert(DispenserStatusCode::Error::AllUnitsEmpty);
}

//--------------------------------------------------------------------------------
template <class T> bool DispenserBase<T>::updateParameters() {
    this->processDeviceData();
    this->adjustUnitList(m_unitError || m_unitData.isEmpty());

    if (m_units == 0) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": No units.");
        return false;
    }

    if (m_resetIsPossible && !this->reset()) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": Failed to reset.");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool DispenserBase<T>::reset() {
    return true;
}

//---------------------------------------------------------------------------
template <class T> void DispenserBase<T>::setUnitList(const TUnitData &aUnitData) {
    m_unitConfigData = aUnitData;

    applyUnitList();
}

//---------------------------------------------------------------------------
template <class T> void DispenserBase<T>::applyUnitList() {
    // Явно указываем адрес метода через имя шаблона класса
    START_IN_WORKING_THREAD(&DispenserBase<T>::applyUnitList);

    if (!m_unitConfigData.isEmpty()) {
        // Не забываем this-> для вызова других методов в шаблонах на macOS
        this->adjustUnitList(true);
    }
}

//---------------------------------------------------------------------------
template <class T> void DispenserBase<T>::adjustUnitList(bool aConfigData) {
    m_unitError = (m_units == 0);

    if (m_unitError) {
        return;
    }

    TUnitData &unitData = aConfigData ? m_unitConfigData : m_unitData;
    m_unitData = unitData.mid(0, m_units);
    int newUnits = m_unitData.size();

    if (newUnits < m_units) {
        m_unitData << TUnitData(m_units - newUnits, 0);
    }

    this->onPoll();

    for (int i = 0; i < unitData.size(); ++i) {
        if ((this->m_StatusCollection.contains(CDispenser::StatusCodes::Data[i].empty) ||
             this->m_StatusCollection.contains(DispenserStatusCode::Error::AllUnitsEmpty)) &&
            unitData[i]) {
            this->emitUnitEmpty(i, " during status filtration");
        }
    }
}

//---------------------------------------------------------------------------
template <class T> void DispenserBase<T>::emitUnitEmpty(int aUnit, const QString &aLog) {
    m_unitData[aUnit] = 0;
    this->toLog(LogLevel::Warning,
                this->m_DeviceName + QString(": emit emptied unit %1%2").arg(aUnit).arg(aLog));

    emit this->unitEmpty(aUnit);
}

//---------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::emitDispensed(int aUnit, int aItems, const QString &aLog) {
    this->toLog(
        LogLevel::Normal,
        this->m_DeviceName +
            QString(": emit dispensed %1 items from %2 unit%3").arg(aItems).arg(aUnit).arg(aLog));

    emit this->dispensed(aUnit, aItems);
}

//---------------------------------------------------------------------------
template <class T> bool DispenserBase<T>::isDeviceReady(int aUnit) {
    MutexLocker locker(&this->m_ExternalMutex);

    if (!this->m_PostPollingAction || (this->m_Initialized != ERequestStatus::Success) ||
        (aUnit >= m_units) || !this->m_StatusCollection.isEmpty(EWarningLevel::Error)) {
        return false;
    }

    using namespace DispenserStatusCode::Warning;

    auto isEmpty = [&](int aCDStatusCode, int aCDUnit) -> bool {
        return this->m_StatusCollection.contains(aCDStatusCode) && (aUnit == aCDUnit);
    };

    return !((aUnit != -1) && (isEmpty(Unit0Empty, 0) || isEmpty(Unit1Empty, 1) ||
                               isEmpty(Unit2Empty, 2) || isEmpty(Unit3Empty, 3)));
}

//--------------------------------------------------------------------------------
template <class T> void DispenserBase<T>::checkUnitStatus(TStatusCodes &aStatusCodes, int aUnit) {
    CDispenser::StatusCodes::SData &data = CDispenser::StatusCodes::Data.data()[aUnit];

    if (aStatusCodes.contains(data.empty)) {
        aStatusCodes.remove(data.nearEmpty);

        if (this->m_unitData.size() > aUnit) {
            this->m_unitData[aUnit] = 0;
        }
    }

    if ((this->m_Initialized != ERequestStatus::InProcess) && (this->m_unitData.size() > aUnit) &&
        (this->m_unitData[aUnit] == 0)) {
        aStatusCodes.insert(data.empty);
        aStatusCodes.remove(data.nearEmpty);
    }
}

//--------------------------------------------------------------------------------
template <class T> void DispenserBase<T>::cleanStatusCodes(TStatusCodes &aStatusCodes) {
    using namespace DispenserStatusCode::Warning;
    using namespace DispenserStatusCode::Error;

    aStatusCodes.remove(DispenserStatusCode::OK::SingleMode);
    aStatusCodes.remove(DispenserStatusCode::OK::Locked);

    for (int i = 0; i < this->m_units; ++i) {
        this->checkUnitStatus(aStatusCodes, i);
    }

    TStatusCodes allEmpty =
        TStatusCodes(CDispenser::StatusCodes::AllEmpty.mid(0, this->m_units).begin(),
                     CDispenser::StatusCodes::AllEmpty.mid(0, this->m_units).end());
    TStatusCodes allNearEmpty =
        TStatusCodes(CDispenser::StatusCodes::AllNearEmpty.mid(0, this->m_units).begin(),
                     CDispenser::StatusCodes::AllNearEmpty.mid(0, this->m_units).end());

    if ((aStatusCodes & allEmpty) == allEmpty) {
        aStatusCodes -= allEmpty + allNearEmpty;
        aStatusCodes.insert(DispenserStatusCode::Error::AllUnitsEmpty);
    } else if ((aStatusCodes & allNearEmpty) == allNearEmpty) {
        aStatusCodes -= allNearEmpty;
        aStatusCodes.insert(DispenserStatusCode::Warning::AllUnitsNearEmpty);
    }

    T::cleanStatusCodes(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                         const TStatusCollection &aOldStatusCollection) {
    if (this->m_needGetUnits && (this->m_units != 0)) {
        this->m_needGetUnits = false;
        this->toLog(LogLevel::Warning, this->m_DeviceName + ": emit units defined");

        emit this->unitsDefined();
    }

    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::emitStatusCodes(TStatusCollection &aStatusCollection,
                                       int /*aExtendedStatus*/) {
    TStatusCodes opened =
        aStatusCollection.value(EWarningLevel::Error) & CDispenser::StatusCodes::AllOpened;
    int extendedStatus = opened.isEmpty() ? EStatus::Actual : EDispenserStatus::CassetteOpened;

    T::emitStatusCodes(aStatusCollection, extendedStatus);
}

//--------------------------------------------------------------------------------
template <class T> int DispenserBase<T>::units() {
    this->m_needGetUnits = (this->m_units == 0);

    return this->m_units;
}

//--------------------------------------------------------------------------------
template <class T> void DispenserBase<T>::dispense(int aUnit, int aItems) {
    if (aItems == 0) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": Nothing for dispense");

        return;
    }

    if (this->m_unitData.size() <= aUnit) {
        this->toLog(LogLevel::Warning,
                    this->m_DeviceName +
                        QString(": emit emptied unit %1 due to no such unit, need max %2")
                            .arg(aUnit)
                            .arg(this->m_units - 1));

        emit this->unitEmpty(aUnit);

        return;
    }
    if (this->m_unitData[aUnit] == 0) {
        this->emitDispensed(aUnit, 0, " due to unit is empty already");

        return;
    }

    this->performDispense(aUnit, aItems);
}

//--------------------------------------------------------------------------------
