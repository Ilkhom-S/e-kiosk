/* @file Диспенсер. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QWriteLocker>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// System
#include "Hardware/CashAcceptors/CashAcceptorStatusesDescriptions.h"
#include <Hardware/Dispensers/DispenserBase.h>
#include "Hardware/Dispensers/DispenserStatusesDescriptions.h"

using namespace SDK::Driver;

//---------------------------------------------------------------------------
template <class T>
DispenserBase<T>::DispenserBase() : mUnits(0), mNeedGetUnits(false), mUnitError(false), mResetIsPossible(true) {
    // описатель статус-кодов
    this->mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new DispenserStatusCode::CSpecifications());
    DeviceStatusCode::PSpecifications billAcceptorStatusCodeSpecification =
        DeviceStatusCode::PSpecifications(new BillAcceptorStatusCode::CSpecifications());
    QMap<int, SStatusCodeSpecification> billAcceptorStatusCodeData = billAcceptorStatusCodeSpecification->data();

    for (auto it = billAcceptorStatusCodeData.begin(); it != billAcceptorStatusCodeData.end(); ++it) {
        this->mStatusCodesSpecification->data().insert(it.key(), it.value());
    }

    // восстановимые ошибки
    this->mRecoverableErrors.insert(DispenserStatusCode::Error::AllUnitsEmpty);
}

//--------------------------------------------------------------------------------
template <class T> bool DispenserBase<T>::updateParameters() {
    this->processDeviceData();
    this->adjustUnitList(mUnitError || mUnitData.isEmpty());

    if (!mUnits) {
        this->toLog(LogLevel::Error, this->mDeviceName + ": No units.");
        return false;
    }

    if (mResetIsPossible && !this->reset()) {
        this->toLog(LogLevel::Error, this->mDeviceName + ": Failed to reset.");
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
    mUnitConfigData = aUnitData;

    applyUnitList();
}

//---------------------------------------------------------------------------
template <class T> void DispenserBase<T>::applyUnitList() {
    // Явно указываем адрес метода через имя шаблона класса
    START_IN_WORKING_THREAD(&DispenserBase<T>::applyUnitList);

    if (!mUnitConfigData.isEmpty()) {
        // Не забываем this-> для вызова других методов в шаблонах на macOS
        this->adjustUnitList(true);
    }
}

//---------------------------------------------------------------------------
template <class T> void DispenserBase<T>::adjustUnitList(bool aConfigData) {
    mUnitError = !mUnits;

    if (mUnitError) {
        return;
    }

    TUnitData &unitData = aConfigData ? mUnitConfigData : mUnitData;
    mUnitData = unitData.mid(0, mUnits);
    int newUnits = mUnitData.size();

    if (newUnits < mUnits) {
        mUnitData << TUnitData(mUnits - newUnits, 0);
    }

    this->onPoll();

    for (int i = 0; i < unitData.size(); ++i) {
        if ((this->mStatusCollection.contains(CDispenser::StatusCodes::Data[i].empty) ||
             this->mStatusCollection.contains(DispenserStatusCode::Error::AllUnitsEmpty)) &&
            unitData[i]) {
            this->emitUnitEmpty(i, " during status filtration");
        }
    }
}

//---------------------------------------------------------------------------
template <class T> void DispenserBase<T>::emitUnitEmpty(int aUnit, const QString &aLog) {
    mUnitData[aUnit] = 0;
    this->toLog(LogLevel::Warning, this->mDeviceName + QString(": emit emptied unit %1%2").arg(aUnit).arg(aLog));

    emit this->unitEmpty(aUnit);
}

//---------------------------------------------------------------------------
template <class T> void DispenserBase<T>::emitDispensed(int aUnit, int aItems, const QString &aLog) {
    this->toLog(LogLevel::Normal,
                this->mDeviceName +
                    QString(": emit dispensed %1 items from %2 unit%3").arg(aItems).arg(aUnit).arg(aLog));

    emit this->dispensed(aUnit, aItems);
}

//---------------------------------------------------------------------------
template <class T> bool DispenserBase<T>::isDeviceReady(int aUnit) {
    MutexLocker locker(&this->mExternalMutex);

    if (!this->mPostPollingAction || (this->mInitialized != ERequestStatus::Success) || (aUnit >= mUnits) ||
        !this->mStatusCollection.isEmpty(EWarningLevel::Error)) {
        return false;
    }

    using namespace DispenserStatusCode::Warning;

    auto isEmpty = [&](int aCDStatusCode, int aCDUnit) -> bool {
        return this->mStatusCollection.contains(aCDStatusCode) && (aUnit == aCDUnit);
    };

    return !((aUnit != -1) &&
             (isEmpty(Unit0Empty, 0) || isEmpty(Unit1Empty, 1) || isEmpty(Unit2Empty, 2) || isEmpty(Unit3Empty, 3)));
}

//--------------------------------------------------------------------------------
template <class T> void DispenserBase<T>::checkUnitStatus(TStatusCodes &aStatusCodes, int aUnit) {
    CDispenser::StatusCodes::SData &data = CDispenser::StatusCodes::Data.data()[aUnit];

    if (aStatusCodes.contains(data.empty)) {
        aStatusCodes.remove(data.nearEmpty);

        if (this->mUnitData.size() > aUnit) {
            this->mUnitData[aUnit] = 0;
        }
    }

    if ((this->mInitialized != ERequestStatus::InProcess) && (this->mUnitData.size() > aUnit) &&
        !this->mUnitData[aUnit]) {
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

    for (int i = 0; i < this->mUnits; ++i) {
        this->checkUnitStatus(aStatusCodes, i);
    }

    TStatusCodes allEmpty = TStatusCodes(CDispenser::StatusCodes::AllEmpty.mid(0, this->mUnits).begin(),
                                         CDispenser::StatusCodes::AllEmpty.mid(0, this->mUnits).end());
    TStatusCodes allNearEmpty = TStatusCodes(CDispenser::StatusCodes::AllNearEmpty.mid(0, this->mUnits).begin(),
                                             CDispenser::StatusCodes::AllNearEmpty.mid(0, this->mUnits).end());

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
    if (this->mNeedGetUnits && this->mUnits) {
        this->mNeedGetUnits = false;
        this->toLog(LogLevel::Warning, this->mDeviceName + ": emit units defined");

        emit this->unitsDefined();
    }

    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template <class T>
void DispenserBase<T>::emitStatusCodes(TStatusCollection &aStatusCollection, int /*aExtendedStatus*/) {
    TStatusCodes opened = aStatusCollection.value(EWarningLevel::Error) & CDispenser::StatusCodes::AllOpened;
    int extendedStatus = opened.isEmpty() ? EStatus::Actual : EDispenserStatus::CassetteOpened;

    T::emitStatusCodes(aStatusCollection, extendedStatus);
}

//--------------------------------------------------------------------------------
template <class T> int DispenserBase<T>::units() {
    this->mNeedGetUnits = !this->mUnits;

    return this->mUnits;
}

//--------------------------------------------------------------------------------
template <class T> void DispenserBase<T>::dispense(int aUnit, int aItems) {
    if (!aItems) {
        this->toLog(LogLevel::Error, this->mDeviceName + ": Nothing for dispense");

        return;
    }

    if (this->mUnitData.size() <= aUnit) {
        this->toLog(
            LogLevel::Warning,
            this->mDeviceName +
                QString(": emit emptied unit %1 due to no such unit, need max %2").arg(aUnit).arg(this->mUnits - 1));

        emit this->unitEmpty(aUnit);

        return;
    } else if (!this->mUnitData[aUnit]) {
        this->emitDispensed(aUnit, 0, " due to unit is empty already");

        return;
    }

    this->performDispense(aUnit, aItems);
}

//--------------------------------------------------------------------------------
