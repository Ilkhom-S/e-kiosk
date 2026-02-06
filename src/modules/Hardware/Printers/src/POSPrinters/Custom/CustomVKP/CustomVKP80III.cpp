/* @file Принтер Custom VKP-80 III. */

#include "CustomVKP80III.h"

template class CustomVKP80III<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template <class T> CustomVKP80III<T>::CustomVKP80III() {
    // параметры моделей
    this->mDeviceName = "Custom VKP-80 III";
    this->mModelID = '\xFF';

    // модели
    this->mParameters = POSPrinters::CommonParameters;
    this->mModelData.data().clear();
    this->mModelData.add(this->mModelID, true, this->mDeviceName);

    this->setConfigParameter(CHardware::Printer::PresenterEnable, false);
    this->setConfigParameter(CHardware::Printer::RetractorEnable, false);
}

//--------------------------------------------------------------------------------
template <class T> bool CustomVKP80III<T>::getModelId(QByteArray &aAnswer) const {
    return this->mIOPort->write(CCustomVKP80III::Command::GetModelId) &&
           this->getAnswer(aAnswer, CPOSPrinter::Timeouts::Info) &&
           (aAnswer.isEmpty() || (aAnswer.size() == 2));
}

//--------------------------------------------------------------------------------
template <class T> bool CustomVKP80III<T>::updateParameters() {
    this->setConfigParameter(CHardware::Printer::Settings::Loop, CHardwareSDK::Values::Use);

    return this->updateParameters();
}

//--------------------------------------------------------------------------------
template <class T> char CustomVKP80III<T>::parseModelId(QByteArray &aAnswer) {
    if (aAnswer != CCustomVKP80III::ModelId) {
        this->toLog(LogLevel::Error,
                    QString("%1: Wrong answer {%2}, need {%3}")
                        .arg(this->mDeviceName)
                        .arg(aAnswer.toHex().data())
                        .arg(QByteArray(CCustomVKP80III::ModelId).toHex().data()));
        return 0;
    }

    return this->mModelID;
}

//--------------------------------------------------------------------------------
template <class T> bool CustomVKP80III<T>::receiptProcessing() {
    int presentationLength =
        this->getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt();
    QString ejectorActionParameter =
        this->getConfigParameter(CHardware::Printer::Settings::PreviousAndNotTakenReceipts)
            .toString();
    char ejectorAction = (ejectorActionParameter == CHardware::Printer::Values::Retract)
                             ? CCustomVKP80III::Retraction
                             : CCustomVKP80III::Pushing;
    int leftReceiptTimeout = 0;

    if (ejectorActionParameter != CHardwareSDK::Values::Auto) {
        leftReceiptTimeout =
            this->getConfigParameter(CHardware::Printer::Settings::LeftReceiptTimeout).toInt();
    }

    QByteArray command = CCustomVKP80III::Command::EjectorActivation;
    command.append(char(presentationLength));
    command.append(CCustomVKP80III::Blinking);
    command.append(ejectorAction);
    command.append(char(leftReceiptTimeout));

    return this->mIOPort->write(command);
}

//--------------------------------------------------------------------------------
