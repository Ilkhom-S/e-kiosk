/* @file Базовый класс устройства приема денег на протоколе ccTalk. */

#include "CCTalkAcceptorBase.h"

#include <QtCore/QRegularExpression>
#include <QtCore/qmath.h>

#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"
#include "Hardware/Common/SerialDeviceBase.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

namespace CCCTalk {
const char TeachMode[] = "TM";
} // namespace CCCTalk

//---------------------------------------------------------------------------
template <class T>
CCTalkAcceptorBase<T>::CCTalkAcceptorBase() : m_Enabled(false), m_Currency(Currency::NoCurrency) {
    // параметры порта
    this->m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    this->m_PortParameters[EParameters::Parity].append(EParity::No);

    // данные устройства
    this->m_MaxBadAnswers = 5;
}

//--------------------------------------------------------------------------------
template <class T> bool CCTalkAcceptorBase<T>::enableMoneyAcceptingMode(bool aEnabled) {
    if (!this->processCommand(CCCTalk::Command::AllSetEnable, QByteArray(1, char(aEnabled)))) {
        return false;
    }

    m_Enabled = aEnabled;

    return true;
}

//---------------------------------------------------------------------------
template <class T> QByteArray CCTalkAcceptorBase<T>::getParTableData() {
    QByteArray result = QByteArray(2, ASCII::NUL);

    for (int i = 1; i <= 16; ++i) {
        SPar par = this->m_EscrowParTable[i];

        if (par.enabled && !par.inhibit) {
            int index = (i - 1) / 8;
            result[index] = result[index] | (1 << ((i - 1) % 8));
        }
    }

    return result;
}

//---------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::parseCurrencyData(const QByteArray &aData,
                                              CCCTalk::SCurrencyData &aCurrencyData) {
    if (aData == QByteArray(aData.size(), ASCII::NUL)) {
        return false;
    }

    if (!CCCTalk::CurrencyData.data().contains(aData)) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName + QString(": Unknown country = %1").arg(aData.data()));
        return false;
    }

    aCurrencyData = CCCTalk::CurrencyData[aData];

    if (aCurrencyData.code == Currency::NoCurrency) {
        this->toLog(LogLevel::Error,
                    QString("%1: Unknown currency code for country %2 (code %3)")
                        .arg(this->m_DeviceName)
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
    TDeviceCodes lastCodes(this->m_Codes);
    this->m_Codes.clear();

    QByteArray answer;

    if (!this->m_ErrorData || !getBufferedStatuses(answer)) {
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
    this->m_Codes.insert(fault);

    int statusCode = (fault && (fault == this->m_ModelData.fault))
                         ? BillAcceptorStatusCode::Busy::Unknown
                         : CCCTalk::Fault[fault].statusCode;
    SStatusCodeSpecification statusCodeSpecification =
        this->m_StatusCodesSpecification->value(statusCode);

    if (!lastCodes.contains(fault)) {
        QString description = CCCTalk::Fault.getDescription(answer);
        LogLevel::Enum logLevel = this->getLogLevel(statusCodeSpecification.warningLevel);

        if (description.isEmpty()) {
            this->toLog(logLevel,
                        this->m_DeviceName + QString(": Self check status = ") +
                            statusCodeSpecification.description);
        } else {
            this->toLog(logLevel,
                        this->m_DeviceName + QString(": Self check status = %1 -> %2")
                                                 .arg(description)
                                                 .arg(statusCodeSpecification.description));
        }
    }

    aStatusCodes.insert(statusCode);

    return true;
}

//---------------------------------------------------------------------------
template <class T>
bool CCTalkAcceptorBase<T>::canApplySimpleStatusCodes(const TStatusCodes & /*aStatusCodes*/) {
    return this->m_StatusCollectionHistory.isEmpty();
}

//---------------------------------------------------------------------------
template <class T>
void CCTalkAcceptorBase<T>::parseBufferedStatuses(const QByteArray &aAnswer,
                                                  TStatusCodes &aStatusCodes) {
    int size = aAnswer[0];

    if (!size || (size == this->m_EventIndex)) {
        if (canApplySimpleStatusCodes(aStatusCodes)) {
            aStatusCodes.insert(m_Enabled ? BillAcceptorStatusCode::Normal::Enabled
                                          : BillAcceptorStatusCode::Normal::Disabled);
        } else {
            aStatusCodes += this->getStatusCodes(this->m_StatusCollectionHistory.lastValue());
        }

        return;
    }

    for (int i = 0; i < (size - this->m_EventIndex); ++i) {
        uchar credit = aAnswer[2 * i + 1];
        uchar error = aAnswer[2 * i + 2];

        if (credit) {
            parseCreditData(credit, error, aStatusCodes);
        } else if (error) {
            this->m_Codes.insert(error);

            if (error == this->m_ModelData.error) {
                aStatusCodes.insert(BillAcceptorStatusCode::Busy::Unknown);
            } else {
                int statusCode = this->m_ErrorData->value(error).statusCode;
                aStatusCodes.insert(statusCode);

                SStatusCodeSpecification codeSpecification =
                    this->m_StatusCodesSpecification->value(statusCode);
                QString localDescription = this->m_ErrorData->value(error).description;

                if (!localDescription.isEmpty()) {
                    LogLevel::Enum logLevel = this->getLogLevel(codeSpecification.warningLevel);
                    this->toLog(logLevel,
                                this->m_DeviceName + QString(": %1 -> %2")
                                                         .arg(localDescription)
                                                         .arg(codeSpecification.description));
                }

                if (this->m_ErrorData->value(error).isRejected) {
                    aStatusCodes.insert(BillAcceptorStatusCode::Reject::Unknown);
                } else if (statusCode == DeviceStatusCode::OK::OK) {
                    aStatusCodes.insert(m_Enabled ? BillAcceptorStatusCode::Normal::Enabled
                                                  : BillAcceptorStatusCode::Normal::Disabled);
                }
            }
        }
    }

    this->m_EventIndex = size;
}

//---------------------------------------------------------------------------
template <class T> void CCTalkAcceptorBase<T>::finalizeInitialization() {
    CCTalkDeviceBase<T>::finalizeInitialization();

    this->m_OldFirmware = (this->m_Currency != Currency::NoCurrency) &&
                          (this->m_FWVersion < this->m_ModelData.minVersions[this->m_Currency]);
}

//--------------------------------------------------------------------------------
