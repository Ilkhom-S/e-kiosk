/* @file Кросс-устройство управления питанием Штрих 3.0. */

#pragma once

#include <Hardware/Watchdogs/ShtrihConstants.h>
#include <Hardware/Watchdogs/WatchdogBase.h>

//--------------------------------------------------------------------------------
class Shtrih : public WatchdogBase {
    SET_SERIES("Shtrih")

public:
    Shtrih();

    /// Перезагрузка линии питания.
    virtual bool reset(const QString &aLine);

protected:
    /// Обработчик сигнала пинга.
    virtual void onPing();

    /// Идентифицирует устройство.
    virtual bool isConnected();

    bool processCommand(int aCommandID);

    bool localProcessCommand(int aCommand,
                             CShtrih::SUnpackedData &aUnpackedData,
                             const QByteArray &aCommandData = "");

    bool getCommandPacket(int aCommand, QByteArray &aCommandPacket, const QByteArray &aCommandData);

    bool parseAnswer(const CShtrih::TAnswersBuffer &aAnswersBuffer,
                     CShtrih::SUnpackedData *aUnpackedData);

    bool perform_Command(const QByteArray &aPacket, QByteArray &aAnswer);

    void packedData(const QByteArray &aCommandPacket, QByteArray &aPacket);

    bool unpackData(const QByteArray &aPacket,
                    const QByteArray &aAnswer,
                    CShtrih::TAnswersBuffer &aUnpackedBuffer) const;

    /// Подсчет контрольной суммы пакета данных.
    uchar calcCRC(const QByteArray &aData) const;

    /// Команда широковещательная?
    bool isBroadcastCommand(const QByteArray &aPacket) const;

    /// Знаем ли такой адрес устройства?
    bool isDeviceAddressExist(const QByteArray &aPacket) const;

    /// Получить строку с параметрами устройства по его типу.
    void setDeviceDataByType(CShtrih::Devices::Type::Enum aType);

private:
    /// Данные устройств на шине RS-485.
    CShtrih::SDevicesData m_DeviceData;

    // TODO: найти, где его инкрементить
    /// Номер посылки.
    uchar m_MessageNumber;

    /// Логика поддержки питания.
    bool m_PowerControlLogicEnable;

    /// Расширенная логика анализа датчиков питания.
    bool m_AdvancedPowerLogicEnable;

    /// Местный мьютекс.
    QMutex m_WaitMutex;
};

//--------------------------------------------------------------------------------