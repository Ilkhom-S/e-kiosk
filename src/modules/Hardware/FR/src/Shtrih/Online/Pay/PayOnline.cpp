/* @file ФР PayOnline-01-FA на протоколе Штрих. */

#include "PayOnline.h"

//--------------------------------------------------------------------------------
template class PayOnline<ShtrihOnlineFRBase<ShtrihTCPFRBase>>;
template class PayOnline<ShtrihOnlineFRBase<ShtrihSerialFRBase>>;

//--------------------------------------------------------------------------------
template <class T> PayOnline<T>::PayOnline() {
    m_DeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::PayOnline01FA].name;
    m_SupportedModels = QStringList() << m_DeviceName;

    setConfigParameter(CHardware::Printer::PresenterEnable, true);
    setConfigParameter(CHardware::Printer::RetractorEnable, true);
    setConfigParameter(CHardware::CanSoftReboot, true);
}

//--------------------------------------------------------------------------------
template <class T> void PayOnline<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    PayFRBase<T>::setDeviceConfiguration(aConfiguration);

    QString printerModel =
        getConfigParameter(CHardware::FR::PrinterModel, CPayPrinters::Default).toString();

    if (aConfiguration.contains(CHardware::FR::PrinterModel) &&
        (printerModel != CPayPrinters::Default) && !isNotPrinting()) {
        for (auto it = CPayPrinters::Models.data().begin(); it != CPayPrinters::Models.data().end();
             ++it) {
            uchar printerModelId = it.key();

            if ((it->name == printerModel) && (printerModelId != CPayPrinters::Virtual)) {
                m_PPTaskList.append([&]() { m_NotPrintingError = !setNotPrintDocument(false); });
            }
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> void PayOnline<T>::processDeviceData() {
    PayFRBase<T>::processDeviceData();

    QByteArray data;

    if (getFRParameter(CShtrihOnlineFR::FRParameters::PrinterModel, data)) {
        uchar printerModelId = uchar(data[0]);

        if (!CPayPrinters::Models.data().contains(printerModelId)) {
            toLog(LogLevel::Error,
                  QString("%1: Unknown printer model Id %2").arg(m_DeviceName).arg(printerModelId));
        } else {
            QString printerModel = CPayPrinters::Models[printerModelId].name;
            QString configModel = getConfigParameter(CHardware::FR::PrinterModel).toString();

            if ((configModel.isEmpty() || (configModel == CPayPrinters::Default)) &&
                (printerModel != configModel)) {
                m_PrinterModelId = printerModelId;
                setConfigParameter(CHardware::FR::PrinterModel, printerModel);

                emit configurationChanged();
            }
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> bool PayOnline<T>::setNotPrintDocument(bool aEnabled, bool /*aZReport*/) {
    if (aEnabled) {
        return PayFRBase<T>::setNotPrintDocument(aEnabled);
    }

    QString printerModel = getConfigParameter(CHardware::FR::PrinterModel).toString();
    auto it = std::find_if(
        CPayPrinters::Models.data().begin(),
        CPayPrinters::Models.data().end(),
        [&](const CPayPrinters::SModelData &aData) -> bool { return aData.name == printerModel; });

    if (it == CPayPrinters::Models.data().end()) {
        toLog(LogLevel::Error, m_DeviceName + ": No printer model " + printerModel);
    }

    uchar printerModelId = it.key();
    QByteArray data;
    bool result = getFRParameter(CShtrihOnlineFR::FRParameters::PrinterModel, data) &&
                  !data.isEmpty() && (uchar(data[0]) == printerModelId);

    if (!result) {
        result = setFRParameter(CShtrihOnlineFR::FRParameters::PrinterModel, printerModelId);
        reboot();
    }

    if (result) {
        m_PrinterModelId = printerModelId;
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> uchar PayOnline<T>::getLeftReceiptTimeout() {
    return 0;
}

//--------------------------------------------------------------------------------
template <class T> bool PayOnline<T>::push() {
    QByteArray commandData;
    commandData.append(CShtrihFR::PrintOnChequeTape);
    commandData.append(1); // количество строк промотки

    return processCommand(CShtrihFR::Commands::Feed, commandData);
}

//--------------------------------------------------------------------------------
