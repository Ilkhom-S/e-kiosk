/* @file Принтер Custom VKP-80 III. */

#include "CustomVKP80III.h"

//--------------------------------------------------------------------------------
/// Константы и команды Custom VKP-80 III.
namespace CCustomVKP80III {
/// Команды.
namespace Command {
const char GetModelId[] = "\x1D\x49\xFF";    /// Получение идентификатора модели.
const char EjectorActivation[] = "\x1C\x50"; /// Неизменяемая часть команды активации эжектора.
} // namespace Command

const char ModelId[] = "\x02\x05"; /// Идентификатор модели.
} // namespace CCustomVKP80III

template class CustomVKP80III<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template <class T> CustomVKP80III<T>::CustomVKP80III() {
    // параметры моделей
    this->m_DeviceName = "Custom VKP-80 III";
    this->m_ModelID = '\xFF';

    // модели
    this->m_Parameters = POSPrinters::CommonParameters;
    this->m_ModelData.data().clear();
    this->m_ModelData.add(this->m_ModelID, true, this->m_DeviceName);

    this->setConfigParameter(CHardware::Printer::PresenterEnable, false);
    this->setConfigParameter(CHardware::Printer::RetractorEnable, false);
}

//--------------------------------------------------------------------------------
template <class T> bool CustomVKP80III<T>::getModelId(QByteArray &aAnswer) const {
    return this->m_IOPort->write(CCustomVKP80III::Command::GetModelId) &&
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
                        .arg(this->m_DeviceName)
                        .arg(aAnswer.toHex().data())
                        .arg(QByteArray(CCustomVKP80III::ModelId).toHex().data()));
        return 0;
    }

    return this->m_ModelID;
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

    return this->m_IOPort->write(command);
}

//--------------------------------------------------------------------------------
