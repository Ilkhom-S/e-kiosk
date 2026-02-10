#include "CitizenPPU700.h"

#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QIODevice>

CitizenPPU700_PRINTER::CitizenPPU700_PRINTER(QObject *parent) : BasePrinterDevices(parent) {
    //    printer_name = "Custom-VKP80";
}

bool CitizenPPU700_PRINTER::OpenPrinterPort() {
    return openPort();
}

bool CitizenPPU700_PRINTER::openPort() {
    if (devicesCreated) {
        is_open = false;

        // Даем девайсу название порта
        serialPort->setPortName(com_Name);

        if (serialPort->open(QIODevice::ReadWrite)) {
            // Если Девайсу удалось открыть порт

            // Устанавливаем параметры открытия порта
            is_open = false;

            if (!serialPort->setDataBits(QSerialPort::Data8)) {
                return false;
            }
            if (!serialPort->setParity(QSerialPort::NoParity)) {
                return false;
            }
            if (!serialPort->setStopBits(QSerialPort::OneStop)) {
                return false;
            }
            if (!serialPort->setFlowControl(QSerialPort::NoFlowControl)) {
                return false;
            }
            if (!serialPort->setBaudRate(QSerialPort::Baud19200)) {
                return false;
            }

            is_open = true;
        } else {
            is_open = false;
        }
    } else {
        is_open = false;
    }

    return is_open;
}

bool CitizenPPU700_PRINTER::isEnabled(CMDCitizenPPU700::SStatus &sStatus, int &status) {
    //    int status = 0;
    if (!this->getStatus(status, sStatus)) {
        return false;
    }

    return (status != PrinterState::PrinterNotAvailable);
}

bool CitizenPPU700_PRINTER::getStatus(int &aStatus, CMDCitizenPPU700::SStatus &sStatus) {
    // смотрим оффлайн
    if (!getState(CMDCitizenPPU700::Constants::Status::Printer, sStatus)) {
        // if(Debugger) qDebug() <<  "Unable to get status, type Printer!";
        return false;
    }

    // если в оффлайне - смотрим причину
    if (sStatus.Offline) {
        if (!getState(CMDCitizenPPU700::Constants::Status::Offline, sStatus)) {
            // if(Debugger) qDebug() << "Unable to get status, type Offline!";
            return false;
        }
    }

    // статус ошибки смотрим всегда
    if (!getState(CMDCitizenPPU700::Constants::Status::Errors, sStatus)) {
        // if(Debugger) qDebug() << "Unable to get status, type Errors!";
        return false;
    }

    if (!getState(CMDCitizenPPU700::Constants::Status::ErrorDetails1, sStatus)) {
        // if(Debugger) qDebug() << "Unable to get status, type ErrorDetails1!";
        return false;
    }

    if (!getState(CMDCitizenPPU700::Constants::Status::ErrorDetails2, sStatus)) {
        // if(Debugger) qDebug() << "Unable to get status, type ErrorDetails2!";
        return false;
    }

    // статус бумаги смотрим только если не в оффлайне из-за конца бумаги
    if (!sStatus.PaperOut) {
        if (!getState(CMDCitizenPPU700::Constants::Status::Paper, sStatus)) {
            // if(Debugger) qDebug() << "Unable to get status, type paper!";
            return false;
        }
    }

    // разбираем статусы
    aStatus = PrinterState::PrinterOK;

    // ошибки
    if (sStatus.NotAvailable) {
        aStatus |= PrinterState::PrinterNotAvailable;
    }

    if (sStatus.Failures.HighVoltage || sStatus.Failures.HighVoltage) {
        aStatus |= PrinterState::PowerSupplyError;
    } else if (sStatus.Failures.Presentor) {
        aStatus |= PrinterState::Mechanism_PositionError;
    } else if (sStatus.Failures.Memory || sStatus.Failures.CPU) {
        aStatus |= PrinterState::ElectronicError;
    } else if (sStatus.Failures.CoverOpen || sStatus.CoverOpen) {
        aStatus |= PrinterState::CoverIsOpened;
    } else if (sStatus.Failures.Cutter) {
        aStatus |= PrinterState::CutterError;
    } else if (sStatus.Failures.Unrecoverable || sStatus.Failures.CRC ||
               sStatus.Failures.DetectionPresenter || sStatus.Error) {
        aStatus |= PrinterState::PrinterError;
    } else if (sStatus.PaperOut || sStatus.Paper.End) {
        aStatus |= PrinterState::PaperEnd;
    } else if (sStatus.Paper.NearEndSensor1 || sStatus.Paper.NearEndSensor2) {
        // Если в админки включен индикатор толщины рулона
        if (this->counterIndicate) {
            // Paper near end
            aStatus |= PrinterState::PaperNearEnd;
        }
    }

    return true;
}

bool CitizenPPU700_PRINTER::getState(char aStatusType, CMDCitizenPPU700::SStatus &aStatus) {
    if (aStatus.NotAvailable) {
        return true;
    }

    QByteArray answerPacket;
    QByteArray commandPacket = QByteArray(1, ASCII::DLE);
    commandPacket.push_back(CMDCitizenPPU700::Commands::Status);
    commandPacket.push_back(aStatusType);
    bool respData = false;
    bool result = true;

    if (this->sendCommand(commandPacket, true, 200, respData, answerPacket, 0)) {
        if (answerPacket.size() != CMDCitizenPPU700::StatusAnswerLength) {
            aStatus.NotAvailable = true;
            return true;
        }

        aStatus.NotAvailable = false;
        char answer = answerPacket.right(1)[0];

        // если ответило другое устройство
        if (!positiveMasking(answer, CMDCitizenPPU700::Control::StatusMask)) {
            aStatus.NotAvailable = true;
            return true;
        }

        switch (aStatusType) {
        case CMDCitizenPPU700::Constants::Status::Printer: {
            aStatus.Offline = getBit(answer, CMDCitizenPPU700::Positions::Answer::Offline);
            break;
        }
        case CMDCitizenPPU700::Constants::Status::Offline: {
            aStatus.CoverOpen = getBit(answer, CMDCitizenPPU700::Positions::Answer::CoverOpen);
            aStatus.PaperOut = getBit(answer, CMDCitizenPPU700::Positions::Answer::PaperOut);
            aStatus.Error = getBit(answer, CMDCitizenPPU700::Positions::Answer::Error);
            break;
        }
        case CMDCitizenPPU700::Constants::Status::Errors: {
            aStatus.Failures.DetectionPresenter =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::Errors::DetectionPresenter);
            aStatus.Failures.Cutter =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::Errors::Cutter);
            aStatus.Failures.Unrecoverable =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::Errors::Unrecoverable);
            break;
        }
        case CMDCitizenPPU700::Constants::Status::Paper: {
            aStatus.Paper.End = getBit(answer, CMDCitizenPPU700::Positions::Answer::Paper::End);
            aStatus.Paper.InPresenter =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::Paper::InPresenter);
            aStatus.Paper.NearEndSensor1 =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::Paper::NearEndSensor1);
            aStatus.Paper.NearEndSensor2 =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::Paper::NearEndSensor2);
            break;
        }
        case CMDCitizenPPU700::Constants::Status::ErrorDetails1: {
            aStatus.Failures.CoverOpen =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails1::CoverOpen);
            aStatus.Failures.HeadOverheat =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails1::HeadOverheat);
            aStatus.Failures.LowVoltage =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails1::LowVoltage);
            aStatus.Failures.HighVoltage =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails1::HighVoltage);
            break;
        }
        case CMDCitizenPPU700::Constants::Status::ErrorDetails2: {
            aStatus.Failures.Memory =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails2::Memory);
            aStatus.Failures.CRC =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails2::CRC);
            aStatus.Failures.Presentor =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails2::Presentor);
            aStatus.Failures.CPU =
                getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails2::CPU);
            break;
        }
        default: {
            result = false;
        }
        }
    } else {
        result = false;
    }

    return result;
}

bool CitizenPPU700_PRINTER::isItYou() {

    QByteArray cmd;
    //    QByteArray data;
    //    QByteArray packet;
    QByteArray answer;
    bool respData = false;
    // Citizen

    // Сначала проверим, что это наша модель принтера
    cmd.push_back(CMDCitizenPPU700::PrinterCommandGetIDFirstByte);
    cmd.push_back(CMDCitizenPPU700::PrinterCommandGetIDSecondByte);
    cmd.push_back(CMDCitizenPPU700::PrinterCommandModelParam); // Узнаем модель принтера

    if (!this->sendCommand(cmd, true, 100, respData, answer, 0)) {
        // if(Debugger) qDebug() << "CitizenPPU700::isItYou(): error in
        // sendPacketInPort()";
        return false;
    }

    bool result = false;

    if (answer.lastIndexOf(CMDCitizenPPU700::PrinterPPU700.toLatin1()) >= 0) {
        if (answer.contains(CMDCitizenPPU700::Response_d::Resp_Mod_Name)) {
            // if(Debugger) qDebug() << "CitizenPPU700::isItYou(): response - " <<
            // CMDCitizenPPU700::Response_d::Resp_Mod_Name;
            result = true;
        }
    } else if (answer.isEmpty()) {
        //            if (this->isEnabled())
        //            {
        //                    result = true;
        //            }
    }

    if (!result) {
    }
    this->closePort();
    return result;
}

void CitizenPPU700_PRINTER::getSpecialCharacters(QByteArray &printText) {
    QByteArray fontTypeBoldStart;
    fontTypeBoldStart.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
    fontTypeBoldStart.push_back(CMDCitizenPPU700::PrinterFontBold);
    fontTypeBoldStart.push_back(1);
    QByteArray fontTypeBoldEnd;
    fontTypeBoldEnd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
    fontTypeBoldEnd.push_back(CMDCitizenPPU700::PrinterFontBold);
    fontTypeBoldEnd.push_back(48);

    QByteArray fontTypeUnderLineStart;
    fontTypeUnderLineStart.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
    fontTypeUnderLineStart.push_back(CMDCitizenPPU700::PrinterFontUnderline);
    fontTypeUnderLineStart.push_back(1);
    QByteArray fontTypeUnderLineEnd;
    fontTypeUnderLineEnd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
    fontTypeUnderLineEnd.push_back(CMDCitizenPPU700::PrinterFontUnderline);
    fontTypeUnderLineEnd.push_back(48);

    QByteArray fontTypeDoubleWidthStart;
    if (smallCheck) {
        fontTypeDoubleWidthStart.push_back(ASCII::NUL);
    } else {
        fontTypeDoubleWidthStart.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
        fontTypeDoubleWidthStart.push_back(CMDCitizenPPU700::PrinterFontCommandSecondByte);
        fontTypeDoubleWidthStart.push_back(0x20);
    }

    QByteArray fontTypeDoubleWidthEnd;
    if (smallCheck) {
        fontTypeDoubleWidthEnd.push_back(ASCII::NUL);
    } else {
        fontTypeDoubleWidthEnd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
        fontTypeDoubleWidthEnd.push_back(CMDCitizenPPU700::PrinterFontCommandSecondByte);
        fontTypeDoubleWidthEnd.push_back(ASCII::NUL);
    }

    QByteArray fontTypeDoubleHeightStart;
    if (smallCheck) {
        fontTypeDoubleHeightStart.push_back(ASCII::NUL);
    } else {
        fontTypeDoubleHeightStart.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
        fontTypeDoubleHeightStart.push_back(CMDCitizenPPU700::PrinterFontCommandSecondByte);
        fontTypeDoubleHeightStart.push_back(0x10);
    }

    QByteArray fontTypeDoubleHeightEnd;
    if (smallCheck) {
        fontTypeDoubleHeightEnd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
        fontTypeDoubleHeightEnd.push_back(CMDCitizenPPU700::PrinterFontCommandSecondByte);
        fontTypeDoubleHeightEnd.push_back(ASCII::NUL);
    } else {
        fontTypeDoubleHeightEnd.push_back(ASCII::NUL);
    }

    // Устанавливаем если есть двойной высоты фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                              CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeDoubleHeightStart);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeDoubleHeightEnd);

    // Устанавливаем если есть двойной ширины фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                              CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeDoubleWidthStart);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeDoubleWidthEnd);

    // Устанавливаем если есть курсивный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeItalic +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть жирный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeBold +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeBoldStart);

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeBoldEnd);

    // Устанавливаем если есть подчеркнутый фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeUnderLine +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeUnderLineStart);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeUnderLine + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeUnderLineEnd);

    // Если надо добавить проабел
    QByteArray probel;
    for (int i = 1; i <= leftMargin; i++) {
        probel.append(ASCII::Space);
    }
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::SpaceCount +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      probel);
}

bool CitizenPPU700_PRINTER::printCheck(const QString &aCheck) {
    // Меняем кодировку
    QByteArray printText;
    printText = this->encodingString(aCheck, CScodec::c_IBM866);

    // Вставляем если есть Подчеркнутый, Жирный, Курсивный... Шрифт
    this->getSpecialCharacters(printText);

    //    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    //    // Проинициализируем принтер
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandInitSecondByte);

    //    if(!this->sendCommand(cmd,false,0,respData,answer,0))
    //            return false;

    //    cmd.clear();
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandPaperSizeSecondByte);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandPaperSizeThirdByteSmall);
    //    cmd.push_back(0x0A);

    return this->sendCommand(printText, true, 200, respData, answer, 50);
}

QString CitizenPPU700_PRINTER::getImage(QString fileName) {

    QImage image(fileName);
    qDebug() << "image.byteCount(); - " << image.sizeInBytes();
    QByteArray ba;

    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);
    image.save(&buffer, "BMP"); // writes image into ba in PNG format
    qDebug() << "buffer.bytesAvailable(); - " << buffer.bytesAvailable();
    qDebug() << "ba.size(); - " << ba.size();
    ////    QByteArray compressed = qCompress(ba, 1);

    QFile vrmFile("img_cont.txt");
    if (vrmFile.open(QFile::WriteOnly)) {
        vrmFile.write(ba);
        vrmFile.close();
    }

    QString data = "";
    if (vrmFile.open(QFile::ReadOnly)) {
        data = vrmFile.readAll();
        vrmFile.close();
    }

    buffer.close();
    return data;
}

bool CitizenPPU700_PRINTER::print(const QString &aCheck) {
    // Инициализация
    this->initialize();
    // Картинка
    //      if(viewLogoImg) this->printImage();

    //     QString content = getImage("2.bmp");
    //     qDebug() << content;
    //     this->printImageI(content,255,true);
    // Печатаем текст
    this->printCheck(aCheck);

    if (!feed(3)) {
        return false;
    }

    // Обрезка
    this->cut();

    // Прокрутка
    //      this->dispense();
    return true;
}

void CitizenPPU700_PRINTER::dispense() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    // Dispense
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandClrDispenserSecondByte);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandDispenseThirdByte);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandDispenseForthByte);

    if (!this->sendCommand(cmd, false, 0, respData, answer, 50)) {
        return;
    }
}

bool CitizenPPU700_PRINTER::initialize() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    // Проинициализируем принтер

    cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
    cmd.push_back(CMDCitizenPPU700::PrinterCommandInitSecondByte);

    if (!this->sendCommand(cmd, false, 0, respData, answer, 0)) {
        return false;
    }

    cmd.clear();

    // Установим русскую Code page
    cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
    cmd.push_back(CMDCitizenPPU700::PrinterCommandSetCodePageSecondByte);
    cmd.push_back(CMDCitizenPPU700::PrinterCommandSetCodePageThirdByte);

    bool result = this->sendCommand(cmd, true, 50, respData, answer, 0);

    if (smallCheck) {
        // Устанавливаем фонт
        cmd.clear();

        cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
        cmd.push_back(CMDCitizenPPU700::SetFontSecondByte);
        cmd.push_back(CMDCitizenPPU700::SetFont_1);

        this->sendCommand(cmd, false, 50, respData, answer, 0);
    }

    if (SmallBetweenString) {
        // Inga normalna
        cmd.clear();
        cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
        cmd.push_back(CMDCitizenPPU700::SetFontSecondByteLine);
        cmd.push_back(0x08);
        this->sendCommand(cmd, false, 50, respData, answer, 0);
    }

    cmd.clear();
    cmd.push_back(CMDCitizenPPU700::PrinterCommandGetIDFirstByte);
    cmd.push_back(0x52);
    cmd.push_back(0x31);
    cmd.push_back(CMDCitizenPPU700::SetFont_1);

    this->sendCommand(cmd, false, 50, respData, answer, 0);

    return result;
}

bool CitizenPPU700_PRINTER::cut() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    //    cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
    //    cmd.push_back(CMDCitizenPPU700::PrinterCommandCutSecondByte);

    cmd.push_back(0x1D);
    cmd.push_back(0x56);
    cmd.push_back(0x01);

    return this->sendCommand(cmd, true, 50, respData, answer, 0);
}

bool CitizenPPU700_PRINTER::feed(int aCount) {

    QByteArray cmd;
    bool respData = false;
    QByteArray answer;
    cmd.push_back(CMDCitizenPPU700::PrinterCommandFeedByte);
    cmd.push_back(0x0D);

    for (int i = 0; i < aCount; ++i) {
        if (!this->sendCommand(cmd, true, 50, respData, answer, 0)) {
            return false;
        }
    }

    return true;
}

bool CitizenPPU700_PRINTER::printImage() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandLogoPrintSecondByte);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandCharacterSetThirdByte);
    //    cmd.push_back(ASCII::NUL);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandLogoPrintThirdByte);
    //    cmd.push_back(ASCII::NUL);
    //    cmd.push_back(CMDCustom_VKP80::PrinterCommandLogoPrintFothByte);

    return this->sendCommand(cmd, false, 0, respData, answer, 50);
}

bool CitizenPPU700_PRINTER::printImageI(const QString &aPixelString,
                                        uchar aWidth,
                                        bool aNeedRegisterLogo) {
    QByteArray cmd;
    QByteArray respData;
    bool respOk = false;

    if (aNeedRegisterLogo) {
        if (!registerLogo(aPixelString, aWidth)) {
            return false;
        }
    }
    //        cmd.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    //        cmd.push_back(0x2F);
    //        cmd.push_back(48);
    //        uchar nil = 0;
    //        cmd.append(CMDCustom_VKP80::PrinterCommandFirstByte);
    //        cmd.append(CMDCustom_VKP80::PrinterCommandAnotherFeedSecondByte);
    //        cmd.append(nil);

    return this->sendCommand(cmd, true, 100, respOk, respData, 0);
}

bool CitizenPPU700_PRINTER::registerLogo(const QString &aPixelString, uchar aWidth) {
    Q_UNUSED(aPixelString)

    if (aWidth == 0U) {
        return true;
    }

    QByteArray cmd;
    QByteArray response;
    bool respData = false;

    //    uchar height = 0;
    //    if (aWidth){
    //        height = (uchar)(aPixelString.size() / aWidth);
    //    }

    //    uchar verticalSize = height / 8; // Размер по вертикали, который мы
    //    передадим принтеру if (height % 8) {
    //        verticalSize++;
    //    }

    //        cmd.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    //        cmd.push_back(CMDCustom_VKP80::PrinterCommandLogoRegSecondByte);
    //        cmd.push_back(aWidth);
    //        cmd.push_back(verticalSize);
    ////        QString str(aPixelString);
    //        QByteArray imageData = packetImage(aPixelString, aWidth);
    ////        QByteArray imageData = aPixelString.toHex();
    ////        //if(Debugger) qDebug() << "this->printDataToHex(imageData);";
    //        //if(Debugger) qDebug() << "aPixelString.size()" <<
    //        aPixelString.size();
    ////        this->printDataToHex(imageData);
    //        cmd.append(imageData);

    return sendCommand(cmd, true, 3000, respData, response, 3000);
}
