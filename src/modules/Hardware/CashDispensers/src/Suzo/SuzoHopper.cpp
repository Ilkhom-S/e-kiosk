/* @file Диспенсер купюр Puloon. */

#include "SuzoHopper.h"

#include <QtCore/qmath.h>

#include "SuzoHopperData.h"
#include "SuzoHopperModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
SuzoHopper::SuzoHopper() {
    // Параметры порта.
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::Parity].append(EParity::No);

    // данные устройства
    m_Models = getModelList();
    m_DeviceName = CCCTalk::Dispenser::Models::Default;
    m_Address = CCCTalk::Address::Hopper2;
    m_ProtocolTypes = getProtocolTypes();
    m_AllModelData = PAllModelData(new CCCTalk::Dispenser::CModelData());
    m_units = 1;
    m_SingleMode = false;
    m_resetIsPossible = false;

    setConfigParameter(CHardware::ProtocolType, CHardware::CashDevice::CCTalkTypes::CRC8);
}

//---------------------------------------------------------------------------
QStringList SuzoHopper::getProtocolTypes() {
    return QStringList() << CHardware::CashDevice::CCTalkTypes::CRC8;
}

//--------------------------------------------------------------------------------
bool SuzoHopper::updateParameters() {
    if (!CCTalkDeviceBase<PortDispenser>::updateParameters()) {
        return false;
    }

    // ответа нет. Неизвестно, работает команда или нет.
    processCommand(CCCTalk::Command::SetPayoutCapacity, CSuzo::MaxPayoutCapacity);

    return true;
}

//---------------------------------------------------------------------------
TResult SuzoHopper::execCommand(const QByteArray &aCommand,
                                const QByteArray &aCommandData,
                                QByteArray *aAnswer) {
    TResult result;
    int index = 0;

    do {
        if (index) {
            SleepHelper::msleep(CSuzo::Pause::CommandIteration);
        }

        result = CCTalkDeviceBase<PortDispenser>::execCommand(aCommand, aCommandData, aAnswer);
    } while (!result && (++index < CSuzo::CommandMaxIteration) &&
             (aCommand != QByteArray(1, CCCTalk::Command::SetPayoutCapacity)));

    return result;
}

//---------------------------------------------------------------------------
bool SuzoHopper::setSingleMode(bool aEnable) {
    QByteArray data;

    if (!processCommand(CCCTalk::Command::GetVariables, &data)) {
        return false;
    }

    data = data.left(3) + char(aEnable);

    if (!processCommand(CCCTalk::Command::SetVariables, data)) {
        return false;
    }

    m_SingleMode = aEnable;

    return true;
}

//---------------------------------------------------------------------------
bool SuzoHopper::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray answer;
    TResult result = processCommand(CCCTalk::Command::TestHopper, &answer);

    if (!result) {
        if (!CommandResult::PresenceErrors.contains(result)) {
            return false;
        }

        aStatusCodes.insert(DeviceStatusCode::Error::Unknown);

        return true;
    }

    TDeviceCodeSpecifications testDeviceCodeSpecification;
    CSuzo::DeviceCodeSpecification.getSpecification(answer, testDeviceCodeSpecification);

    foreach (const SDeviceCodeSpecification &specification, testDeviceCodeSpecification) {
        aStatusCodes.insert(specification.statusCode);

        if ((answer != m_lastDeviceStatusCodes) && !specification.description.isEmpty()) {
            SStatusCodeSpecification statusCodeData =
                m_StatusCodesSpecification->value(specification.statusCode);
            LogLevel::Enum logLevel = getLogLevel(statusCodeData.warningLevel);

            toLog(logLevel,
                  m_DeviceName + QString(": %1 -> %2")
                                     .arg(specification.description)
                                     .arg(statusCodeData.description));
        }
    }

    m_lastDeviceStatusCodes = answer;

    return true;
}

//---------------------------------------------------------------------------
bool SuzoHopper::setEnable(bool aEnabled) {
    char enabled = aEnabled ? CSuzo::Enable : ASCII::NUL;
    bool result = processCommand(CCCTalk::Command::EnableHopper, QByteArray(1, enabled));

    if (!aEnabled) {
        SleepHelper::msleep(CSuzo::Pause::Disabling);
    }

    return result;
}

//---------------------------------------------------------------------------
bool SuzoHopper::getDispensingStatus(CSuzo::SStatus &aStatus) {
    QByteArray data;

    if (!processCommand(CCCTalk::Command::GetHopperStatus, &data)) {
        return false;
    }

    aStatus = CSuzo::SStatus(data[0], data[1], data[2], data[3]);

    return true;
}

//--------------------------------------------------------------------------------
void SuzoHopper::performDispense(int aUnit, int aItems) {
    if (!isWorkingThread()) {
        QMetaObject::invokeMethod(
            this, "performDispense", Qt::QueuedConnection, Q_ARG(int, aUnit), Q_ARG(int, aItems));

        return;
    }

    if (!setSingleMode(true) || !setEnable(true)) {
        return;
    }

    QString productCode = getDeviceParameter(CDeviceData::ProductCode).toString();
    bool needSerialNumberForDispense = CSuzo::isNeedSerialNumberForDispense(productCode);

    TStatusCodes statusCodes;

    if (!getStatus(statusCodes) ||
        !getStatusCollection(statusCodes)[EWarningLevel::Error].isEmpty()) {
        return;
    }

    QByteArray commandData(1, char(aItems));
    QByteArray answer;

    if (needSerialNumberForDispense) {
        int serialNumber = getDeviceParameter(CDeviceData::SerialNumber).toInt();
        char serialNumberData[3] = {char(uchar(serialNumber)),
                                    char(uchar(serialNumber / 0x100)),
                                    char(uchar(serialNumber / 0x10000))};
        commandData.prepend(serialNumberData, 3);
    }

    if (!processCommand(CCCTalk::Command::Dispense, commandData, &answer)) {
        return;
    }

    CSuzo::SStatus status;

    while (getDispensingStatus(status) && status.remains) {
        SleepHelper::msleep(CSuzo::Pause::Dispensing);
    }

    emitDispensed(0, status.paid);

    if (status.unpaid) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Cannot pay out %1 coins").arg(status.unpaid));

        onPoll();
    }

    setEnable(false);
}

//--------------------------------------------------------------------------------
QStringList SuzoHopper::getModelList() {
    return CCCTalk::Dispenser::CModelData().getModels(false);
}

//--------------------------------------------------------------------------------
