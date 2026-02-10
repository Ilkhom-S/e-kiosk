/* @file Купюроприемник на протоколе ccTalk. */

#include "CCTalkCashAcceptor.h"

#include <QtCore/QRegularExpression>
#include <QtCore/qmath.h>

#include "CCTalkCashAcceptorConstants.h"
#include "CCTalkCashAcceptorModelData.h"

namespace CCCTalk {
namespace CashAcceptor {
namespace Models {
const char NV200Spectral[] = "NV200 Spectral";
} // namespace Models
} // namespace CashAcceptor
} // namespace CCCTalk

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
CCTalkCashAcceptor::CCTalkCashAcceptor() {
    m_Models = getModelList();

    m_Address = CCCTalk::Address::BillAcceptor;
    m_ParInStacked = true;
    m_ErrorData = PErrorData(new CCCTalk::ErrorData());
    m_AllModelData = PAllModelData(new CCCTalk::CashAcceptor::CModelData());
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::processReset() {
    toLog(LogLevel::Normal, m_DeviceName + ": processing command reset");

    if (!processCommand(CCCTalk::Command::Reset)) {
        return false;
    }

    SleepHelper::msleep(CCCTalk::ResetPause);

    m_EventIndex = 0;

    return true;
}

//--------------------------------------------------------------------------------
bool CCTalkCashAcceptor::setDefaultParameters() {
    return enableMoneyAcceptingMode(false) &&
           processCommand(CCCTalk::Command::ModifyBillOperatingMode,
                          QByteArray(1, CCCTalk::EscrowEnabling));
}

//--------------------------------------------------------------------------------
void CCTalkCashAcceptor::processDeviceData() {
    TCCTalkCashAcceptor::processDeviceData();

    QByteArray answer;

    if (processCommand(CCCTalk::Command::GetCurrencyRevision, &answer)) {
        setDeviceParameter(CDeviceData::CashAcceptors::BillSet, ProtocolUtils::clean(answer));
    }
}

//--------------------------------------------------------------------------------
double CCTalkCashAcceptor::parseFWVersion(const QByteArray &aAnswer) {
    return aAnswer.mid(6, 3).toDouble() / 100.0;
}

//--------------------------------------------------------------------------------
bool CCTalkCashAcceptor::loadParTable() {
    m_ScalingFactors.clear();

    QByteArray answer;

    for (char i = 1; i <= CCCTalk::NominalCount; ++i) {
        if (processCommand(CCCTalk::Command::GetBillID, QByteArray(1, i), &answer)) {
            CCCTalk::SCurrencyData currencyData;
            QByteArray countryCode = answer.left(2);

            if (parseCurrencyData(countryCode, currencyData)) {
                bool OK = false;
                QByteArray valueData = answer.mid(2, 4);
                int value = valueData.toInt(&OK);

                if (OK) {
                    double nominal = value * m_ScalingFactors[countryCode];
                    int countryCodeId = currencyData.code;

                    SPar par(nominal, countryCodeId, ECashReceiver::BillAcceptor);
                    par.currency = CurrencyCodes.key(currencyData.code);

                    {
                        MutexLocker locker(&m_ResourceMutex);

                        m_EscrowParTable.data().insert(i, par);
                    }
                } else {
                    toLog(LogLevel::Error,
                          QString("%1: Failed to parse nominal value %2 (0x%3)")
                              .arg(m_DeviceName)
                              .arg(valueData.data())
                              .arg(valueData.toHex().toUpper().data()));
                }
            }
        } else {
            toLog(LogLevel::Error,
                  m_DeviceName + QString(": Failed to get bill %1 data").arg(uint(i)));
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool CCTalkCashAcceptor::parseCurrencyData(const QByteArray &aData,
                                           CCCTalk::SCurrencyData &aCurrencyData) {
    if (!TCCTalkCashAcceptor::parseCurrencyData(aData, aCurrencyData)) {
        return false;
    }

    if (!m_ScalingFactors.contains(aData)) {
        QByteArray answer;

        if (!processCommand(CCCTalk::Command::GetCountryScalingFactor, aData, &answer)) {
            toLog(LogLevel::Error,
                  m_DeviceName + ": Failed to get scaling factor for " + aCurrencyData.country);
            return false;
        }

        ushort base = qToBigEndian(answer.left(2).toHex().toUShort(nullptr, 16));
        double value = base * qPow(10, -1 * uchar(answer[2]));
        m_ScalingFactors.insert(aData, value);
    }

    return true;
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::getBufferedStatuses(QByteArray &aAnswer) {
    return processCommand(CCCTalk::Command::GetBufferedBillStatuses, &aAnswer);
}

//---------------------------------------------------------------------------
void CCTalkCashAcceptor::parseCreditData(uchar aCredit, uchar aError, TStatusCodes &aStatusCodes) {
    if (!aError) {
        aStatusCodes.insert(BillAcceptorStatusCode::BillOperation::Stacked);
        m_Codes.insert(CCCTalk::StackedDeviceCode);
    } else {
        aStatusCodes.insert(BillAcceptorStatusCode::BillOperation::Escrow);
        m_Codes.insert(CCCTalk::EscrowDeviceCode);
    }

    m_EscrowPars = TPars() << m_EscrowParTable[aCredit];
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::stack() {
    if (!checkConnectionAbility() || (m_Initialized != ERequestStatus::Success) || m_CheckDisable) {
        return false;
    }

    return route(true);
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::reject() {
    if (!checkConnectionAbility() || (m_Initialized == ERequestStatus::Fail)) {
        return false;
    }

    return route(false);
}

//---------------------------------------------------------------------------
bool CCTalkCashAcceptor::route(bool aDirection) {
    if (!checkConnectionAbility() || (m_Initialized == ERequestStatus::Fail)) {
        return false;
    }

    QByteArray commandData(1, aDirection ? CCCTalk::Stack : CCCTalk::Return);
    QByteArray answer;

    if (!processCommand(CCCTalk::Command::RouteBill, commandData, &answer)) {
        return false;
    }

    if (answer.isEmpty()) {
        m_VirtualRouting.direction = aDirection;
        m_VirtualRouting.active = true;

        return true;
    }

    char data = answer[0];
    QString log =
        QString("%1: Failed to %2 bill").arg(m_DeviceName).arg(aDirection ? "stack" : "return");

    if (m_EscrowPars.isEmpty()) {
        log += ", but escrow pars are empty";
    } else {
        SPar par = *m_EscrowPars.begin();
        log += QString(" %1 (%2)").arg(par.nominal).arg(par.currency);
    }

    if (data == CCCTalk::RoutingErrors::EmptyEscrow) {
        toLog(LogLevel::Error, log + ", due to no bill in escrow");
        return false;
    } if (data == CCCTalk::RoutingErrors::Unknown) {
        toLog(LogLevel::Error, log + ", due to an error");
        return false;
    }

    toLog(LogLevel::Error, log + ", due to unknown error");

    return false;
}

//--------------------------------------------------------------------------------
void CCTalkCashAcceptor::cleanSpecificStatusCodes(TStatusCodes &aStatusCodes) {
    if (m_ModelData.model != CCCTalk::CashAcceptor::Models::NV200Spectral) {
        return;
    }

    using namespace BillAcceptorStatusCode;

    if (!aStatusCodes.contains(BillOperation::Escrow) &&
        !(m_VirtualRouting.direction && aStatusCodes.contains(BillOperation::Stacking)) &&
        (m_VirtualRouting.direction || !aStatusCodes.contains(Busy::Returning))) {
        m_VirtualRouting.active = false;
    }

    TStatusCodes oldStatusCodes = aStatusCodes;
    int routing = m_VirtualRouting.direction ? BillOperation::Stacking : Busy::Returning;

    if (m_VirtualRouting.active) {
        replaceConformedStatusCodes(aStatusCodes, BillOperation::Escrow, routing);
    }

    QStringList log;
    auto addLog = [&](int aStatusCode) {
        if (aStatusCodes != oldStatusCodes) {
            log << QString("Changed to %1: %2")
                       .arg(getStatusTranslations(TStatusCodes() << aStatusCode, false))
                       .arg(getStatusTranslations(oldStatusCodes - aStatusCodes, false));
}
    };

    addLog(routing);
    QStringList removed;

    foreach (int statusCode, aStatusCodes) {
        SStatusCodeSpecification data = m_StatusCodesSpecification->value(statusCode);
        auto status = ECashAcceptorStatus::Enum(data.status);
        CCashAcceptor::TStatuses statuses;
        statuses[status].insert(statusCode);

        if ((m_Enabled && isDisabled(statuses)) || (!m_Enabled && isEnabled(statuses))) {
            aStatusCodes -= statusCode;
            removed << data.description;
        }
    }

    if (!removed.isEmpty()) {
        log << "Removed: " + removed.join(CDevice::StatusSeparator);
    }

    int enabling = m_Enabled ? Normal::Enabled : Normal::Disabled;

    if (aStatusCodes.isEmpty()) {
        log << "Restored: " + m_StatusCodesSpecification->value(enabling).description;
        aStatusCodes.insert(enabling);
    }

    TStatusCodes longStatusCodes = getLongStatusCodes();
    oldStatusCodes = aStatusCodes;

    foreach (int statusCode, aStatusCodes) {
        if (!longStatusCodes.contains(statusCode)) {
            replaceConformedStatusCodes(aStatusCodes, statusCode, enabling);
        }
    }

    addLog(enabling);

    if (!log.isEmpty()) {
        toLog(LogLevel::Normal, m_DeviceName + ": " + log.join(". "));
    }
}

//--------------------------------------------------------------------------------
QStringList CCTalkCashAcceptor::getModelList() {
    return CCCTalk::CashAcceptor::CModelData().getModels(false);
}

//--------------------------------------------------------------------------------
