/* @file Монетоприемник на протоколе NPSTalk. */

#pragma once

#include "Hardware/CoinAcceptors/CoinAcceptorBase.h"
#include "Hardware/Protocols/CashAcceptor/NPSTalk.h"

//--------------------------------------------------------------------------------
class NPSTalkCoinAcceptor : public CoinAcceptorBase {
    SET_SERIES("NPS")

    typedef QMap<uchar, uchar> TCoinsByChannel;

public:
    NPSTalkCoinAcceptor();

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList();

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

protected:
    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Локальный сброс.
    virtual bool processReset();

    /// Установка параметров по умолчанию.
    virtual bool setDefaultParameters();

    /// Загрузка таблицы номиналов из устройства.
    virtual bool loadParTable();

    /// Изменение режима приема денег.
    virtual bool enableMoneyAcceptingMode(bool aEnabled);

    /// Выполнить команду.
    virtual TResult execCommand(const QByteArray &aCommand,
                                const QByteArray &aCommandData,
                                QByteArray *aAnswer = nullptr);

    /// Протокол.
    NPSTalkProtocol m_Protocol;

    // TODO: в базу.
    /// Последние статус-коды устройства.
    TDeviceCodes m_Codes;

    /// Количество монет на канал.
    TCoinsByChannel m_CoinsByChannel;
};

//--------------------------------------------------------------------------------
