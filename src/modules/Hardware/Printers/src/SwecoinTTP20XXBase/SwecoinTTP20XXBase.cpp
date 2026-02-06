/* @file Принтеры Swecoin. */

#include "SwecoinTTP20XXBase.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QRegularExpression>

#include <math.h>

#include "SwecoinPrinterData.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
SwecoinPrinter::SwecoinPrinter() {
    // данные устройства
    mDeviceName = "Swecoin TTP Series printer";

    // данные порта
    mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200); // default
    mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
    mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
    mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
    mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);

    mPortParameters[EParameters::Parity].append(EParity::No); // default
    mPortParameters[EParameters::Parity].append(EParity::Even);
    mPortParameters[EParameters::Parity].append(EParity::Odd);

    // теги
    mTagEngine = Tags::PEngine(new CSwecoinPrinter::TagEngine());

    // данные устройства
    mDeviceName = "Swecoin Printer Series";
    setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x1E");
    setConfigParameter(CHardware::Printer::Commands::Presentation, "\x1B\x0C");
    setConfigParameter(CHardware::Printer::Settings::PresentationLength, 50);
}

//--------------------------------------------------------------------------------
bool SwecoinPrinter::isConnected() {
    if (!mIOPort->write(CSwecoinPrinter::Commands::GetModelId)) {
        return false;
    }

    QByteArray answer;
    QElapsedTimer clockTimer;
    clockTimer.start();
    int length = 0;

    do {
        QByteArray data;

        if (!mIOPort->read(data)) {
            return false;
        }

        // В Qt 6 при обращении к байтам QByteArray возвращается char,
        // используем uchar для корректного получения числового значения длины.
        if ((answer.size() > 1) && (answer[0] == ASCII::NUL)) {
            length = static_cast<unsigned char>(answer[1]);
        }

        answer.append(data);
    } while ((length && (answer.size() < length) &&
              (clockTimer.elapsed() < CSwecoinPrinter::MaxReadIdTimeout)) ||
             (!length && (clockTimer.elapsed() < CSwecoinPrinter::MinReadIdTimeout)));

    if (!length || (answer.size() < length)) {
        return false;
    }

    // Извлекаем идентификатор модели, пропуская заголовки
    answer = answer.mid(2);

    // 1. Используем QRegularExpression. В Qt 6 это значительно быстрее QRegExp.
    QRegularExpression regExp(QStringLiteral(".*MODEL:([^;]+);.*"));

    // 2. Выполняем сопоставление и получаем объект QRegularExpressionMatch
    // match() автоматически выполнит конвертацию QByteArray в QString через UTF-8
    QRegularExpressionMatch match = regExp.match(QString::fromUtf8(answer));

    // 3. Проверяем наличие совпадения через hasMatch()
    if (match.hasMatch()) {
        // 4. Извлекаем захваченную группу (индекс 1) и упрощаем строку
        mDeviceName = match.captured(1).simplified();
    }

    return true;
}

//--------------------------------------------------------------------------------
bool SwecoinPrinter::updateParameters() {
    CSwecoinPrinter::TDeviceParameters deviceParameters =
        CSwecoinPrinter::CDeviceParameters().data();

    for (auto it = deviceParameters.begin(); it != deviceParameters.end(); ++it) {
        QByteArray data;

        if (mIOPort->write(QByteArray(CSwecoinPrinter::Commands::GetData) + it.key()) &&
            mIOPort->read(data)) {
            setDeviceParameter(it->description, it->handler(data));
        }
    }

    // TODO: установка шрифта, параметров эжектора, межстрочного интервала, ...
    bool result = mIOPort->write(CSwecoinPrinter::Commands::Initialize);

    SleepHelper::msleep(CSwecoinPrinter::Pause);

    return result;
}

//--------------------------------------------------------------------------------
bool SwecoinPrinter::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray answer;

    if (!mIOPort->write(CSwecoinPrinter::Commands::GetStatus) || !mIOPort->read(answer) ||
        (answer.size() > 2) || answer.isEmpty() ||
        ((answer.size() != 1) && answer.startsWith(ASCII::ACK)) ||
        ((answer.size() != 2) && answer.startsWith(ASCII::NAK))) {
        return false;
    }

    if (answer[0] != ASCII::ACK) {
        aStatusCodes.insert(CSwecoinPrinter::Statuses[answer[1]]);
    }

    if (!mIOPort->write(CSwecoinPrinter::Commands::GetPaperNearEndData) || !mIOPort->read(answer) ||
        (answer.size() != 1)) {
        return false;
    }

    if (answer[0]) {
        aStatusCodes.insert(PrinterStatusCode::Warning::PaperNearEnd);
    }

    if (!mIOPort->write(CSwecoinPrinter::Commands::GetSensorData) || !mIOPort->read(answer) ||
        (answer.size() != 2)) {
        return false;
    }

    if (answer[1] & CSwecoinPrinter::PaperInPresenterMask) {
        aStatusCodes.insert(PrinterStatusCode::OK::PaperInPresenter);
    }

    return true;
}

//--------------------------------------------------------------------------------
