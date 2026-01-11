#include "CitizenCTS2000.h"

CitizenCTS2000_PRINTER::CitizenCTS2000_PRINTER(QObject *parent) : BasePrinterDevices(parent)
{
//    printer_name = "Custom-VKP80";
}

bool CitizenCTS2000_PRINTER::OpenPrinterPort()
{
    return openPort();
}

bool CitizenCTS2000_PRINTER::openPort()
{
    if(devicesCreated){
        //Если девайс для работы с портом обявлен
        is_open = false;

        //Даем девайсу название порта
        serialPort->setPortName(comName);

        if (serialPort->open(QIODevice::ReadWrite)){
            //Устанавливаем параметры открытия порта
            is_open = false;

            if (!serialPort->setDataBits(QSerialPort::Data8)) return false;
            if (!serialPort->setParity(QSerialPort::NoParity)) return false;
            if (!serialPort->setStopBits(QSerialPort::OneStop)) return false;
            if (!serialPort->setFlowControl(QSerialPort::SoftwareControl)) return false;
            if (!serialPort->setBaudRate(QSerialPort::Baud19200)) return false;

            is_open = true;

        } else {
            is_open = false;
        }
    } else {
        is_open = false;
    }

    return is_open;
}

bool CitizenCTS2000_PRINTER::isEnabled(int &status)
{
    if (!getStatus(status)){
            return false;
    }

    return (status != PrinterState::PrinterNotAvailable);
}

bool CitizenCTS2000_PRINTER::isItYou()
{
    QByteArray cmd;
    QByteArray answer;
    bool resp_data = false;

    // Сначала проверим, что это наша модель принтера
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandCutFirstByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandGetIDSecondByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandModelParam); // Узнаем модель принтера

    if (!this->sendCommand(cmd,true,100,resp_data,answer,0)) {
        //if(Debuger) qDebug() << "CitizenPPU700::isItYou(): error in sendPacketInPort()";
        this->closePort();
        return false;
    }

    if (answer.size() == 1) {
        if (answer[0] == CMDCitizenCTS2000::PrinterCTS2000){
            this->closePort();
            return true;
        }
    }else if (answer.isEmpty()) {
        // В ошибочном состоянии этот принтер не возвращает свой id. Проверим на известные ошибки.
        int status;

        if (getStatus(status)) {
            if (status == PrinterState::PrinterNotAvailable) {
                //if(Debuger) qDebug() << "Printer not available.";
                this->closePort();
                return false;
            }

            if (status == PrinterState::PaperEnd) {
                //if(Debuger) qDebug() << "Printer detected and has no paper.";

                this->closePort();
                return true;
            }else if (status & PrinterState::PrinterError) {
                //if(Debuger) qDebug() << "Printer detected and is in an error state.";
                this->closePort();
                return true;
            }
        }
    }

    this->closePort();
    return false;
}

bool CitizenCTS2000_PRINTER::getStatus(int& aStatus)
{
    QByteArray status = getState();
    int result = 0;

    if (status.size() > CMDCitizenCTS2000::StatusAnswerLength) {
        aStatus = PrinterState::PrinterNotAvailable;
        //if(Debuger) qDebug() << "getStatus(): Printer is not available, perhaps device with similar answer";

        return true;
    }

    if (status.size() < 3) {
        result |= PrinterState::PrinterNotAvailable;
        aStatus = result;
        return false;
    }

    char offlineError = status[0];
    char printerError = status[1];
    char paperDetectorError = status[2];

    if ((offlineError == CMDCitizenCTS2000::PrinterIsNotAvailable) ||
            (printerError == CMDCitizenCTS2000::PrinterIsNotAvailable) ||
            (paperDetectorError == CMDCitizenCTS2000::PrinterIsNotAvailable) ||
            (!(positiveMasking(offlineError, CMDCitizenCTS2000::Control::StatusMask) &&
               positiveMasking(printerError, CMDCitizenCTS2000::Control::StatusMask) &&
               positiveMasking(paperDetectorError, CMDCitizenCTS2000::Control::StatusMask)))) {

        // Printer is not available
        result |= PrinterState::PrinterNotAvailable;
        //if(Debuger) qDebug() << "CitizenCTS2000::getStatus(): Printer is not available";
        aStatus = result;
        return true;
    }

    if (offlineError != CMDCitizenCTS2000::PrinterIsOK) {
        if (offlineError & CMDCitizenCTS2000::CoverOpen) {
                //if(Debuger) qDebug() << "Printer cover is open.";
                result |= PrinterState::PrinterError;
        } else if (offlineError & CMDCitizenCTS2000::PrinterError) {
                //if(Debuger) qDebug() << "Offline printer error occured.";
                result |= PrinterState::PrinterError;
        }
    }

    if (printerError != CMDCitizenCTS2000::PrinterIsOK) {
        // Error
        int code = printerError & CMDCitizenCTS2000::PaperJamError;
        if (code > 0) {
            // Paper jam
            result |= PrinterState::PaperJam;
        }

        code = printerError & CMDCitizenCTS2000::UnrecoverableError;

        if (code > 0) {
            // Unrecoverable error
            result |= PrinterState::PrinterError;
            //if(Debuger) qDebug() << "CitizenCTS2000::getStatus(): Unrecoverable error";
        }

        code = printerError & CMDCitizenCTS2000::RecoverableError;
        if (code > 0) {
            // Recoverable error
            result |= PrinterState::PrinterError;
            //if(Debuger) qDebug() << "CitizenCTS2000::getStatus(): Recoverable error";
        }
    }

    if (paperDetectorError != CMDCitizenCTS2000::PrinterIsOK) {
        // Error
        //int code = paperDetectorError & CMDCitizenCTS2000::PaperEnd;
        if (paperDetectorError == CMDCitizenCTS2000::PaperEnd) {
            // Paper end
            result |= PrinterState::PaperEnd;
            //if(Debuger) qDebug() << "CitizenCTS2000::getStatus(): Paper end";
        }

        //code = paperDetectorError & CMDCitizenCTS2000::PaperNearEnd;
        if (paperDetectorError == CMDCitizenCTS2000::PaperNearEnd) {
            // Если в админки включен индикатор толщины рулона
            if(this->counterIndicate){
                // Paper near end
                result |= PrinterState::PaperNearEnd;
            }
            //if(Debuger) qDebug() << "CitizenCTS2000::getStatus(): Paper near end";
        }
    }

    aStatus = result;
    return true;
}

QByteArray CitizenCTS2000_PRINTER::getState()
{
    // засылаем в порт команду получения статуса
    QByteArray cmd;
    bool resp_data = false;
    QByteArray answer, answer_1,answer_2,answer_3;

    cmd.push_back(CMDCitizenCTS2000::PrinterCommandStatusFirstByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandStatusSecondByte);
    cmd.push_back(2);

    if (!this->sendCommand(cmd,true,CMDCitizenCTS2000::TimeOutAfterWriting,resp_data,answer_1,0)) {
        //if(Debuger) qDebug() << "CitizenCTS2000::getState(): error in sendPacketInPort()";
        return answer;
    }

    answer.push_back(answer_1);
    cmd.clear();

    cmd.push_back(CMDCitizenCTS2000::PrinterCommandStatusFirstByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandStatusSecondByte);
    cmd.push_back(3);

    if (!this->sendCommand(cmd,true,CMDCitizenCTS2000::TimeOutAfterWriting,resp_data,answer_2,0)) {
        //if(Debuger) qDebug() << "CitizenCTS2000::getState(): error in sendPacketInPort()";
        return answer;
    }

    answer.push_back(answer_2);
    cmd.clear();

    cmd.push_back(CMDCitizenCTS2000::PrinterCommandStatusFirstByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandStatusSecondByte);
    cmd.push_back(4);

    if (!this->sendCommand(cmd,true,CMDCitizenCTS2000::TimeOutAfterWriting,resp_data,answer_3,0)) {
        //if(Debuger) qDebug() << "CitizenCTS2000::getState(): error in sendPacketInPort()";
        return answer;
    }

    answer.push_back(answer_3);
    return answer;
}

bool CitizenCTS2000_PRINTER::print(const QString& aCheck)
{
    this->initialize();
    this->printCheck(aCheck);
    this->feed(3);
    this->msleep(100);

    return cut();
}

bool CitizenCTS2000_PRINTER::printCheck(const QString& aCheck)
{
    QByteArray printText;
    printText = this->encodingString(aCheck,CScodec::c_IBM866);

    //Вставляем если есть Подчеркнутый, Жирный, Курсивный... Шрифт
    this->getSpecialCharecters(printText);

    QByteArray answer;
    bool respData = false;

    return sendCommand(printText,true,200,respData,answer,50);
}

void CitizenCTS2000_PRINTER::getSpecialCharecters(QByteArray& printText)
{
    QByteArray fontTypeBold_start;
    fontTypeBold_start.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
    fontTypeBold_start.push_back(CMDCitizenCTS2000::PrinterFontBold);
    fontTypeBold_start.push_back(1);

    QByteArray fontTypeBold_end;
    fontTypeBold_end.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
    fontTypeBold_end.push_back(CMDCitizenCTS2000::PrinterFontBold);
    fontTypeBold_end.push_back(48);


    QByteArray fontTypeUnderLine_start;
    fontTypeUnderLine_start.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
    fontTypeUnderLine_start.push_back(CMDCitizenCTS2000::PrinterFontUnderline);
    fontTypeUnderLine_start.push_back(1);

    QByteArray fontTypeUnderLine_end;
    fontTypeUnderLine_end.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
    fontTypeUnderLine_end.push_back(CMDCitizenCTS2000::PrinterFontUnderline);
    fontTypeUnderLine_end.push_back(48);


    QByteArray fontTypeDoubleWidth_start;

    if(smallChek){
        fontTypeDoubleWidth_start.push_back(ASCII::NUL);
    }else{
        fontTypeDoubleWidth_start.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
        fontTypeDoubleWidth_start.push_back(CMDCitizenCTS2000::PrinterFontCommandSecondByte);
        fontTypeDoubleWidth_start.push_back(0x20);
    }

    QByteArray fontTypeDoubleWidth_end;

    if(smallChek){
        fontTypeDoubleWidth_end.push_back(ASCII::NUL);
    }else{

        fontTypeDoubleWidth_end.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
        fontTypeDoubleWidth_end.push_back(CMDCitizenCTS2000::PrinterFontCommandSecondByte);
        fontTypeDoubleWidth_end.push_back(ASCII::NUL);
    }

    QByteArray fontTypeDoubleHeight_start;

    if(smallChek){
        fontTypeDoubleHeight_start.push_back(ASCII::NUL);
    }else{
        fontTypeDoubleHeight_start.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
        fontTypeDoubleHeight_start.push_back(CMDCitizenCTS2000::PrinterFontCommandSecondByte);
        fontTypeDoubleHeight_start.push_back(0x10);
    }

    QByteArray fontTypeDoubleHeight_end;

    if(smallChek){
        fontTypeDoubleHeight_end.push_back(ASCII::NUL);
    }else{
        fontTypeDoubleHeight_end.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
        fontTypeDoubleHeight_end.push_back(CMDCitizenCTS2000::PrinterFontCommandSecondByte);
        fontTypeDoubleHeight_end.push_back(ASCII::NUL);
    }

    //Устанавливаем если есть двойной высоты фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeDoubleHeight + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,fontTypeDoubleHeight_start);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol + CScharsetParam::FontTypeDoubleHeight + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,fontTypeDoubleHeight_end);

    //Устанавливаем если есть двойной ширины фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeDoubleWidth + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,fontTypeDoubleWidth_start);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol + CScharsetParam::FontTypeDoubleWidth + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,fontTypeDoubleWidth_end);

    //Устанавливаем если есть курсивный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol + CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,asciiNull());

    //Устанавливаем если есть жирный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,fontTypeBold_start);

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol + CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,fontTypeBold_end);

    //Устанавливаем если есть подчеркнутый фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeUnderLine + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,fontTypeUnderLine_start);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol + CScharsetParam::FontTypeUnderLine + CScharsetParam::CloseTagDelimiter).toUtf8()
                      ,fontTypeUnderLine_end);

    //Если надо добавить проабел
    QByteArray probel;
    for(int i = 1; i <= leftMargin; i++){
        probel.append(ASCII::Space);
    }
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::ProbelCount + CScharsetParam::CloseTagDelimiter).toUtf8(), probel);

}

bool CitizenCTS2000_PRINTER::initialize()
{
    QByteArray cmd;
    QByteArray answer;
    bool result = false;
    // Проинициализируем принтер
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandInitializeFirstByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandInitializeSecondByte);

    if (!this->sendCommand(cmd,true,CMDCitizenCTS2000::TimeOutAfterWriting,result,answer,0)){
        return false;
    }

    cmd.clear();

    // Установим русскую Code page
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandCodeTableFirstByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandCodeTableSecondByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandCodeTableRussian);

    this->sendCommand(cmd,true,CMDCitizenCTS2000::TimeOutAfterWriting,result,answer,0);

    if(smallChek){
        //Устанавливаем фонт
        cmd.clear();

        cmd.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
        cmd.push_back(CMDCitizenCTS2000::SetFontSecondByte);
        cmd.push_back(CMDCitizenCTS2000::SetFont_1);

        this->sendCommand(cmd,false,50,result,answer,0);
    }

    if(SmallBeetwenString){
        //Inga normalna
        cmd.clear();
        cmd.push_back(CMDCitizenCTS2000::PrinterCommandFirstByte);
        cmd.push_back(CMDCitizenCTS2000::SetFontSecondByteLine);
        cmd.push_back(0x10);
        this->sendCommand(cmd,false,50,result,answer,0);
    }

    if (result) {
//            QByteArray commandPacket;

//            commandPacket.push_back(ASCII::ESC);
//            commandPacket.push_back(static_cast<uchar>(CMDCitizenCTS2000::Modes::Standart));

//            result = this->sendCommand(commandPacket,true,CMDCitizenCTS2000::TimeOutAfterWriting,result,answer,0);
    }

    return result;
}

bool CitizenCTS2000_PRINTER::cut()
{
    QByteArray cmd;

    QByteArray answer;
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandCutFirstByte);
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandCutSecondByte);
    cmd.push_back(1);

    bool result = false;

    return sendCommand(cmd,true,CMDCitizenCTS2000::TimeOutAfterWriting,result,answer,0);
}

bool CitizenCTS2000_PRINTER::feed(int aCount)
{
    QByteArray cmd;
    bool result = false;
    QByteArray answer;
    cmd.push_back(CMDCitizenCTS2000::PrinterCommandFeedByte);

    if(SmallBeetwenString) aCount *= 3;

    for(int i = 0; i < aCount; ++i) {
        cmd.push_back(CMDCitizenCTS2000::PrinterCommandFeedByte);
    }

    this->sendCommand(cmd,true,CMDCitizenCTS2000::TimeOutAfterWriting,result,answer,0);

    return true;
}
