/* @file Купюроприемник Cashcode GX на протоколе CCNet. */

#include "CCNetCashcodeGX.h"

#include "CCNetCashAcceptorConstants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
CCNetCashcodeGX::CCNetCashcodeGX() {
    // данные устройства
    m_DeviceName = CCCNet::Models::CashcodeGX;
    m_SupportedModels = QStringList() << m_DeviceName;
    m_NeedChangeBaudrate = true;

    setConfigParameter(CHardware::CashAcceptor::InitializeTimeout,
                       CCCNetCashcodeGX::ExitInitializeTimeout);
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::checkConnectionAbility() {
    if (!CCNetCashAcceptorBase::checkConnectionAbility()) {
        return false;
    }

    TPortParameters portParameters;
    m_IOPort->getParameters(portParameters);
    ERTSControl::Enum RTS =
        (m_IOPort->getType() == EPortTypes::COM) ? ERTSControl::Disable : ERTSControl::Enable;

    portParameters[EParameters::RTS] = RTS;

    m_PortParameters[EParameters::RTS].clear();
    m_PortParameters[EParameters::RTS].append(RTS);

    return m_IOPort->setParameters(portParameters);
}

//---------------------------------------------------------------------------------
TResult CCNetCashcodeGX::performCommand(const QByteArray &aCommand,
                                        const QByteArray &aCommandData,
                                        QByteArray *aAnswer) {
    if (m_IOPort->getType() == SDK::Driver::EPortTypes::VirtualCOM) {
        QVariantMap configuration;
        configuration.insert(CHardware::Port::COM::WaitResult, true);
        // m_IOPort->setDeviceConfiguration(configuration);
    }

    return CCNetCashAcceptorBase::performCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::processReset() {
    bool result = processCommand(CCCNet::Commands::Reset);

    m_IOPort->close();
    SleepHelper::msleep(CCCNetCashcodeGX::ResetPause);
    m_IOPort->open();

    bool wait = waitNotBusyPowerUp();

    return (result && wait) || !m_ForceWaitResetCompleting;
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::processUpdating(const QByteArray &aBuffer, int aSectionSize) {
    int sections = int(std::ceil(double(aBuffer.size()) / aSectionSize));
    toLog(LogLevel::Normal,
          m_DeviceName + QString(": section size for updating the firmware = %1, buffer size = %2, "
                                "amount of sections = %3")
                            .arg(aSectionSize)
                            .arg(aBuffer.size())
                            .arg(sections));

    int repeat = 0;
    bool result = true;

    for (int i = 0; i < sections; ++i) {
        uint address = i * aSectionSize;
        QByteArray buffer = aBuffer.mid(address, aSectionSize);
        buffer += QByteArray(aSectionSize - buffer.size(), ASCII::NUL);

        if (!processBlockUpdating(qToBigEndian(address) >> 8, buffer, repeat, i)) {
            result = false;

            break;
        }
    }

    SleepHelper::msleep(CCCNet::ExitUpdatingPause);

    return result;
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::performBaudRateChanging(const TPortParameters &aPortParameters) {
    int baudRate = aPortParameters[EParameters::BaudRate];
    QString hexBaudRate =
        QString("%1").arg(qToBigEndian(uint(baudRate)) >> 8, 6, 16, QChar(ASCII::Zero));

    return processCommand(CCCNet::Commands::UpdatingFirmware::SetBaudRate,
                          ProtocolUtils::getBufferFromString(hexBaudRate)) &&
           m_IOPort->setParameters(aPortParameters);
}

//--------------------------------------------------------------------------------
