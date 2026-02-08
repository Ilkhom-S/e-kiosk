/* @file Базовый ФР на протоколе АТОЛ3. */

#pragma once

#include <Hardware/FR/Atol3FR.h>

#include "../AtolFRBase.h"

//--------------------------------------------------------------------------------
class Atol3FRBase : public AtolFRBase {
    SET_SERIES("ATOL3")

    Atol3FRBase();

protected:
    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Выполнить команду.
    virtual TResult
    perform_Command(const QByteArray &aCommandData, QByteArray &aAnswer, int aTimeout);

    /// Протокол.
    Atol3FRProtocol m_Protocol;

    /// Id задачи.
    uchar m_TId;
};

//--------------------------------------------------------------------------------
