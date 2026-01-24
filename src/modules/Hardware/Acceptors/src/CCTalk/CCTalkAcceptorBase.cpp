/* @file Базовый класс устройства приема денег на протоколе ccTalk. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegularExpression>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// System
#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include "CCTalkAcceptorBase.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
template <class T> CCTalkAcceptorBase<T>::CCTalkAcceptorBase() : mEnabled(false), mCurrency(Currency::NoCurrency) {
    // параметры порта
    this->mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    this->mPortParameters[EParameters::Parity].append(EParity::No);

    // данные устройства
    this->mMaxBadAnswers = 5;
}

//--------------------------------------------------------------------------------
template <class T> bool CCTalkAcceptorBase<T>::enableMoneyAcceptingMode(bool aEnabled) {
    if (!this->processCommand(CCCTalk::Command::AllSetEnable, QByteArray(1, char(aEnabled)))) {
        return false;
    }

    mEnabled = aEnabled;

    return true;
}

//---------------------------------------------------------------------------
template <class T> QByteArray CCTalkAcceptorBase<T>::getParTableData() {
    QByteArray result = QByteArray(2, ASCII::NUL);

    for (int i = 1; i <= 16; ++i) {
        SPar par = this->mEscrowParTable[i];

        if (par.enabled && !par.inhibit) {
            int index = (i - 1) / 8;
            result[index] = result[index] | (1 << ((i - 1) % 8));
        }
    }

    return result;
}

//---------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::parseCurrencyData(const QByteArray &aData, CCCTalk::SCurrencyData &aCurrencyData) {
    if (aData == QByteArray(aData.size(), ASCII::NUL)) {
        return false;
    }

    if (!CCCTalk::CurrencyData.data().contains(aData)) {
        this->toLog(LogLevel::Error, this->mDeviceName + QString(": Unknown country = %1").arg(aData.data()));
        return false;
    }

    aCurrencyData = CCCTalk::CurrencyData[aData];

    if (aCurrencyData.code == Currency::NoCurrency) {
        this->toLog(LogLevel::Error, QString("%1: Unknown currency code for country %2 (code %3)")
                                         .arg(this->mDeviceName)
                                         .arg(aCurrencyData.country)
                                         .arg(aData.data()));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> bool CCTalkAcceptorBase<T>::applyParTable() {
    return this->processCommand(CCCTalk::Command::PartialEnable, getParTableData());
}

//---------------------------------------------------------------------------
template <class T> bool CCTalkAcceptorBase<T>::getStatus(TStatusCodes &aStatusCodes) {
    TDeviceCodes lastCodes(this->mCodes);
    this->mCodes.clear();

    QByteArray answer;

    if (!this->mErrorData || !getBufferedStatuses(answer)) {
        return false;
    }

    if (answer.isEmpty()) {
        aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
    } else {
        parseBufferedStatuses(answer, aStatusCodes);
    }

    if (!this->processCommand(CCCTalk::Command::SelfCheck, &answer)) {
        return false;
    }

    uchar fault = answer[0];
    this->mCodes.insert(fault);

    int statusCode = (fault && (fault == this->mModelData.fault)) ? BillAcceptorStatusCode::Busy::Unknown
                                                                  : CCCTalk::Fault[fault].statusCode;
    SStatusCodeSpecification statusCodeSpecification = this->mStatusCodesSpecification->value(statusCode);

    if (!lastCodes.contains(fault)) {
        QString description = CCCTalk::Fault.getDescription(answer);
        LogLevel::Enum logLevel = this->getLogLevel(statusCodeSpecification.warningLevel);

        if (description.isEmpty()) {
            this->toLog(logLevel,
                        this->mDeviceName + QString(": Self check status = ") + statusCodeSpecification.description);
        } else {
            this->toLog(logLevel, this->mDeviceName + QString(": Self check status = %1 -> %2")
                                                          .arg(description)
                                                          .arg(statusCodeSpecification.description));
        }
    }

    aStatusCodes.insert(statusCode);

    return true;
}

//---------------------------------------------------------------------------
template <class T> bool CCTalkAcceptorBase<T>::canApplySimpleStatusCodes(const TStatusCodes & /*aStatusCodes*/) {
    return this->mStatusCollectionHistory.isEmpty();
}

//---------------------------------------------------------------------------
template <class T>
void CCTalkAcceptorBase<T>::parseBufferedStatuses(const QByteArray &aAnswer, TStatusCodes &aStatusCodes) {
    int size = aAnswer[0];

    if (!size || (size == this->mEventIndex)) {
        if (canApplySimpleStatusCodes(aStatusCodes)) {
            aStatusCodes.insert(mEnabled ? BillAcceptorStatusCode::Normal::Enabled
                                         : BillAcceptorStatusCode::Normal::Disabled);
        } else {
            aStatusCodes += this->getStatusCodes(this->mStatusCollectionHistory.lastValue());
        }

        return;
    }

    for (int i = 0; i < (size - this->mEventIndex); ++i) {
        uchar credit = aAnswer[2 * i + 1];
        uchar error = aAnswer[2 * i + 2];

        if (credit) {
            parseCreditData(credit, error, aStatusCodes);
        } else if (error) {
            this->mCodes.insert(error);

            if (error == this->mModelData.error) {
                aStatusCodes.insert(BillAcceptorStatusCode::Busy::Unknown);
            } else {
                int statusCode = this->mErrorData->value(error).statusCode;
                aStatusCodes.insert(statusCode);

                SStatusCodeSpecification codeSpecification = this->mStatusCodesSpecification->value(statusCode);
                QString localDescription = this->mErrorData->value(error).description;

                if (!localDescription.isEmpty()) {
                    LogLevel::Enum logLevel = this->getLogLevel(codeSpecification.warningLevel);
                    this->toLog(logLevel,
                                this->mDeviceName +
                                    QString(": %1 -> %2").arg(localDescription).arg(codeSpecification.description));
                }

                if (this->mErrorData->value(error).isRejected) {
                    aStatusCodes.insert(BillAcceptorStatusCode::Reject::Unknown);
                } else if (statusCode == DeviceStatusCode::OK::OK) {
                    aStatusCodes.insert(mEnabled ? BillAcceptorStatusCode::Normal::Enabled
                                                 : BillAcceptorStatusCode::Normal::Disabled);
                }
            }
        }
    }

    this->mEventIndex = size;
}

//---------------------------------------------------------------------------
template <class T> void CCTalkAcceptorBase<T>::finalizeInitialization() {
    CCTalkDeviceBase<T>::finalizeInitialization();

    this->mOldFirmware =
        (this->mCurrency != Currency::NoCurrency) && (this->mFWVersion < this->mModelData.minVersions[this->mCurrency]);
}

//--------------------------------------------------------------------------------
