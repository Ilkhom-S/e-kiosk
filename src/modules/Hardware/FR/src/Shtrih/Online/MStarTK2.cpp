/* @file ФР Multisoft MStar-TK2 на протоколе Штрих. */

#include "MStarTK2.h"

//--------------------------------------------------------------------------------
MStarTK2FR::MStarTK2FR() {
    m_DeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::MStarTK2].name;
    m_OFDFiscalFields << CFR::FiscalFields::Cashier;
    m_PrinterStatusEnabled = false;
    m_NeedReceiptProcessingOnCancel = false;

    setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, false);
    setConfigParameter(CHardware::FR::CanZReportWithoutPrinting, true);

    m_SupportedModels = getModelList();

    // ошибки
    m_UnprocessedErrorData.add(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData,
                               CShtrihOnlineFR::Errors::NoRequiedDataInFS);
}

//--------------------------------------------------------------------------------
QStringList MStarTK2FR::getModelList() {
    using namespace CShtrihFR::Models;

    return CData().getModelList(ID::MStarTK2);
}

//--------------------------------------------------------------------------------
void MStarTK2FR::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    QVariantMap configuration(aConfiguration);
    configuration.insert(CHardware::Printer::Settings::LeftReceiptTimeout,
                         CMStarTK2FR::LeftReceiptTimeout);

    TMStarTK2FR::setDeviceConfiguration(configuration);
}

//--------------------------------------------------------------------------------
TResult MStarTK2FR::execCommand(const QByteArray &aCommand,
                                const QByteArray &aCommandData,
                                QByteArray *aAnswer) {
    QVariantMap configuration;
    configuration.insert(CHardware::Port::COM::WaitResult, true);
    m_IOPort->setDeviceConfiguration(configuration);

    return TMStarTK2FR::execCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
bool MStarTK2FR::setNotPrintDocument(bool aEnabled, bool aZReport) {
    using namespace CMStarTK2FR::Printing;

    QByteArray data;
    char value = !aEnabled ? All : (aZReport ? NoZReport : NoFiscal);

    if (getFRParameter(CShtrihOnlineFR::FRParameters::NotPrintDocument, data) && !data.isEmpty() &&
        (data[0] == value)) {
        return true;
    }

    SleepHelper::msleep(CShtrihOnlineFR::NotPrintDocumentPause);
    bool result = setFRParameter(CShtrihOnlineFR::FRParameters::NotPrintDocument, value);
    SleepHelper::msleep(CShtrihOnlineFR::NotPrintDocumentPause);

    return result;
}

//--------------------------------------------------------------------------------
