/* @file Протокол ФР ПРИМ. */

#include <numeric>

#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/LoggingType.h"
#include "Prim_FR.h"
#include "Prim_FRConstants.h"

using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
Prim_FRProtocol::Prim_FRProtocol() : m_Differential(ASCII::Space), m_LastCommandResult(0) {}

//--------------------------------------------------------------------------------
ushort Prim_FRProtocol::calcCRC(const QByteArray &aData) {
    return ushort(
        std::accumulate(aData.begin(), aData.end(), 0, [&](ushort arg1, char arg2) -> ushort {
            return arg1 + uchar(arg2);
        }));
}

//--------------------------------------------------------------------------------
TResult
Prim_FRProtocol::check(const QByteArray &aRequest, const QByteArray &aAnswer, bool aPrinterMode) {
    if (aAnswer.size() < CPrim_FR::MinAnswerSize) {
        toLog(LogLevel::Error,
              QString("PRIM: The length of the packet is less than %1 byte")
                  .arg(CPrim_FR::MinAnswerSize));
        return CommandResult::Protocol;
    }

    if (aAnswer[0] != CPrim_FR::Prefix) {
        toLog(LogLevel::Error,
              QString("PRIM: Invalid prefix = %1, need = %2")
                  .arg(toHexLog(aAnswer[0]))
                  .arg(toHexLog(CPrim_FR::Prefix)));
        return CommandResult::Protocol;
    }

    char postfix = aAnswer.right(5)[0];

    if (postfix != CPrim_FR::Postfix) {
        toLog(LogLevel::Error,
              QString("PRIM: Invalid postfix = %1, need = %2")
                  .arg(toHexLog(postfix))
                  .arg(toHexLog(CPrim_FR::Postfix)));
        return CommandResult::Protocol;
    }

    if ((aAnswer[1] != char(m_Differential)) && !aPrinterMode) {
        toLog(LogLevel::Error,
              QString("PRIM: Invalid differential = %1, need = %2")
                  .arg(toHexLog(aAnswer[1]))
                  .arg(toHexLog(m_Differential)));
        return CommandResult::Id;
    }

    QString requestCommand = aRequest.mid(6, 2);
    QString answerCommand = aAnswer.mid(2, 2);

    if ((requestCommand != answerCommand) && !aPrinterMode) {
        toLog(LogLevel::Error,
              QString("PRIM: Invalid command in answer = \"%1\", need = \"%2\"")
                  .arg(answerCommand)
                  .arg(requestCommand));
        return CommandResult::Id;
    }

    QByteArray unpackedData = aAnswer.left(aAnswer.size() - 4);
    ushort localCRC = calcCRC(unpackedData);
    bool isOK;
    ushort CRC = qToBigEndian(aAnswer.right(4).toUShort(&isOK, 16));

    if (!isOK) {
        toLog(LogLevel::Error,
              QString("PRIM: Filed to convert CRC = %1 from hex to digit")
                  .arg(aAnswer.right(4).data()));
        return CommandResult::CRC;
    } else if (localCRC != CRC) {
        toLog(LogLevel::Error,
              QString("PRIM: Invalid CRC = %1 (%2), need %3 (%4)")
                  .arg(CRC)
                  .arg(toHexLog(CRC))
                  .arg(localCRC)
                  .arg(toHexLog(localCRC)));
        return CommandResult::CRC;
    }

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
TResult
Prim_FRProtocol::processCommand(const QByteArray &aCommandData, QByteArray &aAnswer, int aTimeout) {
    QByteArray request;

    request.append(CPrim_FR::Prefix);
    request.append(CPrim_FR::Password);

    m_Differential = (m_Differential == uchar(ASCII::Full)) ? ASCII::Space : ++m_Differential;

    request.append(m_Differential);
    request.append(aCommandData);

    request.append(CPrim_FR::Separator);
    request.append(CPrim_FR::Postfix);
    request.append(QString("%1")
                       .arg(qToBigEndian(calcCRC(request)), 4, 16, QChar(ASCII::Zero))
                       .toUpper()
                       .toLatin1());

    TResult result = execCommand(request, aAnswer, aTimeout);

    if (result != CommandResult::NoAnswer) {
        return result;
    }

    int index = 0;

    do {
        if (index) {
            toLog(LogLevel::Normal, "PRIM: Command still working, sleep");
            SleepHelper::msleep(CPrim_FR::CommandInProgressPause);
        }

        char answer;
        result = getCommandResult(answer, true);

        if (!result) {
            return result;
        }

        if (~answer & CPrim_FR::CommandResultMask::PrinterError) {
            toLog(LogLevel::Normal, "PRIM: Printer error");
        }

        if (~answer & CPrim_FR::CommandResultMask::PrinterMode) {
            toLog(LogLevel::Normal, "PRIM: Printer mode");
            aAnswer = QByteArray(1, answer);

            return CommandResult::NoAnswer;
        }

        if (~answer & CPrim_FR::CommandResultMask::CommandVerified) {
            toLog(LogLevel::Normal, "PRIM: Command is not verified");
            return CommandResult::Transport;
        }

        if (answer & CPrim_FR::CommandResultMask::CommandComplete) {
            toLog(LogLevel::Normal, "PRIM: Command done, trying to read");
            return execCommand(
                request, aAnswer, CPrim_FR::DefaultTimeout, EPrim_FRCommandConditions::NAKRepeat);
        }
    } while (++index < CPrim_FR::RepeatingCount::CommandInProgress);

    return CommandResult::Transport;
}

//--------------------------------------------------------------------------------
TResult Prim_FRProtocol::getCommandResult(char &aAnswer, bool aOnline) {
    if (!aOnline && m_LastCommandResult) {
        aAnswer = m_LastCommandResult;

        return CommandResult::OK;
    }

    m_RTProtocol.setPort(m_Port);
    m_RTProtocol.setLog(getLog());

    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging,
                         QVariant().from_Value(ELoggingType::ReadWrite));
    m_Port->setDeviceConfiguration(configuration);

    TResult result = m_RTProtocol.processCommand(0, aAnswer);
    m_LastCommandResult = result ? aAnswer : 0;

    configuration.insert(CHardware::Port::IOLogging, QVariant().from_Value(ELoggingType::None));
    m_Port->setDeviceConfiguration(configuration);

    return result;
}

//--------------------------------------------------------------------------------
TResult Prim_FRProtocol::execCommand(const QByteArray &aRequest,
                                     QByteArray &aAnswer,
                                     int aTimeout,
                                     EPrim_FRCommandConditions::Enum aConditions) {
    int index = 0;
    QByteArray request(aRequest);
    TResult result = CommandResult::OK;

    do {
        if (index || (aConditions == EPrim_FRCommandConditions::NAKRepeat)) {
            request = QByteArray(1, ASCII::NAK);
        }

        toLog(LogLevel::Normal, QString("PRIM: >> {%1}").arg(request.toHex().data()));
        aAnswer.clear();

        if (!m_Port->write(request) || !readData(aAnswer, aTimeout)) {
            return CommandResult::Port;
        }

        if (aAnswer.isEmpty()) {
            return CommandResult::NoAnswer;
        }

        result = check(aRequest, aAnswer, aConditions == EPrim_FRCommandConditions::PrinterMode);

        if (result) {
            aAnswer = aAnswer.mid(2, aAnswer.size() - 8);

            return CommandResult::OK;
        }
    } while (++index < CPrim_FR::RepeatingCount::Protocol);

    return result;
}

//--------------------------------------------------------------------------------
bool Prim_FRProtocol::readData(QByteArray &aData, int aTimeout) {
    QTime clockTimer;
    clockTimer.start();

    do {
        QByteArray data;

        if (!m_Port->read(data)) {
            return false;
        }

        aData.append(data);
    } while (((aData.size() < CPrim_FR::MinAnswerSize) ||
              !aData.right(6).startsWith(CPrim_FR::AnswerEndMark)) &&
             (clockTimer.elapsed() < aTimeout));

    int index = aData.lastIndexOf(CPrim_FR::Postfix);

    if (index != -1) {
        aData = aData.left(index + 5);
    }

    toLog(LogLevel::Normal, QString("PRIM: << {%1}").arg(aData.toHex().data()));

    return true;
}

//--------------------------------------------------------------------------------
