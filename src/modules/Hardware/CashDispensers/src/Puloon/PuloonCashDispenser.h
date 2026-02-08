/* @file Диспенсер купюр Puloon. */

#pragma once

#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/Dispensers/PortDispenser.h"
#include "Hardware/Protocols/Dispensers/Puloon.h"

//--------------------------------------------------------------------------------
class PuloonLCDM : public PortDispenser {
    SET_SERIES("Puloon")

public:
    PuloonLCDM();

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList();

protected:
    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Сброс.
    virtual bool reset();

    /// Выдать.
    virtual void perform_Dispense(int aUnit, int aItems);

    /// Выполнить команду.
    TResult processCommand(char aCommand, QByteArray *aAnswer = nullptr);
    TResult processCommand(char aCommand,
                           const QByteArray &aCommandData,
                           QByteArray *aAnswer = nullptr,
                           int aTimeout = 0);

    /// Вывод в лог изменения статуса.
    void logStatusChanging(const SDeviceCodeSpecification &aStatusSpecification,
                           const TDeviceCodeSpecifications &aSensorSpecifications);

    /// Протокол.
    Puloon m_Protocol;
};

//--------------------------------------------------------------------------------
