/* @file Базовый ФР с портовой реализацией протокола. */

#pragma once

#include <Hardware/FR/FRBase.h>
#include <Hardware/FR/ProtoFR.h>
#include <Hardware/Printers/PortPrintersBase.h>

//--------------------------------------------------------------------------------
template <class T> class PortFRBase : public FRBase<T> {
public:
    PortFRBase();

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);

    /// Печать фискального чека.
    virtual bool printFiscal(const QStringList &aReceipt,
                             const SDK::Driver::SPaymentData &aPaymentData,
                             quint32 *aFDNumber = nullptr);

    /// Получить фискальные теги по номеру документа.
    virtual bool checkFiscalFields(quint32 aFDNumber,
                                   SDK::Driver::TFiscalPaymentData &aFPData,
                                   SDK::Driver::TComplexFiscalPaymentData &aPSData);

    /// Выполнить Z-отчет [и распечатать отложенные Z-отчеты].
    virtual bool printZReport(bool aPrintDeferredReports);

    /// Выполнить X-отчет [и распечатать нефискальный чек - баланс].
    virtual bool printXReport(const QStringList &aReceipt);

protected:
    /// Установить начальные параметры.
    virtual void setInitialData();

    /// Идентификация.
    virtual bool checkExistence();

    /// Фоновая логика при появлении определенных состояний устройства.
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection);

    /// Получить статус. Возвращает Fail, Error (константы) или правильный ответ.
    template <class T2>
    QByteArray performStatus(TStatusCodes &aStatusCodes, T2 aCommand, int aIndex = -1);

    /// Сформировать массив байтов для печаит из массива строк.
    typedef QList<QByteArray> TReceiptBuffer;
    void makeReceipt(const QStringList &aReceipt, QStringList &aBuffer);
    void makeReceipt(const QStringList &aReceipt, TReceiptBuffer &aBuffer);

    typedef std::function<TResult(QByteArray &aData)> TGetFiscalTLVData;
    typedef std::function<bool(const CFR::STLV &aTLV)> TProcessTLVAction;
    bool processFiscalTLVData(const TGetFiscalTLVData &aGetFiscalTLVData,
                              SDK::Driver::TFiscalPaymentData *aFPData,
                              SDK::Driver::TComplexFiscalPaymentData *aPSData);
    bool processTLVData(const TGetFiscalTLVData &aGetFiscalTLVData,
                        TProcessTLVAction aAction = TProcessTLVAction());

    /// Загрузить названия отделов.
    typedef std::function<bool(int aIndex, QByteArray &aValue)> TLoadSectionName;
    bool loadSectionNames(const TLoadSectionName &aLoadSectionName);

    /// Является ли ошибка необрабатываемой?
    bool isErrorUnprocessed(char aCommand, char aError);
    virtual bool isErrorUnprocessed(const QByteArray &aCommand, char aError);

    /// Буфер обрабатываемых ошибок.
    class ErrorBuffer : public QList<char> {
    public:
        void removeLast() {
            if (!isEmpty())
                QList::removeLast();
        }
        void pop_back() {
            if (!isEmpty())
                QList::pop_back();
        }
    };

    ErrorBuffer m_ProcessingErrors;

    /// Команда последнего запроса.
    QByteArray m_LastCommand;

    /// Ошибка на последний запрос.
    char m_LastError;

    /// Результат последней выполненной протокольной команды.
    TResult m_LastCommandResult;

    /// Данные ошибок.
    typedef QSharedPointer<FRError::Data> PErrorData;
    PErrorData m_ErrorData;

    /// Данные необрабатываемых ошибок.
    typedef QSet<char> TErrors;

    class UnprocessedErrorData : public CSpecification<QByteArray, TErrors> {
    public:
        void add(char aCommand, char aError) { data()[QByteArray(1, aCommand)].insert(aError); }
        void add(const QByteArray &aCommand, char aError) { data()[aCommand].insert(aError); }
        void add(char aCommand, const TErrors &aErrors) {
            append(QByteArray(1, aCommand), aErrors);
        }
        void add(const QByteArray &aCommand, const TErrors &aErrors) { append(aCommand, aErrors); }
    };

    UnprocessedErrorData m_UnprocessedErrorData;
};

typedef PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>
    TSerialFRBase;
typedef PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>
    TTCPFRBase;

//--------------------------------------------------------------------------------
