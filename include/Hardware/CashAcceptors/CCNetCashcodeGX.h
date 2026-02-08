/* @file Купюроприемник Cashcode GX на протоколе CCNet. */

#pragma once

#include <Hardware/CashAcceptors/CCNetCashAcceptorBase.h>
#include <cmath>

//--------------------------------------------------------------------------------
namespace CCCNetCashcodeGX {
/// Пауза после резета, [мс].
const int ResetPause = 15 * 1000;

/// Выход из initilaize-а.
const int ExitInitializeTimeout = 20 * 1000;
} // namespace CCCNetCashcodeGX

//--------------------------------------------------------------------------------
class CCNetCashcodeGX : public CCNetCashAcceptorBase {
    SET_SUBSERIES("CashcodeGX")

public:
    CCNetCashcodeGX();

protected:
    /// Проверка возможности выполнения функционала, предполагающего связь с устройством.
    virtual bool checkConnectionAbility();

    /// Выполнить команду.
    virtual TResult perform_Command(const QByteArray &aCommand,
                                   const QByteArray &aCommandData,
                                   QByteArray *aAnswer = nullptr);

    /// Локальный сброс.
    virtual bool processReset();

    /// Отправить буфер данных обновления прошивки для купюроприемника Cashcode GX.
    virtual bool processUpdating(const QByteArray &aBuffer, int aSectionSize);

    /// Изменить скорость работы.
    virtual bool perform_BaudRateChanging(const SDK::Driver::TPortParameters &aPortParameters);
};

//--------------------------------------------------------------------------------