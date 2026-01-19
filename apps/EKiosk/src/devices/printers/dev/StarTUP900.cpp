// Project
#include "StarTUP900.h"

StarTUP900_PRINTER::StarTUP900_PRINTER(QObject *parent) : BasePrinterDevices(parent) {
    //    printer_name = "Custom-VKP80";
}

bool StarTUP900_PRINTER::OpenPrinterPort() {
    this->openPort();

    return is_open;
}

bool StarTUP900_PRINTER::openPort() {
    if (devicesCreated) {
        // Если девайс для работы с портом обявлен
        is_open = false;
        //    return is_open;
        // Даем девайсу название порта
        serialPort->setPortName(comName);

        if (serialPort->open(QIODevice::ReadWrite)) {
            // Устанавливаем параметры открытия порта
            is_open = false;

            if (!serialPort->setDataBits(QSerialPort::Data8))
                return false;
            if (!serialPort->setParity(QSerialPort::NoParity))
                return false;
            if (!serialPort->setStopBits(QSerialPort::OneStop))
                return false;
            if (!serialPort->setFlowControl(QSerialPort::NoFlowControl))
                return false;
            if (!serialPort->setBaudRate(QSerialPort::Baud19200))
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

bool StarTUP900_PRINTER::isEnabled() {
    int status = 0;
    //    CMDCustomVKP80::SStatus s_status;
    if (!this->getStatus(status))
        return false;
    return (status != PrinterState::PrinterNotAvailable);
}

bool StarTUP900_PRINTER::isItYou() {

    bool result = isEnabled();

    return result;
}

bool StarTUP900_PRINTER::cut() {
    QByteArray cmd;
    QByteArray answer;
    bool respData;

    cmd.push_back(CMDStarTUP900::PrinterCommandCutFirstByte);
    cmd.push_back(CMDStarTUP900::PrinterCommandCutSecondByte);
    cmd.push_back(48);

    return this->sendCommand(cmd, true, CMDStarTUP900::TimeOutAfterWriting, respData, answer, 0);
}

//--------------------------------------------------------------------------------
bool StarTUP900_PRINTER::initialize() {
    QByteArray cmd;
    QByteArray answer;
    bool respData;

    cmd.push_back(CMDStarTUP900::PrinterCommandInitFirstByte);
    cmd.push_back(CMDStarTUP900::PrinterCommandInitSecondByte);
    if (!this->sendCommand(cmd, true, CMDStarTUP900::TimeOutAfterWriting, respData, answer, 0))
        return false;

    // Устанавливаем левую границу
    cmd.clear();
    cmd.push_back(CMDStarTUP900::PrinterCommandInitFirstByte);
    cmd.push_back(CMDStarTUP900::PrinterCommandSetLeftMarginSecondByte);
    cmd.push_back(0x02 /*0x09*/); // Смещение от левой границы
    if (!this->sendCommand(cmd, true, CMDStarTUP900::TimeOutAfterWriting, respData, answer, 0))
        return false;

    // PresenterAutoPush
    cmd.clear();
    cmd.push_back(CMDStarTUP900::PrinterCommandInitFirstByte);
    cmd.push_back(CMDStarTUP900::PresenterAutoPushCmdSecondByte);
    cmd.push_back(CMDStarTUP900::PresenterAutoPushCmdThirdByte);
    cmd.push_back(60);
    if (!this->sendCommand(cmd, true, CMDStarTUP900::TimeOutAfterWriting, respData, answer, 0))
        return false;

    // Code page
    cmd.clear();
    cmd.push_back(0x1B);
    cmd.push_back(0x1D);
    cmd.push_back(0x74);
    cmd.push_back(10);
    if (!this->sendCommand(cmd, true, CMDStarTUP900::TimeOutAfterWriting, respData, answer, 0))
        return false; /**/

    // Set font
    cmd.clear();
    cmd.push_back(CMDStarTUP900::PrinterCommandInitFirstByte);
    cmd.push_back(CMDStarTUP900::PrinterSetFontCmdSecondByte);
    cmd.push_back('0');
    cmd.push_back('0');
    return this->sendCommand(cmd, true, CMDStarTUP900::TimeOutAfterWriting, respData, answer, 0);
}

bool StarTUP900_PRINTER::print(const QString &aCheck) {
    this->initialize();

    this->printCheck(aCheck);

    if (!feed(4))
        return false;

    return cut();
}

bool StarTUP900_PRINTER::feed(int aCount) {
    QByteArray cmd;
    bool result = false;
    QByteArray answer;
    cmd.push_back(CMDStarTUP900::PrinterCommandFeedFirstByte);
    cmd.push_back(CMDStarTUP900::PrinterCommandFeedSecondByte);
    for (int i = 0; i < aCount; ++i) {
        if (!sendCommand(cmd, true, CMDStarTUP900::TimeOutAfterWriting, result, answer, 0))
            return false;
    }
    return true;
}

bool StarTUP900_PRINTER::printCheck(const QString &aCheck) {
    QByteArray printText;
    printText = this->encodingString(aCheck, CScodec::c_IBM866);

    // Вставляем если есть Подчеркнутый, Жирный, Курсивный... Шрифт
    this->getSpecialCharecters(printText);

    //    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    auto result = this->sendCommand(printText, true, 200, respData, answer, 50);

    return result;
}

void StarTUP900_PRINTER::getSpecialCharecters(QByteArray &printText) {
    // Устанавливаем если есть жирный фонт
    printText.replace(
        QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter)
            .toUtf8(),
        asciiNull());

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть двойной высоты фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleHeight + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть двойной ширины фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleWidth + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть курсивный фонт
    printText.replace(
        QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter)
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

    // Если надо добавить проабел
    QByteArray probel;
    for (int i = 1; i <= leftMargin; i++) {
        probel.append(ASCII::Space);
    }
    printText.replace(
        QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::ProbelCount + CScharsetParam::CloseTagDelimiter)
            .toUtf8(),
        probel);

    // Добавляем звезды
    int col_z = (chekWidth - 11 - leftMargin) / 2;
    QByteArray star;
    for (int j = 1; j <= col_z; j++)
        star.append("*");

    printText.replace(
        QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::StarCount + CScharsetParam::CloseTagDelimiter)
            .toUtf8(),
        star);
}

bool StarTUP900_PRINTER::getStatus(int &aStatus) {
    QByteArray status = getState();
    int result = 0;
    if (status.size() != 1) {
        result |= PrinterState::PrinterNotAvailable;
        aStatus = result;
        return false;
    }

    // проверка фиксированных битов
    uchar stateByte = status[0];
    if ((stateByte & 0x80) || !(stateByte & 0x10) || (stateByte & 0x01)) {
        // не его статус
        result |= PrinterState::PrinterNotAvailable;
        aStatus = result;
        return false;
    }

    int paperDetectorError = status[0];
    if (paperDetectorError == CMDStarTUP900::PrinterIsNotAvailable) {
        // Printer is not available
        result |= PrinterState::PrinterNotAvailable;
        // if(Debuger) qDebug() << "StarTUP900::getStatus(): Printer is not
        // available";
        aStatus = result;
        return true;
    }

    if (paperDetectorError != CMDStarTUP900::PrinterIsOK) {
        // Error
        int code = paperDetectorError & CMDStarTUP900::PaperNearEnd;
        if (code > 0) {
            // Paper near end
            result |= PrinterState::PaperNearEnd;
            // if(Debuger) qDebug() << "StarTUP900::getStatus(): Paper near end";
        }
        code = paperDetectorError & CMDStarTUP900::PaperEnd;
        if (code > 0) {
            // Paper end
            result |= PrinterState::PaperEnd;
            // if(Debuger) qDebug() << "StarTUP900::getStatus(): Paper end";
        }
        code = paperDetectorError & CMDStarTUP900::PaperJamError;
        if (code > 0) {
            // Paper jam
            result |= PrinterState::PaperJam;
            // if(Debuger) qDebug() << "StarTUP900::getStatus(): Paper jam";
        }
    }
    aStatus = result;
    return true;
}

QByteArray StarTUP900_PRINTER::getState() {
    // засылаем в порт команду получения статуса
    QByteArray cmd;
    bool respData;
    QByteArray answer;

    cmd.push_back(CMDStarTUP900::PrinterStatusCommand);

    if (!this->sendCommand(cmd, true, CMDStarTUP900::TimeOutAfterWriting, respData, answer, 0)) {
        // if(Debuger) qDebug() << "StarTUP900::getState(): error in
        // sendPacketInPort()"; 		return answer;
    }

    return answer;
}
