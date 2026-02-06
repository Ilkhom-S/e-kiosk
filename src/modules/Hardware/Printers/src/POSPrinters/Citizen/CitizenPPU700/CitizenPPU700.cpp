/* @file Принтер Citizen PPU-700. */

#include "CitizenPPU700.h"

#include <QtCore/QElapsedTimer>

template class SerialPOSPrinter<CitizenPPU700<TSerialPrinterBase>>;
template class SerialPOSPrinter<CitizenPPU700II<TSerialPrinterBase>>;

//--------------------------------------------------------------------------------
template <class T> CitizenPPU700<T>::CitizenPPU700() {
    // статусы ошибок
    this->mParameters.errors.clear();

    this->mParameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

    this->mParameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
    this->mParameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
    this->mParameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

    this->mParameters.errors[3][1].insert('\x04', PrinterStatusCode::Error::Presenter);
    this->mParameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
    this->mParameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

    this->mParameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
    this->mParameters.errors[4][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
    this->mParameters.errors[4][1].insert('\x40', PrinterStatusCode::OK::PaperInPresenter);

    this->mParameters.errors[5][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
    this->mParameters.errors[5][1].insert('\x08', PrinterStatusCode::Error::Temperature);
    this->mParameters.errors[5][1].insert('\x60', DeviceStatusCode::Error::PowerSupply);

    this->mParameters.errors[6][1].insert('\x0C', DeviceStatusCode::Error::MemoryStorage);
    this->mParameters.errors[6][1].insert('\x20', PrinterStatusCode::Error::Presenter);
    this->mParameters.errors[6][1].insert('\x40', DeviceStatusCode::Error::Electronic);

    // параметры моделей
    this->mDeviceName = "Citizen PPU-700";
    this->mModelID = '\x75';
    this->setConfigParameter(CHardware::Printer::FeedingAmount, 4);

    // модели
    this->mModelData.data().clear();
    this->mModelData.add(this->mModelID, true, this->mDeviceName);
    this->mMaxBadAnswers = 4;
    this->mOptionMSW = false;
}

//--------------------------------------------------------------------------------
template <class T> bool CitizenPPU700<T>::isConnected() {
    if (!this->isConnected()) {
        return false;
    }

    if (this->mModelCompatibility) {
        QByteArray answer;

        if (!this->mIOPort->write(CCitizenPPU700::Command::GetMemorySwitch5) ||
            !this->mIOPort->read(answer,
                                 CCitizenPPU700::MemorySwitches::ReadingTimeout,
                                 CCitizenPPU700::MemorySwitches::AnswerSize)) {
            return false;
        }

        this->toLog(LogLevel::Normal,
                    QString("%1: << {%2}").arg(this->mDeviceName).arg(answer.toHex().data()));

        this->mModelCompatibility = this->mOptionMSW == !answer.isEmpty();
        this->mDeviceName = answer.isEmpty() ? "Citizen PPU-700" : "Citizen PPU-700II";
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void CitizenPPU700<T>::processDeviceData() {
    this->processDeviceData();
    QByteArray answer;

    if (this->mIOPort->write(CCitizenPPU700::Command::GetFirmware) &&
        this->getNULStoppedAnswer(answer, CPOSPrinter::Timeouts::Info)) {
        this->setDeviceParameter(CDeviceData::Firmware, answer);
    }

    if (this->mIOPort->write(CCitizenPPU700::Command::GetSerialNumber) &&
        this->getNULStoppedAnswer(answer, CPOSPrinter::Timeouts::Info)) {
        this->setDeviceParameter(CDeviceData::SerialNumber, answer);
    }
}

//--------------------------------------------------------------------------------
template <class T>
bool CitizenPPU700<T>::getNULStoppedAnswer(QByteArray &aAnswer, int aTimeout) const {
    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::Write));
    this->mIOPort->setDeviceConfiguration(configuration);

    aAnswer.clear();

    QElapsedTimer timer;
    timer.start();

    do {
        QByteArray data;

        if (!this->mIOPort->read(data, 10)) {
            return false;
        }

        aAnswer.append(data);
    } while (!aAnswer.endsWith(ASCII::NUL) && (timer.elapsed() < aTimeout));

    this->toLog(aAnswer.isEmpty() ? LogLevel::Warning : LogLevel::Normal,
                QString("%1: << {%2}").arg(this->mDeviceName).arg(aAnswer.toHex().data()));

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
