/* @file ФР СПАРК. */

#pragma once

#include "Hardware/FR/PortFRBase.h"
#include "Hardware/Protocols/FR/FiscalChequeStates.h"
#include "Hardware/Protocols/FR/SparkFR.h"
#include "SparkFRConstants.h"
#include "SparkModelData.h"

//--------------------------------------------------------------------------------
class SparkFR : public TSerialFRBase {
    SET_SERIES("SPARK")

public:
    SparkFR();

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList();

protected:
    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Проверить параметры налогов.
    typedef QList<QByteArray> TTaxData;
    bool checkTaxFlags(const TTaxData &aTaxData);

    /// Проверить параметры налогов во флагах.
    typedef QList<QByteArray> TTaxData;
    bool checkSystem_Flags(QByteArray &aFlagData);

    /// Проверить установки системного флага.
    bool checkSystem_Flag(const QByteArray &aFlagBuffer, int aNumber);

    /// Получить системные флаги.
    bool getSystem_Flags(QByteArray &aData, TTaxData *aTaxes = nullptr);

    /// Получить дату и время ФР.
    virtual QDateTime getDateTime();

    /// Напечатать строку.
    virtual bool printLine(const QByteArray &aString);

    /// Применить теги.
    virtual void execTags(Tags::SLexeme &aTagLexeme, QVariant &aLine);

    /// Печать фискального чека.
    virtual bool perform_Fiscal(const QStringList &aReceipt,
                               const SDK::Driver::SPaymentData &aPaymentData,
                               quint32 *aFDNumber = nullptr);

    /// Печать Z отчета.
    virtual bool perform_ZReport(bool aPrintDeferredReports);

    /// Локальная печать X-отчета.
    virtual bool processXReport();

    /// Печать выплаты.
    virtual bool processPayout(double aAmount);

    /// Получить сумму в кассе.
    virtual double getAmountInCash();

    /// Отрезка.
    virtual bool cut();

    /// Забрать чек в ретрактор.
    virtual bool retract();

    /// Выполнить команду.
    virtual TResult execCommand(const QByteArray &aCommand,
                                const QByteArray &aCommandData,
                                QByteArray *aAnswer = nullptr);

    /// Получить состояние Z-буффера.
    void getZBufferState();

    /// Аварийно завершить фискальный(?) документ в случае ошибки фискальной части.
    bool cancelDocument(bool aDocumentIsOpened);

    /// Запросить и вывести в лог критичные параметры ФР.
    void processDeviceData();

    /// Получить данные о ККМ.
    typedef QList<QByteArray> TKKMInfoData;
    bool getKKMData(TKKMInfoData &aData);

    /// Получить дату и время из запроса данных о ККМ.
    QDateTime parseDateTime(TKKMInfoData &aData);

    /// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
    bool processAnswer(char aError);

    /// Выполнить Z-отчет.
    virtual bool execZReport(bool aAuto);

    /// Получить состояние смены.
    virtual SDK::Driver::ESessionState::Enum getSessionState();

    /// Получить состояние документа.
    virtual SDK::Driver::EDocumentState::Enum getDocumentState();

    /// Получить номер последнего фискального документа.
    int getLastDocumentNumber();

    /// Адаптивный ли способ формирования фискального чека.
    bool isAdaptiveFCCreation();

    /// Установить фискальные реквизиты ПФД.
    bool setFiscalParameters(const QStringList &aReceipt);

    /// Внесение/выплата.
    bool payIO(double aAmount, bool aIn);

    /// Продажа.
    bool sale(const SDK::Driver::SUnitData &aUnitData);

    /// Извлечь данные из специфичного BCD-формата.
    template <class T> T from_BCD(const QByteArray &aData);
    char from_BCD(char aData);

    /// Подождать готовность эжектора.
    bool waitEjectorReady();

    /// Подождать окончание печати следующего документа.
    bool waitNextPrinting();

    /// Протокол.
    SparkFRProtocol m_Protocol;

    /// Список поддерживаемых плагином моделей.
    QStringList m_SupportedModels;

    /// Последняя ошибка.
    char m_DocumentState;

    /// Системные флаги.
    CSparkFR::System_Flags::Data m_System_Flags;

    /// Дата и время начала смены.
    QDateTime m_SessionOpeningDT;

    /// Количество Z-отчетов в буфере.
    int m_ZReports;

    /// Можно ли проверять статус в нештатных ситуациях при выполнении команды.
    bool m_CheckStatus;

    /// Налоги.
    typedef QList<SDK::Driver::TVAT> TTaxes;
    TTaxes m_Taxes;
};

//--------------------------------------------------------------------------------
