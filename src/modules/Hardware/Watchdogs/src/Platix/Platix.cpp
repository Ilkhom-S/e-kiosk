/* @file Сторожевой таймер Platix. */

#include "Platix.h"

using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
Platix::Platix() {
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
    m_PortParameters[EParameters::Parity].append(EParity::No);

    m_DeviceName = "Platix";
}

//----------------------------------------------------------------------------
bool Platix::isConnected() {
    return processCommand(CPlatix::Commands::GetID);
}

//----------------------------------------------------------------------------
bool Platix::reset(const QString & /*aLine*/) {
    if (!checkConnectionAbility()) {
        return false;
    }

    return processCommand(CPlatix::Commands::ResetModem);
}

//-----------------------------------------------------------------------------
void Platix::onPing() {
    processCommand(CPlatix::Commands::Poll);
}

//--------------------------------------------------------------------------------
ushort Platix::calcCRC(const QByteArray &aData) {
    ushort sum = 0;

    for (char i : aData) {
        sum += uchar(CPlatix::Sync - i);
    }

    return 256 - sum;
}

//--------------------------------------------------------------------------------
bool Platix::check(const QByteArray &aAnswer) {
    if (aAnswer.size() < CPlatix::MinAnswerSize) {
        toLog(LogLevel::Error,
              QString("Platix: The length of the packet is less than %1 bytes")
                  .arg(CPlatix::MinAnswerSize));
        return false;
    }

    if (aAnswer[0] != CPlatix::Sync) {
        toLog(LogLevel::Error,
              QString("Platix: Invalid first byte (prefix) = %1, need %2")
                  .arg(ProtocolUtils::toHexLog<char>(aAnswer[0]))
                  .arg(ProtocolUtils::toHexLog(CPlatix::Sync)));
        return false;
    }

    int length = uchar(aAnswer[1]);

    if (length < aAnswer.size()) {
        toLog(LogLevel::Error,
              QString("Platix: Invalid length = %1, need %2").arg(aAnswer.size()).arg(length));
        return false;
    }

    ushort answerCRC = calcCRC(aAnswer.left(length - 1));
    ushort crc = aAnswer[length - 1];

    if (answerCRC != crc) {
        toLog(LogLevel::Error,
              QString("Platix: Invalid CRC = %1, need %2")
                  .arg(ProtocolUtils::toHexLog(answerCRC))
                  .arg(ProtocolUtils::toHexLog(crc)));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool Platix::processCommand(char aCommand) {
    QByteArray request;
    request.append(CPlatix::Sync);
    request.append(CPlatix::CommandSize);
    request.append(aCommand);
    QString crc = QString("%1").arg(calcCRC(request), 4, 16, QChar(ASCII::Zero));
    request.append(ProtocolUtils::getBufferFrom_String(crc));

    if (!m_IOPort->write(request)) {
        return false;
    }

    if ((aCommand == CPlatix::Commands::RebootPC) || (aCommand == CPlatix::Commands::ResetModem)) {
        return true;
    };

    QByteArray answer;

    // TODO: чтение данных с контролем длины
    return m_IOPort->read(answer) && check(answer);
}

//----------------------------------------------------------------------------
