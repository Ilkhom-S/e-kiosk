/* @file Базовый ФР с портовой реализацией протокола. */

#include "PortFRBase.h"

//--------------------------------------------------------------------------------
template class PortFRBase<
    SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;
template class PortFRBase<
    PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;

template QByteArray
PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>::
    perform_Status(TStatusCodes &, char, int);
template QByteArray
PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>::
    perform_Status(TStatusCodes &, const char *, int);
template QByteArray
PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>::
    perform_Status(TStatusCodes &, char, int);
template QByteArray
PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>::
    perform_Status(TStatusCodes &, const char *, int);

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
template <class T>
PortFRBase<T>::PortFRBase() : m_LastError('\x00'), m_LastCommandResult(CommandResult::OK) {
    setInitialData();

    // ошибки
    m_ErrorData = PErrorData(new FRError::Data());
}

//--------------------------------------------------------------------------------
template <class T> void PortFRBase<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    bool opened = m_IOPort && m_IOPort->opened();

    FRBase<T>::setDeviceConfiguration(aConfiguration);

    if (m_OperatorPresence && m_IOPort && !opened && m_IOPort->opened()) {
        m_IOPort->close();
    }
}

//--------------------------------------------------------------------------------
template <class T> void PortFRBase<T>::setInitialData() {
    FRBase<T>::setInitialData();

    m_ProcessingErrors.clear();
    m_IOMessageLogging = ELoggingType::None;
}

//---------------------------------------------------------------------------
template <class T> bool PortFRBase<T>::checkExistence() {
    m_ProcessingErrors.clear();

    return FRBase<T>::checkExistence();
}

//---------------------------------------------------------------------------
template <class T> bool PortFRBase<T>::loadSectionNames(const TLoadSectionName &aLoadSectionName) {
    if (!aLoadSectionName) {
        toLog(LogLevel::Error,
              m_DeviceName + ": Failed to get section names due to no functional logic");
    }

    TSectionNames sectionNames;
    QByteArray data;
    int group = 0;

    while (aLoadSectionName(++group, data)) {
        sectionNames.insert(group, m_Codec->toUnicode(data.replace(ASCII::NUL, "").simplified()));
    }

    LogLevel::Enum logLevel = sectionNames.isEmpty() ? LogLevel::Error : LogLevel::Warning;
    toLog(logLevel, m_DeviceName + QString(": Failed to get name for %1 section").arg(group));

    if (!m_LastCommandResult || !m_LastError || !isErrorUnprocessed(m_LastCommand, m_LastError)) {
        return false;
    }

    if (sectionNames.isEmpty()) {
        toLog(LogLevel::Error,
              m_DeviceName + ": Failed to get section names due to they are not exist");
        return false;
    }

    m_ProcessingErrors.pop_back();
    m_LastError = 0;

    setConfigParameter(CHardwareSDK::FR::SectionNames,
                       QVariant::from_Value<TSectionNames>(sectionNames));

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
template <class T2>
QByteArray PortFRBase<T>::perform_Status(TStatusCodes &aStatusCodes, T2 aCommand, int aIndex) {
    QByteArray data;
    TResult result = processCommand(aCommand, &data);

    if (result == CommandResult::Device) {
        int statusCode = getErrorStatusCode(m_ErrorData->value(m_LastError).type);
        aStatusCodes.insert(statusCode);
    } else if ((result == CommandResult::Answer) || (data.size() <= aIndex)) {
        aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
    } else if (result) {
        return data;
    }

    return !CORRECT(result) ? CFR::Result::Fail : CFR::Result::Error;
}

//--------------------------------------------------------------------------------
template <class T>
void PortFRBase<T>::makeReceipt(const QStringList &aReceipt, QStringList &aBuffer) {
    Tags::TLexemeReceipt lexemeReceipt;
    makeLexemeReceipt(aReceipt, lexemeReceipt);

    foreach (auto collection, lexemeReceipt) {
        foreach (auto buffer, collection) {
            QString line;

            foreach (auto tagLexeme, buffer) {
                if (!tagLexeme.tags.contains(Tags::Type::Image)) {
                    QString data = tagLexeme.data;

                    if (tagLexeme.tags.contains(Tags::Type::BarCode)) {
                        data = "Barcode <" + data + ">";
                    }

                    line += data;
                }
            }

            aBuffer << line;
        }
    }
}

//--------------------------------------------------------------------------------
template <class T>
void PortFRBase<T>::makeReceipt(const QStringList &aReceipt, TReceiptBuffer &aBuffer) {
    QStringList buffer;
    makeReceipt(aReceipt, buffer);

    foreach (const QString &line, buffer) {
        aBuffer << m_Codec->from_Unicode(line);
    }
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::printFiscal(const QStringList &aReceipt,
                                const SPaymentData &aPaymentData,
                                quint32 *aFDNumber) {
    bool result = FRBase<T>::printFiscal(aReceipt, aPaymentData, aFDNumber);

    if (m_OperatorPresence) {
        m_IOPort->close();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::checkFiscalFields(quint32 aFDNumber,
                                      TFiscalPaymentData &aFPData,
                                      TComplexFiscalPaymentData &aPSData) {
    bool result = FRBase<T>::checkFiscalFields(aFDNumber, aFPData, aPSData);

    if (m_OperatorPresence) {
        m_IOPort->close();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::processFiscalTLVData(const TGetFiscalTLVData &aGetFiscalTLVData,
                                         TFiscalPaymentData *aFPData,
                                         TComplexFiscalPaymentData *aPSData) {
    TProcessTLVAction fiscalTLVAction = [&](const CFR::STLV aTLV) -> bool {
        if (aPSData)
            m_FFEngine.parseSTLVData(aTLV, *aPSData);
        if (aFPData)
            m_FFEngine.parseTLVData(aTLV, *aFPData);

        return true;
    };

    return processTLVData(aGetFiscalTLVData, fiscalTLVAction);
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::processTLVData(const TGetFiscalTLVData &aGetFiscalTLVData,
                                   TProcessTLVAction aAction) {
    TResult commandResult;
    bool result = true;

    do {
        QByteArray data;
        CFR::STLV TLV;
        commandResult = aGetFiscalTLVData(data);

        if ((commandResult == CommandResult::Device) &&
            isErrorUnprocessed(m_LastCommand, m_LastError)) {
            m_ProcessingErrors.pop_back();
            m_LastError = 0;

            return result;
        } else if (!commandResult) {
            return false;
        } else if (data.isEmpty()) {
            toLog(LogLevel::Warning, m_DeviceName + ": Failed to add fiscal field due to no data");

            continue;
        } else if (!m_FFEngine.parseTLV(data, TLV)) {
            result = false;
        } else if (!m_FFData.data().contains(TLV.field)) {
            toLog(
                LogLevel::Warning,
                m_DeviceName +
                    QString(": Failed to add fiscal field %1 due to it is unknown").arg(TLV.field));
        } else if (aAction && !aAction(TLV)) {
            result = false;
        }
    } while (commandResult);

    return false;
}

//--------------------------------------------------------------------------------
template <class T> bool PortFRBase<T>::printZReport(bool aPrintDeferredReports) {
    bool result = FRBase<T>::printZReport(aPrintDeferredReports);

    if (m_OperatorPresence) {
        m_IOPort->close();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool PortFRBase<T>::printXReport(const QStringList &aReceipt) {
    bool result = FRBase<T>::printXReport(aReceipt);

    if (m_OperatorPresence) {
        m_IOPort->close();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T>
void PortFRBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                      const TStatusCollection &aOldStatusCollection) {
    bool opened = m_IOPort->opened();

    FRBase<T>::postPollingAction(aNewStatusCollection, aOldStatusCollection);

    if (m_OperatorPresence && !opened && m_IOPort->opened()) {
        m_IOPort->close();
    }
}

//--------------------------------------------------------------------------------
template <class T> bool PortFRBase<T>::isErrorUnprocessed(char aCommand, char aError) {
    return m_UnprocessedErrorData.data().value(QByteArray(1, aCommand)).contains(aError);
}

//--------------------------------------------------------------------------------
template <class T> bool PortFRBase<T>::isErrorUnprocessed(const QByteArray &aCommand, char aError) {
    return m_UnprocessedErrorData.data().value(aCommand).contains(aError);
}

//--------------------------------------------------------------------------------
