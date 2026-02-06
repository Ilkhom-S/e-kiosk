/* @file POS-принтеры  с эжектором. */

#include "EjectorPOS.h"

#include <QtCore/QtGlobal>

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
template class EjectorPOS<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template <class T> EjectorPOS<T>::EjectorPOS() {
    // данные устройства
    this->mDeviceName = "POS Printer with ejector";

    this->setConfigParameter(CHardware::Printer::PresenterEnable, true);
    this->setConfigParameter(CHardware::Printer::RetractorEnable, true);

    this->setConfigParameter(CHardware::Printer::Commands::Presentation,
                             CPOSPrinter::Command::Present);
    this->setConfigParameter(CHardware::Printer::Commands::Pushing, CPOSPrinter::Command::Push);
    this->setConfigParameter(CHardware::Printer::Commands::Retraction,
                             CPOSPrinter::Command::Retract);
}

//--------------------------------------------------------------------------------
template <class T> void EjectorPOS<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    this->setDeviceConfiguration(aConfiguration);

    if (this->containsConfigParameter(CHardware::Printer::Settings::PresentationLength)) {
        int presentationLength = qMax(
            CEjectorPOS::MinPresentationLength,
            this->getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt());
        this->setConfigParameter(CHardware::Printer::Settings::PresentationLength,
                                 presentationLength);
    }
}

//--------------------------------------------------------------------------------
template <class T> bool EjectorPOS<T>::updateParameters() {
    if (!this->updateParameters()) {
        return false;
    }

    QString loop = this->getConfigParameter(CHardware::Printer::Settings::Loop).toString();

    if (loop == CHardwareSDK::Values::Auto) {
        return true;
    }

    return this->mIOPort->write((loop == CHardwareSDK::Values::Use)
                                    ? CPOSPrinter::Command::LoopEnable
                                    : CPOSPrinter::Command::LoopDisable);
}

//--------------------------------------------------------------------------------
