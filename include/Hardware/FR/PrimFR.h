/* @file Протокол ФР ПРИМ. */

#pragma once

#include <Hardware/Common/ProtocolBase.h>
#include <Hardware/FR/Prim_FRRealTime.h>

/// Условия выполнения команд.
namespace EPrim_FRCommandConditions {
enum Enum {
    None = 0,    /// Нет дополнительных условий
    PrinterMode, /// Команда протокола выполняется из режима принтера
    NAKRepeat    /// Повторный запрос ответа с помощью NAK-а
};
} // namespace EPrim_FRCommandConditions

//--------------------------------------------------------------------------------
/// Класс протокола Prim_FR.
class Prim_FRProtocol : public ProtocolBase {
public:
    Prim_FRProtocol();

    /// Выполнить команду протокола.
    TResult processCommand(const QByteArray &aCommandData, QByteArray &aAnswer, int aTimeout);

    /// Выполнить команду протокола без запаковки ответа.
    TResult
    execCommand(const QByteArray &aRequest,
                QByteArray &aAnswer,
                int aTimeout,
                EPrim_FRCommandConditions::Enum aConditions = EPrim_FRCommandConditions::None);

    /// Получить результат выполнения последней команды.
    TResult getCommandResult(char &aAnswer, bool aOnline = false);

protected:
    /// Подсчет контрольной суммы пакета данных.
    ushort calcCRC(const QByteArray &aData);

    /// Распаковка пришедших из порта данных.
    TResult check(const QByteArray &aRequest, const QByteArray &aAnswer, bool aPrinterMode);

    /// Считываем данные из порта.
    bool readData(QByteArray &aData, int aTimeout);

    /// Отличительный байт.
    uchar m_Differential;

    /// Протокол реал-тайм запросов.
    Prim_FRRealTimeProtocol m_RTProtocol;

    /// Ответ на запрос выполнения последней команды.
    char m_LastCommandResult;
};

//--------------------------------------------------------------------------------