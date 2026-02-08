/* @file Базовый ФР Pay на протоколе Штрих. */

#include "PayFRBase.h"

//--------------------------------------------------------------------------------
template class PayFRBase<ShtrihOnlineFRBase<ShtrihTCPFRBase>>;
template class PayFRBase<ShtrihOnlineFRBase<ShtrihSerialFRBase>>;

//--------------------------------------------------------------------------------
template <class T> PayFRBase<T>::PayFRBase() : m_PrinterModelId(0) {
    m_OFDFiscalFields << CFR::FiscalFields::Cashier;
}

//--------------------------------------------------------------------------------
template <class T> void PayFRBase<T>::processDeviceData() {
    ShtrihRetractorFRLite<T>::processDeviceData();

    QString SDCardData = getDeviceParameter(CDeviceData::SDCard).toString();
    bool SDCardError = SDCardData.isEmpty() || SDCardData.startsWith(CDeviceData::Error) ||
                       SDCardData.startsWith(CDeviceData::NotConnected);
    m_CanProcessZBuffer = m_CanProcessZBuffer && !SDCardError &&
                         (m_ModelData.date >= CShtrihOnlineFR::MinFWDate::ZBuffer);
}

//--------------------------------------------------------------------------------
template <class T> void PayFRBase<T>::appendStatusCodes(ushort aFlags, TStatusCodes &aStatusCodes) {
    ShtrihRetractorFRLite<T>::appendStatusCodes(aFlags, aStatusCodes);

    bool paperWeightSensor = (~aFlags & CShtrihFR::Statuses::WeightSensor::NoChequePaper);
    bool useRemotePaperSensor =
        getConfigParameter(CHardware::Printer::Settings::RemotePaperSensor).toString() ==
        CHardwareSDK::Values::Use;
    bool hasPNESensor = CPayPrinters::Models[m_PrinterModelId].hasPNESensor;

    if (paperWeightSensor && useRemotePaperSensor && hasPNESensor) {
        aStatusCodes.insert(PrinterStatusCode::Warning::PaperNearEnd);
        toLog(LogLevel::Warning, "ShtrihFR: Paper near end, report weight sensor");
    }
}

//--------------------------------------------------------------------------------
template <class T> bool PayFRBase<T>::execZReport(bool aAuto) {
    if (m_CanProcessZBuffer && ShtrihFRBase::execZReport(aAuto)) {
        return true;
    }

    return ShtrihOnlineFRBase::execZReport(aAuto);
}

//--------------------------------------------------------------------------------
template <class T> bool PayFRBase<T>::retract() {
    return processCommand(CShtrihFR::Commands::Cut, QByteArray(1, CShtrihFR::FullCutting));
}

//--------------------------------------------------------------------------------
