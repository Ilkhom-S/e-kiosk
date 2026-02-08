/* @file ФР семейства Штрих на порту. */

#include "ShtrihFRBase.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/qmath.h>

using namespace SDK::Driver;

// TODO: реализовать тег Bold командой Печать жирной строки (12h)

//--------------------------------------------------------------------------------
template class ShtrihFRBase<ShtrihSerialFRBase>;
template class ShtrihFRBase<ShtrihTCPFRBase>;

//--------------------------------------------------------------------------------
template <class T> ShtrihFRBase<T>::ShtrihFRBase() {
    // параметры семейства ФР
    m_LineFeed = false;
    setConfigParameter(CHardware::Printer::FeedingAmount, 6);

    // данные команд
    using namespace CShtrihFR::Commands;

    m_CommandData.add(GetLongStatus, 6 * 1000);
    m_CommandData.add(ExtentionPrinting, 6 * 1000);
    m_CommandData.add(CancelDocument, 6 * 1000);
    m_CommandData.add(ZReport, 30 * 1000);
    m_CommandData.add(ZReportInBuffer, 30 * 1000);
    m_CommandData.add(PrintDeferredZReports, 6 * 1000);
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::updateParameters() {
    if (!ProtoShtrihFR<T>::updateParameters()) {
        return false;
    }

    getZReportQuantity();

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::isConnected() {
    EPortTypes::Enum portType = m_IOPort->getType();

    if (portType == EPortTypes::COMEmulator) {
        toLog(LogLevel::Error, m_DeviceName + ": Port type is COM-emulator");
        return false;
    }

    QByteArray answer;
    TResult result = processCommand(CShtrihFR::Commands::GetModelInfo, &answer);

    if (!CORRECT(result)) {
        return false;
    }

    m_Type = CShtrihFR::Types::NoType;
    m_Model = CShtrihFR::Models::ID::NoModel;
    CShtrihFR::Models::CData modeData;

    bool isLoading = !isAutoDetecting();
    QString modelName = getConfigParameter(CHardwareSDK::ModelName).toString();
    auto modelDataIt = std::find_if(
        modeData.data().begin(),
        modeData.data().end(),
        [&modelName](const CShtrihFR::SModelData &data) -> bool { return data.name == modelName; });

    if (result && (answer.size() > 6)) {
        m_Type = uchar(answer[2]);
        m_Model = uchar(answer[6]);
    } else if (!result && isLoading && !modelName.isEmpty() &&
               (modelDataIt != modeData.data().end())) {
        m_Type = CShtrihFR::Types::KKM;
        m_Model = modelDataIt.key();
    }

    answer = answer.mid(7).replace(ASCII::NUL, "");

    QString modelId = m_Codec->toUnicode(answer);
    modelDataIt = std::find_if(modeData.data().begin(),
                               modeData.data().end(),
                               [&modelName, &modelId](const CShtrihFR::SModelData &data) -> bool {
                                   return !modelId.isEmpty() && !data.id.isEmpty() &&
                                          modelId.contains(data.id, Qt::CaseInsensitive);
                               });

    if (modelDataIt != modeData.data().end()) {
        m_Type = CShtrihFR::Types::KKM;
        m_Model = modelDataIt.key();
    }

    m_Verified = false;
    m_DeviceName = CShtrihFR::Models::Default;
    m_ModelData = modeData[m_Model];
    m_CanProcessZBuffer = m_ModelData.ZBufferSize;

    if (m_Type == CShtrihFR::Types::KKM) {
        m_Verified = m_ModelData.verified;
        m_DeviceName = m_ModelData.name;
        setConfigParameter(CHardware::Printer::FeedingAmount, m_ModelData.feed);
    } else if ((m_Type == CShtrihFR::Types::Printer) &&
               (m_Model == CShtrihFR::Models::ID::Shtrih500)) {
        m_DeviceName = "Shtrih-M Shtrih-500";
        m_FontNumber = CShtrihFR::Fonts::Shtrih500;
    }

    if (m_DeviceName == CShtrihFR::Models::Default) {
        toLog(LogLevel::Error,
              QString("ShtrihFR: Unknown model number = %1 or type = %2").arg(m_Model).arg(m_Type));
    }

    m_ModelCompatibility = m_SupportedModels.contains(m_DeviceName);

    if (CShtrihFR::FRParameters::Fields.data().contains(m_Model)) {
        m_Parameters = CShtrihFR::FRParameters::Fields[m_Model];
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::getStatus(TStatusCodes &aStatusCodes) {
    if (!ProtoShtrihFR<T>::getStatus(aStatusCodes)) {
        return false;
    }

    if (canGetZReportQuantity() && !m_WhiteSpaceZBuffer) {
        aStatusCodes.insert(FRStatusCode::Warning::ZBufferFull);
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::performZReport(bool aPrintDeferredReports) {
    toLog(LogLevel::Normal, "ShtrihFR: Begin processing Z-report");
    bool printDeferredZReportsOK = true;

    bool ZBufferOverflow = m_ZBufferOverflow;
    bool printZReportOK = execZReport(false);

    if (printZReportOK && ZBufferOverflow) {
        m_ZBufferFull = true;
    }

    if (aPrintDeferredReports && m_CanProcessZBuffer) {
        printDeferredZReportsOK = printDeferredZReports();
        getZReportQuantity();

        if (printDeferredZReportsOK || (m_WhiteSpaceZBuffer > 0)) {
            m_ZBufferFull = false;
        }
    }

    return (printDeferredZReportsOK && aPrintDeferredReports) || printZReportOK;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::printDeferredZReports() {
    toLog(LogLevel::Normal, "ShtrihFR: Begin printing deferred Z-reports");

    bool printDeferredZReportSuccess = processCommand(CShtrihFR::Commands::PrintDeferredZReports);
    SleepHelper::msleep(CShtrihFR::Pause::ZReportPrintingEnd);

    if (!printDeferredZReportSuccess) {
        toLog(LogLevel::Error, "ShtrihFR: Failed to print deferred Z-reports");
        return false;
    }

    SleepHelper::msleep(CShtrihFR::Pause::ZReportPrintingEnd);
    waitForPrintingEnd();

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::prepareZReport(bool aAuto, QVariantMap &aData) {
    bool needCloseSession = m_Mode == CShtrihFR::InnerModes::SessionExpired;

    if (aAuto) {
        if (m_OperatorPresence) {
            toLog(LogLevel::Error,
                  m_DeviceName +
                      ": Failed to process auto-Z-report due to presence of the operator.");
            m_NeedCloseSession = m_NeedCloseSession || needCloseSession;

            return false;
        }

        if (!m_IsOnline && !m_CanProcessZBuffer) {
            toLog(LogLevel::Normal,
                  m_DeviceName + ": FR has no buffer, auto-Z-report is not available");
            m_NeedCloseSession = m_NeedCloseSession || needCloseSession;

            return false;
        }
    }

    return ProtoShtrihFR<T>::prepareZReport(aAuto, aData);
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::execZReport(bool aAuto) {
    QVariantMap outData;

    if (!prepareZReport(aAuto, outData)) {
        return false;
    }

    char command = aAuto ? CShtrihFR::Commands::ZReportInBuffer : CShtrihFR::Commands::ZReport;
    m_NeedCloseSession = false;
    bool success = processCommand(command);

    if (success) {
        m_ZBufferOverflow = false;
        SleepHelper::msleep(CShtrihFR::Pause::ZReportPrintingEnd);
        success = waitForChangeZReportMode();
    }

    if (getLongStatus()) {
        m_NeedCloseSession = m_Mode == CShtrihFR::InnerModes::SessionExpired;
    }

    if (success) {
        emit FRSessionClosed(outData);
    }

    if (command == CShtrihFR::Commands::ZReportInBuffer) {
        getZReportQuantity();
    }

    toLog(success ? LogLevel::Normal : LogLevel::Error,
          success ? "ShtrihFR: Z-report is successfully processed"
                  : "ShtrihFR: error in processing Z-report");

    return success;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::canGetZReportQuantity() {
    return m_Fiscalized && (m_Model == CShtrihFR::Models::ID::ShtrihComboFRK) &&
           isFiscalReady(false, EFiscalPrinterCommand::ZReport);
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::getZReportQuantity() {
    if (!canGetZReportQuantity()) {
        return false;
    }

    QByteArray ZReportQuantity;

    if (!getRegister(CShtrihFR::Registers::ZReportsQuantity, ZReportQuantity)) {
        toLog(LogLevel::Error, "ShtrihFR: Failed to get Z-report quantity");
        return false;
    }

    int count = ZReportQuantity.toInt();
    toLog(LogLevel::Error, "ShtrihFR: Z-report count = " + QString::number(count));

    m_WhiteSpaceZBuffer = m_ModelData.ZBufferSize - count;

    if (m_WhiteSpaceZBuffer < 0) {
        m_WhiteSpaceZBuffer = 0;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::waitForChangeZReportMode() {
    QElapsedTimer clockTimer;
    clockTimer.start();

    TStatusCodes errorStatusCodes = getErrorFRStatusCodes();

    do {
        QDateTime clock = QDateTime::currentDateTime();

        // 3.1. запрашиваем статус
        TStatusCodes statusCodes;

        if (getStatus(statusCodes)) {
            // 3.2. анализируем режим и подрежим, если печать Z-отчета окончена - выходим
            if (!statusCodes.intersect(errorStatusCodes).isEmpty()) {
                toLog(LogLevel::Error, "ShtrihFR: Failed to print Z-report, exit!");
                return false;
            }

            if ((m_Submode == CShtrihFR::InnerSubmodes::PaperEndPassive) ||
                (m_Submode == CShtrihFR::InnerSubmodes::PaperEndActive)) {
                // 3.3. подрежим - закончилась бумага
                return false;
            }
            // если режим или подрежим - печать или печать отчета или Z-отчета, то
            else if ((m_Mode == CShtrihFR::InnerModes::DataEjecting) ||
                     (m_Mode == CShtrihFR::InnerModes::PrintFullZReport) ||
                     (m_Mode == CShtrihFR::InnerModes::PrintEKLZReport) ||
                     (m_Submode == CShtrihFR::InnerSubmodes::PrintingFullReports) ||
                     (m_Submode == CShtrihFR::InnerSubmodes::Printing)) {
                toLog(LogLevel::Normal, "ShtrihFR: service Z-report process, wait...");
            } else if ((m_Mode == CShtrihFR::InnerModes::SessionClosed) ||
                       (m_Submode == CShtrihFR::InnerSubmodes::PaperOn)) {
                // 3.3. режим - тот, который ожидаем, если Z-отчет допечатался, все хорошо
                return true;
            } else {
                // 3.4. режим не тот, который ожидаем в соответствии с протоколом, выходим с ошибкой
                toLog(LogLevel::Error,
                      QString("ShtrihFR: Z-report, unknown mode.submode = %1.%2")
                          .arg(int(m_Mode))
                          .arg(int(m_Submode)));
                return false;
            }

            // спим до периода опроса
            int sleepTime =
                CShtrihFR::Interval::ReportPoll - abs(clock.time().msecsTo(QTime::currentTime()));

            if (sleepTime > 0) {
                SleepHelper::msleep(sleepTime);
            }
        }
    } while (clockTimer.elapsed() < CShtrihFR::Timeouts::MaxZReportNoAnswer);

    toLog(LogLevel::Normal, "ShtrihFR: Timeout for Z-report.");

    // вышли по таймауту, значит, не смогли дождаться нужного режима/подрежима
    return false;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihFRBase<T>::processAnswer(const QByteArray &aCommand, char aError) {
    if (ProtoShtrihFR<T>::processAnswer(aCommand, aError)) {
        return true;
    }

    if (aError == CShtrihFR::Errors::ChequeBufferOverflow) {
        m_ZBufferOverflow = m_CanProcessZBuffer;
    }

    return false;
}

//--------------------------------------------------------------------------------
