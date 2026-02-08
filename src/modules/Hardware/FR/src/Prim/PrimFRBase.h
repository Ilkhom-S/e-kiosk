/* @file Принтеры семейства ПРИМ. */

#pragma once

#include <Hardware/FR/Prim_FR.h>

#include "Hardware/FR/PortFRBase.h"
#include "Prim_FRConstants.h"
#include "Prim_ModelData.h"

class Prim_SeriesType {};

//--------------------------------------------------------------------------------
class Prim_FRBase : public TSerialFRBase {
    SET_SERIES("PRIM")
    typedef Prim_SeriesType TSeriesType;

public:
    Prim_FRBase();

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList();

protected:
    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Напечатать [и выдать] чек.
    virtual bool processReceipt(const QStringList &aReceipt, bool aProcessing = true);

    /// Напечатать [и выдать] чек.
    virtual bool perform_Receipt(const QStringList &aReceipt, bool aProcessing = true);

    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Получить дату и время ФР.
    virtual QDateTime getDateTime();

    /// Напечатать строку.
    virtual bool printLine(const QByteArray &aString);

    /// Печать фискального чека.
    virtual bool perform_Fiscal(const QStringList &aReceipt,
                               const SDK::Driver::SPaymentData &aPaymentData,
                               quint32 *aFDNumber = nullptr);

    /// Преобразование нефискальной квитанции для ПФД.
    void makeAFDReceipt(QStringList &aReceipt);

    /// Заполнить фискальные данные для ПФД.
    virtual void setFiscalData(CPrim_FR::TData &aCommandData,
                               CPrim_FR::TDataList &aAdditionalAFDData,
                               const SDK::Driver::SPaymentData &aPaymentData,
                               int aReceiptSize);

    /// Печать Z отчета.
    virtual bool perform_ZReport(bool aPrintDeferredReports);

    /// Печать Z отчета.
    virtual bool execZReport(bool aAuto);

    /// Выполнить Z-отчет.
    virtual TResult doZReport(bool aAuto);

    /// Открыть смену.
    virtual bool openSession();

    /// Локальная печать X-отчета.
    virtual bool processXReport();

    /// Печать выплаты.
    virtual bool processPayout(double aAmount);

    /// Получить сумму в кассе.
    virtual double getAmountInCash();

    /// Получить инфо о ресурсах и статусе.
    bool getStatusInfo(TStatusCodes &aStatusCodes, CPrim_FR::TData &aAnswer);

    /// Получить проверочный код последнего фискального документа - номер КПК.
    virtual int getVerificationCode();

    /// Установка режима работы принтер/ФР.
    bool setMode(EFRMode::Enum aMode);

    /// Проверить параметры налога.
    virtual bool checkTax(SDK::Driver::TVAT aVAT, CFR::Taxes::SData &aData);

    /// Получить параметры налога.
    bool getTaxData(int aGroup, CPrim_FR::Taxes::SData &aData);

    /// Установить параметры налога.
    bool setTaxData(int aGroup, const CPrim_FR::Taxes::SData &aData);

    /// Установка начального номера в буфере Z-отчетов.
    bool setStartZReportNumber(int aNumber, const CPrim_FR::TData &aExtraData);

    /// Получение начального номера в буфере Z-отчетов.
    int getStartZReportNumber(CPrim_FR::TData &aExtraData);

    /// Получение конечного номера в буфере Z-отчетов.
    int getEndZReportNumber();

    /// Печать отложенного Z-отчета.
    bool printDeferredZReport(int aNumber);

    /// Выполнить команду.
    TResult processCommand(char aCommand, CPrim_FR::TData *aAnswer = nullptr);
    TResult processCommand(char aCommand,
                           const CPrim_FR::TData &aCommandData,
                           CPrim_FR::TData *aAnswer = nullptr);

    template <class T>
    TResult processCommand(char aCommand, int aIndex, const QString &aLog, T &aResult);
    template <class T>
    TResult processCommand(char aCommand,
                           const CPrim_FR::TData &aCommandData,
                           int aIndex,
                           const QString &aLog,
                           T &aResult);

    /// Распарсить данные ответа.
    template <class T>
    bool parseAnswerData(const CPrim_FR::TData &aData, int aIndex, const QString &aLog, T &aResult);

    /// Загрузить данные устройства.
    template <class T>
    void loadDeviceData(const CPrim_FR::TData &aData,
                        const QString &aName,
                        const QString &aLog,
                        int aIndex,
                        const QString &aExtensibleName = "");

    /// Проверить ответ.
    TResult checkAnswer(TResult aResult, const QByteArray &aAnswer, CPrim_FR::TData &aAnswerData);

    /// Добавить обязательное G-поле ПФД в данные команды.
    CPrim_FR::TData addGFieldToBuffer(int aX, int aY, int aFont = CPrim_FR::FiscalFont::Default);

    /// Сформировать необязательное G-поле ПФД
    CPrim_FR::TData addArbitraryFieldToBuffer(int aX,
                                             int aY,
                                             const QString &aData,
                                             int aFont = CPrim_FR::FiscalFont::Default);

    /// Обработка ответа предыдущей команды. Автоисправление некоторых ошибок.
    virtual bool processAnswer(char aError);

    /// Проверить параметры ФР.
    bool checkParameters();

    /// Получить параметр 3 ФР.
    virtual ushort getParameter3();

    /// Проверить настройки ФР.
    bool checkControlSettings();

    /// Получить реал-тайм статусы принтера.
    void getRTStatuses(TStatusCodes &aStatusCodes);

    /// Получить реал-тайм статусы принтера по реал-тайм коду.
    TStatusCodes getRTStatus(int aCommand);

    /// Распарсить реал-тайм статусы принтера по реал-тайм коду.
    TStatusCodes parseRTStatus(int aCommand, char aAnswer);

    /// Получить состояние смены.
    virtual SDK::Driver::ESessionState::Enum getSessionState();

    /// Получить состояние документа.
    virtual SDK::Driver::EDocumentState::Enum getDocumentState();

    /// Получить ASCII-представление 1-байтного целочисленного числа.
    inline QString int2String(int aValue);
    inline QByteArray int2ByteArray(int aValue);

    /// Режим работы.
    EFRMode::Enum m_Mode;

    /// Модели данной реализации.
    CPrim_FR::TModels m_Models;

    /// Id модели.
    CPrim_FR::Models::Enum m_Model;

    /// Признак нахождения ФР в оффлайне из-за ошибки принтера, фискальная подсистема не отвечает на
    /// запросы.
    bool m_Offline;

    /// Протокол реал-тайм запросов.
    Prim_FRRealTimeProtocol m_RTProtocol;

    /// Протокол.
    Prim_FRProtocol m_Protocol;

    /// Ошибки.
    typedef QSharedPointer<CPrim_FR::Errors::ExtraDataBase> PExtraErrorData;
    PExtraErrorData m_ExtraErrorData;

    /// Таймауты.
    CPrim_FR::CommandTimouts m_CommandTimouts;
};

//--------------------------------------------------------------------------------
