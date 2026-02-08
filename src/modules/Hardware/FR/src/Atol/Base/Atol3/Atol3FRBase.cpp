/* @file Базовый ФР на протоколе АТОЛ3. */

#include "Atol3FRBase.h"

#include "Atol3FRBaseConstants.h"

using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
Atol3FRBase::Atol3FRBase() : m_TId(0) {}

//--------------------------------------------------------------------------------
bool Atol3FRBase::isConnected() {
    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);
    m_Protocol.cancel();

    return AtolFRBase::isConnected();
}

//--------------------------------------------------------------------------------
TResult
Atol3FRBase::perform_Command(const QByteArray &aCommandData, QByteArray &aAnswer, int aTimeout) {
    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);

    QTime clockTimer;
    clockTimer.start();

    m_TId = (m_TId == uchar(CAtol3FR::LastTId)) ? ASCII::NUL : ++m_TId;
    TResult result = m_Protocol.processCommand(m_TId, aCommandData, aAnswer);

    using namespace CAtol3FR;

    if (aAnswer[0] == States::InProgress) {
        do {
            result = m_Protocol.waitForAnswer(aAnswer, CAtol3FR::Timeouts::WaitForAnswer);
        } while ((clockTimer.elapsed() < aTimeout) &&
                 ((result == CommandResult::NoAnswer) || (aAnswer[0] == States::InProgress)));
    }

    if (!result) {
        m_Protocol.cancel();

        return result;
    }

    auto answerResult = [&](const QString aLog) -> TResult {
        if (!aLog.isEmpty())
            toLog(LogLevel::Error, m_DeviceName + QString(": %1, aborting").arg(aLog));
        m_Protocol.cancel();
        return CommandResult::Answer;
    };

    if (aAnswer.isEmpty()) {
        toLog(LogLevel::Error, m_DeviceName + ": No task state, trying to get the result");

        if (!m_Protocol.getResult(m_TId, aAnswer) || aAnswer.isEmpty()) {
            return answerResult("No task state again");
        }
    }

    char state = aAnswer[0];
    aAnswer = aAnswer.mid(1);

    if ((state == States::AsyncResult) || (state == States::AsyncError)) {
        if (!aAnswer.isEmpty()) {
            uchar TId = uchar(aAnswer[0]);

            if (m_TId != TId) {
                return answerResult(QString("Invalid task Id = %1, need %2")
                                        .arg(toHexLog(TId))
                                        .arg(toHexLog(m_TId)));
            }

            aAnswer = aAnswer.mid(1);
        } else {
            toLog(LogLevel::Error, m_DeviceName + ": No task Id, trying to get the result");

            if (!m_Protocol.getResult(m_TId, aAnswer) || aAnswer.isEmpty()) {
                return answerResult("No task Id again");
            }
        }
    }

    if ((state == States::Result) || (state == States::AsyncResult)) {
        m_Protocol.sendACK(m_TId);

        return CommandResult::OK;
    } else if ((state == States::Error) || (state == States::AsyncError)) {
        m_Protocol.cancel();

        return CommandResult::OK;
    }

    return answerResult(QString("Task %1 state = %2").arg(toHexLog(m_TId)).arg(toHexLog(state)));
}

//--------------------------------------------------------------------------------
