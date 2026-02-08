/* @file Базовый ФР на протоколе АТОЛ2. */

#pragma once

#include <Hardware/FR/Atol2FR.h>

#include "AtolFRBase.h"

//--------------------------------------------------------------------------------
class Atol2FRBase : public AtolFRBase {
    SET_SERIES("ATOL2")

protected:
    /// Выполнить команду.
    virtual TResult
    perform_Command(const QByteArray &aCommandData, QByteArray &aAnswer, int aTimeout) {
        m_Protocol.setPort(m_IOPort);
        m_Protocol.setLog(m_Log);

        return m_Protocol.processCommand(aCommandData, aAnswer, aTimeout);
    }

    /// Протокол.
    Atol2FRProtocol m_Protocol;
};

//--------------------------------------------------------------------------------
