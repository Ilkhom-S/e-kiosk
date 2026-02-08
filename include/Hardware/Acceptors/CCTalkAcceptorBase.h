/* @file Базовый акцептор CCTalk. */

#pragma once

#include <Hardware/CashDevices/CCTalkDeviceBase.h>

#include "CCTalkCurrencyData.h"
#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"

//--------------------------------------------------------------------------------
template <class T> class CCTalkAcceptorBase : public CCTalkDeviceBase<T> {
public:
    CCTalkAcceptorBase();

protected:
    /// Завершение инициализации.
    virtual void finalizeInitialization();

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Получить буферизованные статусы.
    virtual bool getBufferedStatuses(QByteArray & /*aAnswer*/) { return false; }

    /// Распарсить буферизованные статусы.
    virtual void parseBufferedStatuses(const QByteArray &aAnswer, TStatusCodes &aStatusCodes);

    /// Распарсить данные о купюре.
    virtual void
    parseCreditData(uchar /*aCredit*/, uchar /*aError*/, TStatusCodes & /*aStatusCodes*/) {}

    /// Применить таблицу номиналов.
    virtual bool applyParTable();

    /// Изменение режима приема денег.
    virtual bool enableMoneyAcceptingMode(bool aEnabled);

    /// Получить данные для применения таблицы номиналов.
    QByteArray getParTableData();

    /// Получить данные валюты.
    virtual bool parseCurrencyData(const QByteArray &aData, CCCTalk::SCurrencyData &aCurrencyData);

    /// Можно ли применить простые статус-код (включен/отключен).
    virtual bool canApplySimpleStatusCodes(const TStatusCodes &aStatusCodes);

    /// Признак включенности на прием денег.
    bool m_Enabled;

    /// Валюта. Нужна для установки предупреждения о старой прошивке.
    int m_Currency;
};

//--------------------------------------------------------------------------------
