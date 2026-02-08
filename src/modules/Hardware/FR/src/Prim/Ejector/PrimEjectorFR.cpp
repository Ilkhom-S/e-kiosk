/* @file Базовый ФР ПРИМ c эжектором. */

#include "PrimEjectorFR.h"

#include "../PrimModelData.h"
#include "Hardware/Printers/CustomVKP80.h"
#include "Hardware/Printers/POSPrinterData.h"
#include "PrimEjectorFRConstants.h"

//--------------------------------------------------------------------------------
template class PrimEjectorFR<PrimFRBase>;
template class PrimEjectorFR<PrimOnlineFRBase>;

//--------------------------------------------------------------------------------
template <class T> PrimEjectorFR<T>::PrimEjectorFR() {
    // данные устройства
    setConfigParameter(CHardware::Printer::RetractorEnable, true);

    m_OldBuildNumber = false;
    m_Printer = PPrinter(new SerialCustomVKP80());
    m_DeviceName = CPrimFR::ModelData[CPrimFR::Models::PRIM_21K_03].name;
    m_LPC22RetractorErrorCount = 0;
}

//--------------------------------------------------------------------------------
template <class T> bool PrimEjectorFR<T>::updateParameters() {
    return PrimPresenterFR<T>::updateParameters() && setPresentationMode() &&
           checkPresentationLength();
}

//--------------------------------------------------------------------------------
template <class T> ushort PrimEjectorFR<T>::getParameter3() {
    using namespace CHardware::Printer;

    if (getConfigParameter(Settings::NotTakenReceipt).toString() != Values::Retract) {
        return 0;
    }

    return ushort(getConfigParameter(Settings::LeftReceiptTimeout).toInt());
}

//--------------------------------------------------------------------------------
template <class T> bool PrimEjectorFR<T>::checkPresentationLength() {
    if (!PrimFRBase::checkParameters()) {
        return false;
    }

    CPrimFR::TData data;

    if (processCommand(CPrimFR::Commands::GetMoneyBoxSettings, &data) && (data.size() >= 8)) {
        bool OK;
        int presentationLength =
            getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt();
        int FRPresentationLength = data[5].toInt(&OK, 16);

        if (!OK || (presentationLength == FRPresentationLength)) {
            return OK;
        }

        data = data.mid(5);
        data[0] = int2ByteArray(presentationLength);

        return processCommand(CPrimFR::Commands::SetMoneyBoxSettings, data);
    }

    return false;
}

//---------------------------------------------------------------------------
template <class T> void PrimEjectorFR<T>::cleanStatusCodes(TStatusCodes &aStatusCodes) {
    if (m_OldBuildNumber) {
        aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
    }

    if (aStatusCodes.contains(PrinterStatusCode::Warning::PaperEndVirtual)) {
        aStatusCodes.remove(PrinterStatusCode::Warning::PaperEndVirtual);
        aStatusCodes.insert(PrinterStatusCode::Warning::PaperNearEnd);
    }

    TSerialFRBase::cleanStatusCodes(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T> bool PrimEjectorFR<T>::processAnswer(char aError) {
    if (((aError == CPrimFR::Errors::IncorrigibleError) ||
         (aError == CPrimFR::Errors::NotReadyForPrint)) &&
        (m_LPC22RetractorErrorCount < CPrimFR::MaxRepeat::RetractorError)) {
        m_ProcessingErrors.push_back(aError);

        m_LPC22RetractorErrorCount++;

        toLog(LogLevel::Normal, "Abnormal error, try to reset printer");

        return processEjectorAction(CPrimEjectorFRActions::Reset);
    }

    if (aError && (m_LPC22RetractorErrorCount >= CPrimFR::MaxRepeat::RetractorError) &&
        (aError != CPrimFR::Errors::IncorrigibleError) &&
        (aError != CPrimFR::Errors::NotReadyForPrint)) {
        toLog(LogLevel::Normal, "Reset Abnormal error count");
        m_LPC22RetractorErrorCount = 0;
    }

    return PrimPresenterFR<T>::processAnswer(aError);
}

//--------------------------------------------------------------------------------
template <class T> bool PrimEjectorFR<T>::push() {
    if (m_Mode == EFRMode::Printer) {
        return TSerialFRBase::push();
    }

    return processEjectorAction(CPrimEjectorFRActions::Push);
}

//--------------------------------------------------------------------------------
template <class T> bool PrimEjectorFR<T>::retract() {
    if (m_Mode == EFRMode::Printer) {
        return TSerialFRBase::retract();
    }

    return processEjectorAction(CPrimEjectorFRActions::Retract);
}

//--------------------------------------------------------------------------------
template <class T> bool PrimEjectorFR<T>::setPresentationMode() {
    QString loop = getConfigParameter(CHardware::Printer::Settings::Loop).toString();

    if (loop == CHardwareSDK::Values::Auto) {
        return true;
    }

    bool loopEnable = loop == CHardwareSDK::Values::Use;

    if (m_Mode == EFRMode::Printer) {
        return m_IOPort->write(loopEnable ? CPOSPrinter::Command::LoopEnable
                                         : CPOSPrinter::Command::LoopDisable);
    }

    return processEjectorAction(loopEnable ? CPrimEjectorFRActions::SetLoopEnabled
                                           : CPrimEjectorFRActions::SetLoopDisabled);
}

//--------------------------------------------------------------------------------
template <class T> bool PrimEjectorFR<T>::present() {
    return (m_Mode == EFRMode::Printer) && TSerialFRBase::present();
}

//--------------------------------------------------------------------------------
template <class T> bool PrimEjectorFR<T>::processEjectorAction(const QString &aAction) {
    CPrimFR::TData commandData;

    for (int i = 0; i < aAction.size(); ++i) {
        commandData << int2ByteArray(QString(aAction[i]).toInt(0, 16));
    }

    return processCommand(CPrimFR::Commands::SetEjectorAction, commandData);
}

//--------------------------------------------------------------------------------
