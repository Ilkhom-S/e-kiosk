#include "Phoenix.h"

Phoenix_PRINTER::Phoenix_PRINTER(QObject *parent) : BasePrinterDevices(parent) {}

bool Phoenix_PRINTER::OpenPrinterPort() {
    this->openPort();
    return is_open;
}

bool Phoenix_PRINTER::openPort() {
    if (devicesCreated) {
        // Если девайс для работы с портом обявлен
        is_open = false;

        // Даем девайсу название порта
        serialPort->setPortName(com_Name);

        if (serialPort->open(QIODevice::ReadWrite)) {
            // Устанавливаем параметры открытия порта
            is_open = false;
            if (!serialPort->setDataBits(QSerialPort::Data8))
                return false;
            if (!serialPort->setParity(QSerialPort::NoParity))
                return false;
            if (!serialPort->setStopBits(QSerialPort::OneStop))
                return false;
            if (!serialPort->setFlowControl(QSerialPort::SoftwareControl))
                return false;
            if (!serialPort->setBaudRate(QSerialPort::Baud9600))
                return false;

            is_open = true;
        } else {
            is_open = false;
        }
    } else {
        is_open = false;
    }

    return is_open;
}

bool Phoenix_PRINTER::isItYou() {
    int status = 0;
    bool result = isEnabled(status);
    this->closePort();
    return result;
}

bool Phoenix_PRINTER::isEnabled(int status) {
    //        int status = 0;
    if (!getStatus(status))
        return false;
    return (status != PrinterState::PrinterNotAvailable);
}

bool Phoenix_PRINTER::getStatus(int &aStatus) {
    aStatus = PrinterState::PrinterNotAvailable;
    // засылаем в порт команду самоидентификации
    QByteArray cmd;
    QByteArray answer;
    bool resp_data = false;

    cmd.push_back(CMDPhoenix::PrinterStatusCommandFirstByte);
    cmd.push_back(CMDPhoenix::PrinterStatusCommandSecondByte);
    if (!this->sendCommand(cmd, true, 200, resp_data, answer, 0)) {
        // if(Debugger) qDebug() << "AV268::getStatus(): error in
        // sendPacketInPort()";
        return false;
    }

    if (answer.size() < 1) {
        // if(Debugger) qDebug() << QString("AV268::getStatus(): wrong size of
        // buffer. Buffer is: %1").arg(answer.data());
        return false;
    }

    uchar status = 0;
    // В некоторых случаях присылается больше 1 байта
    // тогда наш байт - второй
    if (answer.size() > 1)
        status = answer[1];
    else
        status = answer[0];
    // Проверим, что это наш статус
    if ((status & 0x10) || (status & 0x80)) {
        // Не наш принтер
        // if(Debugger) qDebug() << QString("AV268::getStatus(): wrong byte returned:
        // %1").arg(status);
        return false;
    }
    // Наш принтер
    aStatus = PrinterState::PrinterOK;
    if (status != CMDPhoenix::PrinterNormalState) {
        // Error
        int code = status & CMDPhoenix::PrinterTemperatureError;
        if (code > 0) {
            // Temperature error
            aStatus |= PrinterState::TemperatureError;
            // if(Debugger) qDebug() << "AV268::getStatus(): Temperature error";
        }
        code = status & CMDPhoenix::PrinterNoPaperError;
        if (code > 0) {
            // No paper
            aStatus |= PrinterState::PaperEnd;
            // if(Debugger) qDebug() << "AV268::getStatus(): No paper";
        }
        code = status & CMDPhoenix::PrinterHeadOpenError;
        if (code > 0) {
            // Printing head open
            aStatus |= PrinterState::PrintingHeadError;
            // if(Debugger) qDebug() << "AV268::getStatus(): Printing head open";
        }
        code = status & CMDPhoenix::PrinterSystem_Error;
        if (code > 0) {
            // System error
            aStatus |= PrinterState::PrinterError;
            // if(Debugger) qDebug() << "AV268::getStatus(): System error";
        }
        code = status & CMDPhoenix::PrinterDataReceiveError;
        if (code > 0) {
            // Data receive error
            aStatus |= PrinterState::PortError;
            // if(Debugger) qDebug() << "AV268::getStatus(): Data receive error";
        }
    }

    return true;
}

bool Phoenix_PRINTER::initialize() {
    // засылаем в порт команду самоидентификации
    QByteArray cmd;
    cmd.push_back(CMDPhoenix::PrinterStatusCommandFirstByte);
    cmd.push_back(CMDPhoenix::PrinterInitCommandSecondByte);
    bool resp_data = false;
    QByteArray response;

    bool res = this->sendCommand(cmd, false, 200, resp_data, response, 290);

    return res;
}

bool Phoenix_PRINTER::cut() {
    QByteArray cmd;

    cmd.push_back(CMDPhoenix::PrinterStatusCommandFirstByte);
    cmd.push_back(CMDPhoenix::PrinterCutCommandSecondByte);

    bool resp_data = false;
    QByteArray response;
    bool res = this->sendCommand(cmd, true, 200, resp_data, response, 50);

    return res;
}

void Phoenix_PRINTER::getSpecialCharacters(QByteArray &printText) {

    // Устанавливаем если есть жирный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeBold +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть двойной высоты фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                              CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть двойной ширины фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                              CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть курсивный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeItalic +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть подчеркнутый фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeUnderLine +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeUnderLine + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Если надо добавить пробел
    QByteArray probel;
    for (int i = 1; i <= leftMargin; i++) {
        probel.append(ASCII::Space);
    }
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::SpaceCount +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      probel);

    // Добавляем звезды
    int col_z = (checkWidth - 11 - leftMargin) / 2;
    QByteArray star;
    for (int j = 1; j <= col_z; j++)
        star.append("*");

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::StarCount +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      star);
}

bool Phoenix_PRINTER::printCheck(const QString &aCheck) {
    // Меняем кодировку
    QByteArray printText;
    printText = this->encodingString(aCheck, CScodec::c_IBM866);

    // Вставляем если есть Подчеркнутый, Жирный, Курсивный... Шрифт
    this->getSpecialCharacters(printText);

    //    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    if (!this->sendCommand(printText, false, 0, respData, answer, 0))
        return false;

    return true;
}

bool Phoenix_PRINTER::print(const QString &aCheck) {
    this->initialize();
    quint16 avzero = 0;
    // установим размер шрифта
    QByteArray cmd;
    cmd.push_back(CMDPhoenix::PrinterStatusCommandFirstByte);
    cmd.push_back(0x4D);
    cmd.push_back(avzero);

    bool resp_data = false;
    QByteArray response;

    this->sendCommand(cmd, false, 200, resp_data, response, 290);

    // Печатаем текст
    this->printCheck(aCheck);
    // Обрезка
    this->cut();

    return true;
}
