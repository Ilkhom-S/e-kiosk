/* @file Сканер на порту. */

#pragma once

#include <Hardware/Common/BaseStatusTypes.h>
#include <Hardware/HID/HIDBase.h>

//--------------------------------------------------------------------------------
namespace CScanner {
/// Интервал опроса порта, [мс].
const int PollingInterval = 500;

/// Таймаут проверки пришедшего ответа в порту, [мс].
const int CheckingTimeout = 100;
} // namespace CScanner

//--------------------------------------------------------------------------------
template <class T> class PortScanner : public HIDBase<T> {
public:
    PortScanner();

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова
    /// initialize().
    virtual bool release();

protected:
    /// Интервал опроса порта.
    int m_PollingInterval;

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Получить данные
    virtual bool getData(QByteArray &aAnswer);
};

//--------------------------------------------------------------------------------