/* @file Протокол ID003. */

#include <QtCore/QElapsedTimer>

#include <Hardware/Protocols/CashAcceptor/ID003.h>

#include "ID003Constants.h"

ushort ID003Protocol::calcCRC16(const QByteArray &aData) {
    ushort crc = 0;

    for (char i : aData) {
        ushort byteCRC = 0;
        ushort value = uchar(crc ^ i);

        for (int j = 0; j < 8; ++j) {
            ushort data = byteCRC >> 1;
            byteCRC = (((byteCRC ^ value) & 1) != 0) ? (data ^ CID003::Polynominal) : data;
            value = value >> 1;
        }

        crc = byteCRC ^ (crc >> 8);
    }

    return crc;
}

//--------------------------------------------------------------------------------
void ID003Protocol::pack(QByteArray &aCommandData) {
    aCommandData.prepend(uchar(aCommandData.size() + 4));
    aCommandData.prepend(CID003::Prefix);

    uint crc = calcCRC16(aCommandData);
    aCommandData.append(uchar(crc));
    aCommandData.append(uchar(crc >> 8));
}

//--------------------------------------------------------------------------------
bool ID003Protocol::check(const QByteArray &aAnswer) {
    // минимальный размер ответа
    if (aAnswer.size() < CID003::MinAnswerSize) {
        toLog(LogLevel::Error,
              QString("ID003: Invalid answer length = %1, need %2 minimum")
                  .arg(aAnswer.size())
                  .arg(CID003::MinAnswerSize));
        return false;
    }

    // первый байт
    char prefix = aAnswer[0];

    if (prefix != CID003::Prefix) {
        toLog(LogLevel::Error,
              QString("ID003: Invalid prefix = %1, need = %2")
                  .arg(ProtocolUtils::toHexLog(prefix))
                  .arg(ProtocolUtils::toHexLog(CID003::Prefix)));
        return false;
    }

    // длина
    ushort length = aAnswer[1] & 0x00FF;

    if (length != aAnswer.size()) {
        toLog(LogLevel::Error,
              QString("ID003: Invalid length = %1, need %2").arg(aAnswer.size()).arg(length));
        return false;
    }

    // CRC
    ushort answerCRC = calcCRC16(aAnswer.mid(0, length - 2));
    ushort crc = qToBigEndian(aAnswer.right(2).toHex().toUShort(nullptr, 16));

    if (crc != answerCRC) {
        toLog(LogLevel::Error,
              QString("ID003: Invalid CRC = %1, need %2")
                  .arg(ProtocolUtils::toHexLog(crc))
                  .arg(ProtocolUtils::toHexLog(answerCRC)));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
TResult ID003Protocol::processCommand(const QByteArray &aCommandData, QByteArray &aAnswerData) {
    QByteArray request = aCommandData;
    pack(request);

    toLog(LogLevel::Normal, QString("ID003: >> {%1}").arg(request.toHex().data()));

    if (!m_Port->write(request) || !getAnswer(aAnswerData)) {
        return CommandResult::Port;
    }

    if (aAnswerData.isEmpty()) {
        return CommandResult::NoAnswer;
    }

    if (!check(aAnswerData)) {
        return CommandResult::Protocol;
    }

    aAnswerData = aAnswerData.mid(2, aAnswerData.size() - 4);

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool ID003Protocol::getAnswer(QByteArray &aAnswerData) {
    aAnswerData.clear();
    QByteArray answer;
    uchar length = 0;

    QElapsedTimer clockTimer;
    clockTimer.restart();

    do {
        answer.clear();

        if (!m_Port->read(answer, 20)) {
            return false;
        }

        aAnswerData.append(answer);

        if (aAnswerData.size() > 1) {
            length = aAnswerData[1];
        }
    } while ((clockTimer.elapsed() < CID003::AnswerTimeout) &&
             ((aAnswerData.size() < length) || (length == 0U)));

    toLog(LogLevel::Normal, QString("ID003: << {%1}").arg(aAnswerData.toHex().data()));

    return true;
}

//--------------------------------------------------------------------------------
bool ID003Protocol::sendACK() {
    QByteArray commandData(1, CID003::ACK);
    pack(commandData);

    toLog(LogLevel::Normal, QString("ID003: >> {%1} - ACK").arg(commandData.toHex().data()));

    return m_Port->write(commandData);
}

//--------------------------------------------------------------------------------
