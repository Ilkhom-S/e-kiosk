/* @file Принтер Citizen PPU-700. */

#include "CitizenPPU700.h"

#include <QtCore/QElapsedTimer>

//--------------------------------------------------------------------------------
namespace CCitizenPPU700 {
/// Команды.
namespace Command {
const char GetFirmware[] = "\x1D\x49\x41";     /// Получение версии прошивки.
const char GetSerialNumber[] = "\x1D\x49\x44"; /// Получение серийного номера.
} // namespace Command
} // namespace CCitizenPPU700

template class SerialPOSPrinter<CitizenPPU700<TSerialPrinterBase>>;
template class SerialPOSPrinter<CitizenPPU700II<TSerialPrinterBase>>;

//--------------------------------------------------------------------------------
template <class T> CitizenPPU700<T>::CitizenPPU700() {
    // статусы ошибок
    this->m_Parameters.errors.clear();

    this->m_Parameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

    this->m_Parameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
    this->m_Parameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
    this->m_Parameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

    this->m_Parameters.errors[3][1].insert('\x04', PrinterStatusCode::Error::Presenter);
    this->m_Parameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
    this->m_Parameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

    this->m_Parameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
    this->m_Parameters.errors[4][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
    this->m_Parameters.errors[4][1].insert('\x40', PrinterStatusCode::OK::PaperInPresenter);

    this->m_Parameters.errors[5][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
    this->m_Parameters.errors[5][1].insert('\x08', PrinterStatusCode::Error::Temperature);
    this->m_Parameters.errors[5][1].insert('\x60', DeviceStatusCode::Error::PowerSupply);

    this->m_Parameters.errors[6][1].insert('\x0C', DeviceStatusCode::Error::MemoryStorage);
    this->m_Parameters.errors[6][1].insert('\x20', PrinterStatusCode::Error::Presenter);
    this->m_Parameters.errors[6][1].insert('\x40', DeviceStatusCode::Error::Electronic);

    // параметры моделей
    this->m_DeviceName = "Citizen PPU-700";
    this->m_ModelID = '\x75';
    this->setConfigParameter(CHardware::Printer::FeedingAmount, 4);

    // модели
    this->m_ModelData.data().clear();
    this->m_ModelData.add(this->m_ModelID, true, this->m_DeviceName);
    this->m_MaxBadAnswers = 4;
    this->m_OptionMSW = false;
}

//--------------------------------------------------------------------------------
template <class T> bool CitizenPPU700<T>::isConnected() {
    if (!this->isConnected()) {
        return false;
    }

    if (this->m_ModelCompatibility) {
        QByteArray answer;

        if (!this->m_IOPort->write(CCitizenPPU700::Command::GetMemorySwitch5) ||
            !this->m_IOPort->read(answer,
                                  CCitizenPPU700::MemorySwitches::ReadingTimeout,
                                  CCitizenPPU700::MemorySwitches::AnswerSize)) {
            return false;
        }

        this->toLog(LogLevel::Normal,
                    QString("%1: << {%2}").arg(this->m_DeviceName).arg(answer.toHex().data()));

        this->m_ModelCompatibility = this->m_OptionMSW == !answer.isEmpty();
        this->m_DeviceName = answer.isEmpty() ? "Citizen PPU-700" : "Citizen PPU-700II";
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void CitizenPPU700<T>::processDeviceData() {
    this->processDeviceData();
    QByteArray answer;

    if (this->m_IOPort->write(CCitizenPPU700::Command::GetFirmware) &&
        this->getNULStoppedAnswer(answer, CPOSPrinter::Timeouts::Info)) {
        this->setDeviceParameter(CDeviceData::Firmware, answer);
    }

    if (this->m_IOPort->write(CCitizenPPU700::Command::GetSerialNumber) &&
        this->getNULStoppedAnswer(answer, CPOSPrinter::Timeouts::Info)) {
        this->setDeviceParameter(CDeviceData::SerialNumber, answer);
    }
}

//--------------------------------------------------------------------------------
template <class T>
bool CitizenPPU700<T>::getNULStoppedAnswer(QByteArray &aAnswer, int aTimeout) const {
    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::Write));
    this->m_IOPort->setDeviceConfiguration(configuration);

    aAnswer.clear();

    QElapsedTimer timer;
    timer.start();

    do {
        QByteArray data;

        if (!this->m_IOPort->read(data, 10)) {
            return false;
        }

        aAnswer.append(data);
    } while (!aAnswer.endsWith(ASCII::NUL) && (timer.elapsed() < aTimeout));

    this->toLog(aAnswer.isEmpty() ? LogLevel::Warning : LogLevel::Normal,
                QString("%1: << {%2}").arg(this->m_DeviceName).arg(aAnswer.toHex().data()));

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
void CitizenPPU700<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    this->setDeviceConfiguration(aConfiguration);

    int lineSpacing = this->getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

    int feeding = 4;
    if (lineSpacing >= 202)
        feeding = 0;
    else if (lineSpacing >= 102)
        feeding = 1;
    else if (lineSpacing >= 72)
        feeding = 2;
    else if (lineSpacing >= 52)
        feeding = 3;

    this->setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
