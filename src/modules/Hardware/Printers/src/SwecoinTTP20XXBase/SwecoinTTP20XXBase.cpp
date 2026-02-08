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
    m_DeviceName = "Swecoin TTP Series printer";

    // данные порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200); // default
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);

    m_PortParameters[EParameters::Parity].append(EParity::No); // default
    m_PortParameters[EParameters::Parity].append(EParity::Even);
    m_PortParameters[EParameters::Parity].append(EParity::Odd);

    // теги
    m_TagEngine = Tags::PEngine(new CSwecoinPrinter::TagEngine());

    // данные устройства
    m_DeviceName = "Swecoin Printer Series";
    setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x1E");
    setConfigParameter(CHardware::Printer::Commands::Presentation, "\x1B\x0C");
    setConfigParameter(CHardware::Printer::Settings::PresentationLength, 50);
}

//--------------------------------------------------------------------------------
bool SwecoinPrinter::isConnected() {
    if (!m_IOPort->write(CSwecoinPrinter::Commands::GetModelId)) {
        return false;
    }

    QByteArray answer;
    QElapsedTimer clockTimer;
    clockTimer.start();
    int length = 0;

    do {
        QByteArray data;

        if (!m_IOPort->read(data)) {
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
        m_DeviceName = match.captured(1).simplified();
    }

    return true;
}

//--------------------------------------------------------------------------------
bool SwecoinPrinter::updateParameters() {
    CSwecoinPrinter::TDeviceParameters deviceParameters =
        CSwecoinPrinter::CDeviceParameters().data();

    for (auto it = deviceParameters.begin(); it != deviceParameters.end(); ++it) {
        QByteArray data;

        if (m_IOPort->write(QByteArray(CSwecoinPrinter::Commands::GetData) + it.key()) &&
            m_IOPort->read(data)) {
            setDeviceParameter(it->description, it->handler(data));
        }
    }

    // TODO: установка шрифта, параметров эжектора, межстрочного интервала, ...
    bool result = m_IOPort->write(CSwecoinPrinter::Commands::Initialize);

    SleepHelper::msleep(CSwecoinPrinter::Pause);

    return result;
}

//--------------------------------------------------------------------------------
bool SwecoinPrinter::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray answer;

    if (!m_IOPort->write(CSwecoinPrinter::Commands::GetStatus) || !m_IOPort->read(answer) ||
        (answer.size() > 2) || answer.isEmpty() ||
        ((answer.size() != 1) && answer.startsWith(ASCII::ACK)) ||
        ((answer.size() != 2) && answer.startsWith(ASCII::NAK))) {
        return false;
    }

    if (answer[0] != ASCII::ACK) {
        aStatusCodes.insert(CSwecoinPrinter::Statuses[answer[1]]);
    }

    if (!m_IOPort->write(CSwecoinPrinter::Commands::GetPaperNearEndData) ||
        !m_IOPort->read(answer) || (answer.size() != 1)) {
        return false;
    }

    if (answer[0]) {
        aStatusCodes.insert(PrinterStatusCode::Warning::PaperNearEnd);
    }

    if (!m_IOPort->write(CSwecoinPrinter::Commands::GetSensorData) || !m_IOPort->read(answer) ||
        (answer.size() != 2)) {
        return false;
    }

    if (answer[1] & CSwecoinPrinter::PaperInPresenterMask) {
        aStatusCodes.insert(PrinterStatusCode::OK::PaperInPresenter);
    }

    return true;
}

//--------------------------------------------------------------------------------
