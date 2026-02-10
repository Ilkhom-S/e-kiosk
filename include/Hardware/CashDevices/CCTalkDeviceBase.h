/* @file Базовый класс устройства на протоколе ccTalk. */

#pragma once

#include <QtCore/QSharedPointer>

#include "CCTalkModelData.h"
#include "Hardware/CashDevices/CCTalkDeviceConstants.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Protocols/CashAcceptor/CCTalk.h"

//--------------------------------------------------------------------------------
template <class T> class CCTalkDeviceBase : public T {
    SET_SERIES("ccTalk")

public:
    CCTalkDeviceBase();

    /// Получить поддерживаемые тпы протоколов.
    static QStringList getProtocolTypes();

protected:
    /// Запросить и сохранить параметры устройства.
    virtual void processDeviceData();

    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Попытка самоидентификации.
    bool checkConnection();

    /// Распарсить данные прошивки.
    virtual double parseFWVersion(const QByteArray &aAnswer);

    /// Выполнить команду.
    virtual TResult execCommand(const QByteArray &aCommand,
                                const QByteArray &aCommandData,
                                QByteArray *aAnswer = nullptr);

    /// Распарсить дату.
    QDate parseDate(const QByteArray &aData);

    /// Данные всех моделей.
    typedef QSharedPointer<CCCTalk::CModelDataBase> PAllModelData;
    PAllModelData m_AllModelData;

    /// Базовый год (для парсинга дат).
    int m_BaseYear{};

    /// Протокол.
    CCTalkCAProtocol m_Protocol;

    /// Индекс события.
    int m_EventIndex;

    // TODO: в базу.
    /// Последние девайс-коды устройства.
    TDeviceCodes m_Codes;

    /// Номер прошивки.
    double m_FWVersion;

    /// Модели данной реализации.
    QStringList m_Models;

    /// Данные модели.
    CCCTalk::SModelData m_ModelData;

    /// Адрес устройства.
    uchar m_Address;

    /// Поддерживаемые тпы протоколов.
    QStringList m_ProtocolTypes;

    /// Данные ошибок.
    typedef QSharedPointer<CCCTalk::ErrorDataBase> PErrorData;
    PErrorData m_ErrorData;
};

//--------------------------------------------------------------------------------
