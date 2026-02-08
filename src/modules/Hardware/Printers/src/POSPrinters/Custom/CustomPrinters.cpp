/* @file Принтеры Custom. */

#include "CustomPrinters.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QtMath>

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний принтеров Custom.
namespace CCustomPrinter {
namespace Models {
const char TG2480[] = "Custom TG2480";
const char TG2480H[] = "Custom TG2480H";
const char TG2460H[] = "Custom TG2460H";
const char TL80[] = "Custom TL80";
const char TL60[] = "Custom TL60";
} // namespace Models

/// Команды.
namespace Commands {
const char PrintImage[] = "\x1B\x2A"; /// Печать изображения.
} // namespace Commands

//----------------------------------------------------------------------------
/// GAM (Graphics advanced mode).
namespace GAM {
/// Команды.
namespace Commands {
const char SetPageLength[] = "\x1B\x26\x6C\x30\x52"; /// Установить длину страницы = 0. По размеру?
const char SetResolution204[] = "\x1B\x2A\x74\x32\x30\x34\x52"; /// Установить разрешение 240 dpi.
const char SetNoCompression[] = "\x1B\x2A\x62\x30\x4D";         /// Отменить сжатие изображения.
const char SetLeftMargin[] = "\x1B\x2A\x70\x58";                /// Установить сдвиг слева.
const char SendData[] = "\x1B\x2A\x62\x57";                     /// Послать картинку в принтер.
const char PrintImage[] = "\x1B\x2A\x72\x42";                   /// Печать изображения.
} // namespace Commands
} // namespace GAM
} // namespace CCustomPrinter

template class CustomPrinter<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template <class T> CustomPrinter<T>::CustomPrinter() {
    this->m_Parameters = POSPrinters::CommonParameters;

    // статусы ошибок
    this->m_Parameters.errors.clear();

    this->m_Parameters.errors[20][3].insert('\x01', PrinterStatusCode::Error::PaperEnd);
    this->m_Parameters.errors[20][3].insert('\x04', PrinterStatusCode::Warning::PaperNearEnd);
    this->m_Parameters.errors[20][3].insert('\x20', PrinterStatusCode::OK::PaperInPresenter);
    // this->m_Parameters.errors[20][3].insert('\x40', PrinterStatusCode::Warning::PaperEndVirtual);

    this->m_Parameters.errors[20][4].insert('\x01', PrinterStatusCode::Error::PrintingHead);
    this->m_Parameters.errors[20][4].insert('\x02', DeviceStatusCode::Error::CoverIsOpened);

    this->m_Parameters.errors[20][5].insert('\x01', PrinterStatusCode::Error::Temperature);
    this->m_Parameters.errors[20][5].insert('\x04', PrinterStatusCode::Error::Port);
    this->m_Parameters.errors[20][5].insert('\x08', DeviceStatusCode::Error::PowerSupply);
    this->m_Parameters.errors[20][5].insert('\x40', PrinterStatusCode::Error::PaperJam);

    this->m_Parameters.errors[20][6].insert('\x01', PrinterStatusCode::Error::Cutter);
    this->m_Parameters.errors[20][6].insert('\x4C', DeviceStatusCode::Error::MemoryStorage);

    // теги
    this->m_Parameters.tagEngine.appendSingle(Tags::Type::Italic, "\x1B\x34", "\x01");
    this->m_Parameters.tagEngine.appendCommon(Tags::Type::DoubleWidth, "\x1B\x21", "\x20");
    this->m_Parameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");

    // параметры моделей
    this->m_DeviceName = "Custom Printer";
    this->m_ModelID = '\x93';
    this->m_PrintingStringTimeout = 50;

    // модели
    this->m_ModelData.data().clear();
    this->m_ModelData.add('\x93', false, CCustomPrinter::Models::TG2480);
    this->m_ModelData.add('\xA7', false, CCustomPrinter::Models::TG2460H);
    this->m_ModelData.add('\xAC', false, CCustomPrinter::Models::TL80);
    this->m_ModelData.add('\xAD', false, CCustomPrinter::Models::TL60);

    this->setConfigParameter(CHardware::Printer::FeedingAmount, 1);
}

//--------------------------------------------------------------------------------
template <class T> QStringList CustomPrinter<T>::getModelList() {
    return QStringList() << CCustomPrinter::Models::TG2480 << CCustomPrinter::Models::TG2460H
                         << CCustomPrinter::Models::TL80 << CCustomPrinter::Models::TL60;
}

//--------------------------------------------------------------------------------
template <class T>
void CustomPrinter<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    this->setDeviceConfiguration(aConfiguration);

    int lineSpacing = this->getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();
    int feeding = (lineSpacing >= 60) ? 0 : 1;

    this->setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
template <class T>
bool CustomPrinter<T>::printImageDefault(const QImage &aImage, const Tags::TTypes &aTags) {
    int width = aImage.width();
    int height = aImage.height();
    int lines = qCeil(height / double(CCustomPrinter::LineHeight));
    int widthInBytes = qCeil(width / 8.0);

    if (width > CCustomPrinter::MaxImageWidth) {
        this->toLog(LogLevel::Warning,
                    this->m_DeviceName +
                        QString(": Image width > %1, so it cannot be printing properly")
                            .arg(CCustomPrinter::MaxImageWidth));
        return false;
    }

    if (!this->m_IOPort->write(CPOSPrinter::Command::SetLineSpacing(0))) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName + ": Failed to set null line spacing for printing the image");
        return false;
    }

    bool result = true;

    for (int i = 0; i < lines; ++i) {
        QList<QByteArray> lineData;

        for (int j = 0; j < CCustomPrinter::LineHeight; ++j) {
            int index = i * CCustomPrinter::LineHeight + j;

            if (index < height) {
                lineData << QByteArray::fromRawData((const char *)aImage.scanLine(index),
                                                    widthInBytes);
            } else {
                lineData << QByteArray(widthInBytes, ASCII::NUL);
            }
        }

        QByteArray request(CCustomPrinter::Commands::PrintImage);
        request.append(CCustomPrinter::Image24BitMode);
        request.append(uchar(width % 256));
        request.append(uchar(width / 256));

        for (int j = 0; j < width; ++j) {
            QByteArray data(3, ASCII::NUL);
            char mask = 1 << (7 - (j % 8));

            for (int k = 0; k < CCustomPrinter::LineHeight; ++k) {
                if (lineData[k][j / 8] & mask) {
                    data[k / 8] = data[k / 8] | (1 << (7 - (k % 8)));
                }
            }

            request.append(data);
        }

        if (!this->m_IOPort->write(request + ASCII::LF)) {
            result = false;

            break;
        }
    }

    int lineSpacing = this->getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

    if (!this->m_IOPort->write(CPOSPrinter::Command::SetLineSpacing(lineSpacing))) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName + ": Failed to set line spacing after printing the image");
    }

    return result;
}

//--------------------------------------------------------------------------------

template <class T>
bool CustomPrinter<T>::printImage(const QImage &aImage, const Tags::TTypes &aTags) {
    int width = aImage.width();

    if (width > CCustomPrinter::GAM::MaxImageWidth) {
        this->toLog(LogLevel::Warning,
                    this->m_DeviceName +
                        QStringLiteral(": Image width > %1, so it cannot be printing properly")
                            .arg(CCustomPrinter::GAM::MaxImageWidth));
        return false;
    }

    // Использование QStringLiteral и QByteArray::append для чистоты кода
    QByteArray initializeGAM;
    initializeGAM.append(CCustomPrinter::GAM::Commands::SetPageLength);
    initializeGAM.append(CCustomPrinter::GAM::Commands::SetResolution204);
    initializeGAM.append(CCustomPrinter::GAM::Commands::SetNoCompression);

    int leftMargin = qFloor((CCustomPrinter::GAM::MaxImageWidth - width) / 2.0);

    if (aTags.contains(Tags::Type::Center) && (leftMargin > 0)) {
        // Формируем команду отступа: вставляем значение перед последним байтом-разделителем
        QByteArray marginValue = QByteArray::number(leftMargin);
        initializeGAM.insert(initializeGAM.size() - 1, marginValue);
        initializeGAM.prepend(CCustomPrinter::GAM::Commands::SetLeftMargin);
    }

    // В Qt 6 для замера интервалов используем QElapsedTimer (точнее и быстрее)
    QElapsedTimer timer;
    timer.start();

    if (!this->m_IOPort->write(initializeGAM)) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName + QStringLiteral(": Failed to initialize GAM"));
        return false;
    }

    int widthInBytes = qCeil(width / 8.0);

    for (int i = 0; i < aImage.height(); ++i) {
        // Qt 6: scanLine возвращает uchar*, приводим к const char* для QByteArray
        QByteArray data(reinterpret_cast<const char *>(aImage.scanLine(i)), widthInBytes);

        QByteArray command = CCustomPrinter::GAM::Commands::SendData;
        command.append(data);

        // Вставляем длину данных в команду (индекс 3 согласно протоколу GAM)
        command.insert(3, QByteArray::number(data.size()));

        if (!this->m_IOPort->write(command)) {
            this->toLog(LogLevel::Error,
                        this->m_DeviceName + QStringLiteral(": Failed to send image data"));
            return false;
        }
    }

    if (!this->m_IOPort->write(CCustomPrinter::GAM::Commands::PrintImage)) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName + QStringLiteral(": Failed to set resolution 204 dpi"));
        return false;
    }

    SDK::Driver::TPortParameters portParameters;
    this->m_IOPort->getParameters(portParameters);
    qint64 pause = CCustomPrinter::GAM::getImagePause(aImage, portParameters);

    // Расчет оставшейся паузы
    qint64 elapsed = timer.elapsed();
    this->toLog(LogLevel::Normal,
                this->m_DeviceName +
                    QStringLiteral(": Pause after printing image = %1 - %2 = %3 (ms)")
                        .arg(pause)
                        .arg(elapsed)
                        .arg(pause - elapsed));

    pause -= elapsed;

    if (pause > 0) {
        SleepHelper::msleep(static_cast<int>(pause));
    }

    return true;
}

//--------------------------------------------------------------------------------
