/* @file Базовый ФР ПРИМ c эжектором. */

#include "Prim_EjectorFR.h"

#include "../Prim_ModelData.h"
#include "Hardware/Printers/Custom_VKP80.h"
#include "Hardware/Printers/POSPrinterData.h"
#include "Prim_EjectorFRConstants.h"

//--------------------------------------------------------------------------------
template class Prim_EjectorFR<Prim_FRBase>;
template class Prim_EjectorFR<Prim_OnlineFRBase>;

//--------------------------------------------------------------------------------
template <class T> Prim_EjectorFR<T>::Prim_EjectorFR() {
    // данные устройства
    setConfigParameter(CHardware::Printer::RetractorEnable, true);

    m_OldBuildNumber = false;
    m_Printer = PPrinter(new SerialCustom_VKP80());
    m_DeviceName = CPrim_FR::ModelData[CPrim_FR::Models::PRIM_21K_03].name;
    m_LPC22RetractorErrorCount = 0;
}

//--------------------------------------------------------------------------------
template <class T> bool Prim_EjectorFR<T>::updateParameters() {
    return Prim_PresenterFR<T>::updateParameters() && setPresentationMode() &&
           checkPresentationLength();
}

//--------------------------------------------------------------------------------
template <class T> ushort Prim_EjectorFR<T>::getParameter3() {
    using namespace CHardware::Printer;

    if (getConfigParameter(Settings::NotTakenReceipt).toString() != Values::Retract) {
        return 0;
    }

    return ushort(getConfigParameter(Settings::LeftReceiptTimeout).toInt());
}

//--------------------------------------------------------------------------------
template <class T> bool Prim_EjectorFR<T>::checkPresentationLength() {
    if (!Prim_FRBase::checkParameters()) {
        return false;
    }

    CPrim_FR::TData data;

    if (processCommand(CPrim_FR::Commands::GetMoneyBoxSettings, &data) && (data.size() >= 8)) {
        bool OK;
        int presentationLength =
            getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt();
        int FRPresentationLength = data[5].toInt(&OK, 16);

        if (!OK || (presentationLength == FRPresentationLength)) {
            return OK;
        }

        data = data.mid(5);
        data[0] = int2ByteArray(presentationLength);

        return processCommand(CPrim_FR::Commands::SetMoneyBoxSettings, data);
    }

    return false;
}

//---------------------------------------------------------------------------
template <class T> void Prim_EjectorFR<T>::cleanStatusCodes(TStatusCodes &aStatusCodes) {
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
template <class T> bool Prim_EjectorFR<T>::processAnswer(char aError) {
    if (((aError == CPrim_FR::Errors::IncorrigibleError) ||
         (aError == CPrim_FR::Errors::NotReadyForPrint)) &&
        (m_LPC22RetractorErrorCount < CPrim_FR::MaxRepeat::RetractorError)) {
        m_ProcessingErrors.push_back(aError);

        m_LPC22RetractorErrorCount++;

        toLog(LogLevel::Normal, "Abnormal error, try to reset printer");

        return processEjectorAction(CPrim_EjectorFRActions::Reset);
    }

    if (aError && (m_LPC22RetractorErrorCount >= CPrim_FR::MaxRepeat::RetractorError) &&
        (aError != CPrim_FR::Errors::IncorrigibleError) &&
        (aError != CPrim_FR::Errors::NotReadyForPrint)) {
        toLog(LogLevel::Normal, "Reset Abnormal error count");
        m_LPC22RetractorErrorCount = 0;
    }

    return Prim_PresenterFR<T>::processAnswer(aError);
}

//--------------------------------------------------------------------------------
template <class T> bool Prim_EjectorFR<T>::push() {
    if (m_Mode == EFRMode::Printer) {
        return TSerialFRBase::push();
    }

    return processEjectorAction(CPrim_EjectorFRActions::Push);
}

//--------------------------------------------------------------------------------
template <class T> bool Prim_EjectorFR<T>::retract() {
    if (m_Mode == EFRMode::Printer) {
        return TSerialFRBase::retract();
    }

    return processEjectorAction(CPrim_EjectorFRActions::Retract);
}

//--------------------------------------------------------------------------------
template <class T> bool Prim_EjectorFR<T>::setPresentationMode() {
    QString loop = getConfigParameter(CHardware::Printer::Settings::Loop).toString();

    if (loop == CHardwareSDK::Values::Auto) {
        return true;
    }

    bool loopEnable = loop == CHardwareSDK::Values::Use;

    if (m_Mode == EFRMode::Printer) {
        return m_IOPort->write(loopEnable ? CPOSPrinter::Command::LoopEnable
                                         : CPOSPrinter::Command::LoopDisable);
    }

    return processEjectorAction(loopEnable ? CPrim_EjectorFRActions::SetLoopEnabled
                                           : CPrim_EjectorFRActions::SetLoopDisabled);
}

//--------------------------------------------------------------------------------
template <class T> bool Prim_EjectorFR<T>::present() {
    return (m_Mode == EFRMode::Printer) && TSerialFRBase::present();
}

//--------------------------------------------------------------------------------
template <class T> bool Prim_EjectorFR<T>::processEjectorAction(const QString &aAction) {
    CPrim_FR::TData commandData;

    for (int i = 0; i < aAction.size(); ++i) {
        commandData << int2ByteArray(QString(aAction[i]).toInt(0, 16));
    }

    return processCommand(CPrim_FR::Commands::SetEjectorAction, commandData);
}

//--------------------------------------------------------------------------------
