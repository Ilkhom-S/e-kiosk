#include "CustomVKP80.h"

#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QIODevice>

Custom_VKP80_PRINTER::Custom_VKP80_PRINTER(QObject *parent) : BasePrinterDevices(parent) {
    //    printer_name = "Custom-VKP80";
}

bool Custom_VKP80_PRINTER::OpenPrinterPort() {
    this->openPort();

    return is_open;
}

bool Custom_VKP80_PRINTER::openPort() {
    if (devicesCreated) {
        // Если девайс для работы с портом обявлен
        is_open = false;

        // Даем девайсу название порта
        serialPort->setPortName(com_Name);

        if (serialPort->open(QIODevice::ReadWrite)) {
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

bool Custom_VKP80_PRINTER::isEnabled(CMDCustom_VKP80::SStatus &s_status, int &state) {
    if (!this->getStatus(state, s_status)) {
        return false;
    }

    return (state != PrinterState::PrinterNotAvailable);
}

bool Custom_VKP80_PRINTER::getStatus(int &aStatus, CMDCustom_VKP80::SStatus &s_status) {
    // смотрим оффлайн
    if (!getState(CMDCustom_VKP80::Constants::Status::Printer, s_status)) {
        // if(Debugger) qDebug() << "Unable to get status, type Printer!";
        return false;
    }

    // если в оффлайне - смотрим причину
    if (s_status.Offline) {
        if (!getState(CMDCustom_VKP80::Constants::Status::Offline, s_status)) {
            // if(Debugger) qDebug() <<  "Unable to get status, type Offline!";
            return false;
        }
    }

    // статус ошибки смотрим всегда
    if (!getState(CMDCustom_VKP80::Constants::Status::Errors, s_status)) {
        // if(Debugger) qDebug() << "Unable to get status, type Errors!";
        return false;
    }

    if (!getState(CMDCustom_VKP80::Constants::Status::Printing, s_status)) {
        // if(Debugger) qDebug() << "Unable to get status, type Printing!";
        return false;
    }

    // статус бумаги смотрим только если не в оффлайне из-за конца бумаги
    if (!s_status.PaperOut) {
        if (!getState(CMDCustom_VKP80::Constants::Status::Paper, s_status)) {
            // if(Debugger) qDebug() << "Unable to get status, type Paper!";
            return false;
        }
    }

    // разбираем статусы
    aStatus = PrinterState::PrinterOK;

    // ошибки
    if (s_status.NotAvailabled) {
        aStatus |= PrinterState::PrinterNotAvailable;
    } else if (s_status.CoverOpen) {
        aStatus |= PrinterState::CoverIsOpened;
    } else if (s_status.Failures.Cutter) {
        aStatus |= PrinterState::CutterError;
    } else if (s_status.PaperOut || s_status.Paper.End || s_status.Printing.PaperOff) {
        aStatus |= PrinterState::PaperEnd;
    } else if (s_status.Failures.Unrecoverable || s_status.Error) {
        aStatus |= PrinterState::PrinterError;
    }

    /*
    //TODO: похоже, этот статус нам не нужен. Надо прояснить этот вопрос.
    // Off - означает, что мотор не работает или что он неживой?
    else if (status.Printing.MotorOff)
    {
                    aStatus |= PrinterState::PrinterError;
    }
    */
    /*
    // ворнинги
    // не обрабатываем этот статус, т.к. если у кастома нет ноги (а на нем -
    датчика),
    // то принтер всегда будет слать данный статус*/
    //    if (s_status.Paper.NearEnd)
    //    {
    //         // Если в админки включен индикатор толщины рулона
    //         if(this->counterIndicate){
    //            // Paper near end
    //            aStatus |= PrinterState::PaperNearEnd;
    //         }
    //    }

    qDebug() << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&";
    qDebug() << QString("aStatus - %1").arg(QString::number(aStatus));
    return true;
}

bool Custom_VKP80_PRINTER::getState(char aStatusType, CMDCustom_VKP80::SStatus &aStatus) {
    qDebug() << "----------inter to get state---------";

    if (aStatus.NotAvailabled) {
        qDebug() << "----------aStatus.NotAvailabled---------";
        return true;
    }

    bool respData = false;
    QByteArray answerPacket;
    QByteArray commandPacket = QByteArray(1, ASCII::DLE);
    commandPacket.push_back(CMDCustom_VKP80::Commands::Status);
    commandPacket.push_back(aStatusType);

    bool result = true;

    if (this->sendCommand(commandPacket, true, 50, respData, answerPacket, 0)) {
        if (answerPacket.size() != CMDCustom_VKP80::AnswersLength::Status) {
            qDebug() << "==========aStatus.NotAvailabled = true;======";
            aStatus.NotAvailabled = true;
            return true;
        }

        aStatus.NotAvailabled = false;
        char answer = answerPacket.right(1)[0];

        // если ответило другое устройство
        if (!(positiveMasking(answer, CMDCustom_VKP80::Control::StatusMask1) &&
              negativeMasking(answer, CMDCustom_VKP80::Control::StatusMask0))) {
            aStatus.NotAvailabled = true;
            return true;
        }

        switch (aStatusType) {
        case CMDCustom_VKP80::Constants::Status::Printer: {
            aStatus.Offline = getBit(answer, CMDCustom_VKP80::Positions::Answer::Offline);
            break;
        }
        case CMDCustom_VKP80::Constants::Status::Offline: {
            aStatus.CoverOpen = getBit(answer, CMDCustom_VKP80::Positions::Answer::CoverOpen);
            aStatus.PaperOut = getBit(answer, CMDCustom_VKP80::Positions::Answer::PaperOut);
            aStatus.Error = getBit(answer, CMDCustom_VKP80::Positions::Answer::Error);
            break;
        }
        case CMDCustom_VKP80::Constants::Status::Errors: {
            aStatus.Failures.Cutter =
                getBit(answer, CMDCustom_VKP80::Positions::Answer::Failures::Cutter);
            aStatus.Failures.Unrecoverable =
                getBit(answer, CMDCustom_VKP80::Positions::Answer::Failures::Unrecoverable);
            aStatus.Failures.AutoRecovery =
                getBit(answer, CMDCustom_VKP80::Positions::Answer::Failures::AutoRecovery);
            break;
        }
        case CMDCustom_VKP80::Constants::Status::Paper: {
            aStatus.Paper.End = getBit(answer, CMDCustom_VKP80::Positions::Answer::Paper::End);
            aStatus.Paper.NearEnd =
                getBit(answer, CMDCustom_VKP80::Positions::Answer::Paper::NearEnd);
            break;
        }
        case CMDCustom_VKP80::Constants::Status::Printing: {
            aStatus.Printing.MotorOff =
                getBit(answer, CMDCustom_VKP80::Positions::Answer::Printing::MotorOff);
            aStatus.Printing.PaperOff =
                getBit(answer, CMDCustom_VKP80::Positions::Answer::Printing::PaperOff);
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

bool Custom_VKP80_PRINTER::isItYou() {
    CMDCustom_VKP80::SControlInfo controlInfo;

    if (!getControlInfo(CMDCustom_VKP80::Constants::ControlInfo::ModelID, controlInfo)) {
        // if(Debugger) qDebug() << "Unable to get control info, type Model ID!";
    }

    if (!getControlInfo(CMDCustom_VKP80::Constants::ControlInfo::Features, controlInfo)) {
        // if(Debugger) qDebug() << "Unable to get control info, type Features!";
    }

    if (!getControlInfo(CMDCustom_VKP80::Constants::ControlInfo::ROM, controlInfo)) {
        // if(Debugger) qDebug() << "Unable to get control info, type ROM version!";
    }

    bool result = false;

    // может не ответить, если находится в оффлайне
    if (!controlInfo.wrongAnswer) {
        if (!controlInfo.noAnswer) {
            /*                         if(Debugger) qDebug() << QString("Control info:\
                                                            \nResolution = %1,\
                                                            \nFeatures: Unicode
               symbols is %2supported, Autocutter is %3supplied\
                                                            \nROM version = %4")
                                                                            .arg(controlInfo.Resolution)
                                                                            .arg(controlInfo.isUnicodeSupported
               ? "" : "not ") .arg(controlInfo.isAutoCutterSupplied ? "" : "not ")
                                                                            .arg(controlInfo.ROMVersion);
            */
            comment = controlInfo.ROMVersion;
            result = true;
        }
    }

    this->closePort();

    return result;
}

bool Custom_VKP80_PRINTER::getControlInfo(char aInfoType,
                                          CMDCustom_VKP80::SControlInfo &aControlInfo) {
    if (aControlInfo.noAnswer || aControlInfo.wrongAnswer) {
        return true;
    }

    bool resp = false;
    QByteArray answerPacket;
    QByteArray commandPacket = QByteArray(1, ASCII::GS);
    commandPacket.push_back(CMDCustom_VKP80::Commands::GetControlInfo);
    commandPacket.push_back(aInfoType);

    if (this->sendCommand(commandPacket, true, 200, resp, answerPacket, 0)) {
        if (answerPacket.isEmpty()) {
            aControlInfo.noAnswer = true;
            return true;
        }

        switch (aInfoType) {
        case CMDCustom_VKP80::Constants::ControlInfo::ModelID: {
            if (answerPacket.size() == CMDCustom_VKP80::AnswersLength::ControlInfo::ModelID) {
                if (answerPacket[0] == CMDCustom_VKP80::Constants::ModelIDs::PrinterID200Dpi) {
                    aControlInfo.Resolution = 200;
                } else if (answerPacket[0] ==
                           CMDCustom_VKP80::Constants::ModelIDs::PrinterID300Dpi) {
                    aControlInfo.Resolution = 300;
                } else {
                    aControlInfo.wrongAnswer = true;
                }
            } else {
                aControlInfo.wrongAnswer = true;
            }

            if (aControlInfo.wrongAnswer) {

                // if(Debugger) qDebug() << "getControlInfo, request modelID: wrong
                // answer from printer";
            }

            break;
        }

        case CMDCustom_VKP80::Constants::ControlInfo::Features: {
            if (answerPacket.size() == CMDCustom_VKP80::AnswersLength::ControlInfo::Features) {
                aControlInfo.isUnicodeSupported =
                    getBit(answerPacket[0],
                           CMDCustom_VKP80::Positions::Answer::GetFeaturesInfo::UnicodeSupported);
                aControlInfo.isAutoCutterSupplied =
                    getBit(answerPacket[0],
                           CMDCustom_VKP80::Positions::Answer::GetFeaturesInfo::AutoCutterSupplied);
            } else {
                // if(Debugger) qDebug() << "getControlInfo, request Features: wrong
                // answer from printer";
                aControlInfo.wrongAnswer = true;
            }
            break;
        }

        case CMDCustom_VKP80::Constants::ControlInfo::ROM: {
            if (answerPacket.size() == CMDCustom_VKP80::AnswersLength::ControlInfo::ROM) {
                aControlInfo.ROMVersion = answerPacket;
            } else {
                // if(Debugger) qDebug() << "getControlInfo, request ROM version: wrong
                // answer from printer";
                aControlInfo.wrongAnswer = true;
            }
            break;
        }
        default: {
            // if(Debugger) qDebug() << QString("getControlInfo, unknown request = 0x
            // %1").arg(QString(QByteArray(1, aInfoType).toHex()));
            aControlInfo.wrongAnswer = true;
        }
        }
    } else {
        return false;
    }

    return true;
}

void Custom_VKP80_PRINTER::getSpecialCharacters(QByteArray &printText) {

    QByteArray fontTypeBold_start;
    fontTypeBold_start.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    fontTypeBold_start.push_back(CMDCustom_VKP80::PrinterFontBoldSecondByte);
    fontTypeBold_start.push_back(1);

    QByteArray fontTypeBold_end;
    fontTypeBold_end.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    fontTypeBold_end.push_back(CMDCustom_VKP80::PrinterFontBoldSecondByte);
    fontTypeBold_end.push_back(48);

    QByteArray fontTypeDoubleHeight_start;
    fontTypeDoubleHeight_start.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    fontTypeDoubleHeight_start.push_back(CMDCustom_VKP80::PrinterFontDoubleWidthHeight);
    fontTypeDoubleHeight_start.push_back(1);

    QByteArray fontTypeDoubleHeight_end;
    fontTypeDoubleHeight_end.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    fontTypeDoubleHeight_end.push_back(CMDCustom_VKP80::PrinterFontDoubleWidthHeight);
    fontTypeDoubleHeight_end.push_back(ASCII::NUL);

    QByteArray fontTypeDoubleWidth_start;
    fontTypeDoubleWidth_start.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    fontTypeDoubleWidth_start.push_back(CMDCustom_VKP80::PrinterFontDoubleWidthHeight);
    fontTypeDoubleWidth_start.push_back(0x10);

    QByteArray fontTypeDoubleWidth_end;
    fontTypeDoubleWidth_end.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    fontTypeDoubleWidth_end.push_back(CMDCustom_VKP80::PrinterFontDoubleWidthHeight);
    fontTypeDoubleWidth_end.push_back(ASCII::NUL);

    QByteArray fontTypeUnderLine_start;
    fontTypeUnderLine_start.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    fontTypeUnderLine_start.push_back(CMDCustom_VKP80::PrinterFontUnderline);
    fontTypeUnderLine_start.push_back(1);

    QByteArray fontTypeUnderLine_end;
    fontTypeUnderLine_end.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    fontTypeUnderLine_end.push_back(CMDCustom_VKP80::PrinterFontUnderline);
    fontTypeUnderLine_end.push_back(48);

    QByteArray fontTypeItalic_start;
    fontTypeItalic_start.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    fontTypeItalic_start.push_back(CMDCustom_VKP80::PrinterFontItalic);
    fontTypeItalic_start.push_back(1);

    QByteArray fontTypeItalic_end;
    fontTypeItalic_end.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    fontTypeItalic_end.push_back(CMDCustom_VKP80::PrinterFontItalic);
    fontTypeItalic_end.push_back(48);

    // Устанавливаем если есть жирный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeBold +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeBold_start);

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeBold_end);

    // Устанавливаем если есть двойной высоты фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                              CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeDoubleHeight_start);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeDoubleHeight_end);

    // Устанавливаем если есть двойной ширины фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                              CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeDoubleWidth_start);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeDoubleWidth_end);

    // Устанавливаем если есть курсивный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeItalic +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeItalic_start);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeItalic_end);

    // Устанавливаем если есть подчеркнутый фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeUnderLine +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeUnderLine_start);
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeUnderLine + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      fontTypeUnderLine_end);

    // Если надо добавить пробел
    QByteArray space;
    for (int i = 1; i <= leftMargin; i++) {
        space.append(ASCII::Space);
    }
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::SpaceCount +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      space);

    // Добавляем звезды
    int col_z = (checkWidth - 11 - leftMargin) / 2;
    QByteArray star;
    for (int j = 1; j <= col_z; j++) {
        star.append("*");
    }

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::StarCount +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      star);
}

bool Custom_VKP80_PRINTER::printCheck(const QString &aCheck) {
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

    if (!this->sendCommand(printText, true, 200, respData, answer, 50)) {
        return false;
    }

    return true;
}

QByteArray Custom_VKP80_PRINTER::getImage(QString fileName) {
    QImage image(fileName);

    QByteArray ba;

    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);
    image.save(&buffer, "BMP"); // writes image into ba in PNG format

    //    QByteArray balet = ba.toHex();

    return ba;
}

void Custom_VKP80_PRINTER::print(const QString &aCheck) {
    //    cmd.push_back(ASCII::NUL);
    //    cmd.push_back(0x0D);
    //    cmd.push_back(0x01);

    //    answer.clear();
    //    cmd.clear();
    //    cmd.push_back(0x1C);
    //    cmd.push_back(0x24);
    //    cmd.push_back(0x52);

    //    this->sendCommand(cmd,true,50,respData,answer,50);

    //    answer.clear();
    //    cmd.clear();
    //    cmd.push_back(0x1C);
    //    cmd.push_back(0x24);
    //    cmd.push_back(0x57);
    //    cmd.push_back(0x7F);
    //    cmd.push_back(0x74);
    //    cmd.push_back(0x90);
    //    cmd.push_back(0x7F);

    //    this->sendCommand(cmd,true,50,respData,answer,50);

    // Инициализация
    this->initialize();

    // Картинка
    if (viewLogoImg) {
        this->printImage();
    }

    //     QString content = getImage("2.bmp");
    //          QString content/* = getImage("2.bmp")*/;
    //     qDebug() << content;
    //     this->printImageI(content,8,true);
    // Печатаем текст
    this->printCheck(aCheck);

    // Fiscalization

    //     if(viewLogoImg){
    //         this->sendFiscalData();

    //         QString a_aCheck = "[dw]Ф[/dw]\n";
    //         this->printCheck(a_aCheck);
    //     }

    // Обрезка
    this->cut();

    // Прокрутка
    this->dispense();
}

void Custom_VKP80_PRINTER::dispense() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    // Dispense
    cmd.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandClrDispenserSecondByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandDispenseThirdByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandDispenseForthByte);

    if (!this->sendCommand(cmd, false, 0, respData, answer, 50)) {
        return;
    }
}

bool Custom_VKP80_PRINTER::initialize() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    // Проинициализируем принтер
    cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandInitSecondByte);

    if (!this->sendCommand(cmd, false, 0, respData, answer, 0)) {
        return false;
    }

    // Установим размер чека
    if (smallCheck) {
        // Укороченный чек
        // if(Debugger) qDebug() << QString("small check true");

        cmd.clear();
        cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
        cmd.push_back(CMDCustom_VKP80::PrinterCommandPaperSizeSecondByte);
        cmd.push_back(CMDCustom_VKP80::PrinterCommandPaperSizeThirdByteSmall);

        if (!this->sendCommand(cmd, false, 0, respData, answer, 0)) {
            return false;
        }
    }

    // Установим русскую Code page
    cmd.clear();
    cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandSetCodePageSecondByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandSetCodePageThirdByte);

    if (!this->sendCommand(cmd, false, 0, respData, answer, 0)) {
        return false;
    }

    // Установим international character set
    cmd.clear();
    cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandCharacterSetSecondByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandCharacterSetThirdByte);

    if (!this->sendCommand(cmd, false, 0, respData, answer, 0)) {
        return false;
    }

    return true;
}

void Custom_VKP80_PRINTER::sendFiscalData() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = true;

    //     Fiscal Mode
    cmd.push_back(0x1D);
    cmd.push_back(0x42);
    cmd.push_back(0x01);
    //    cmd.append("F");

    this->sendCommand(cmd, true, 50, respData, answer, 50);
}

bool Custom_VKP80_PRINTER::cut() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandCutSecondByte);
    //    cmd.push_back(CMDCustom_VKP80::PaperEnd);

    if (!this->sendCommand(cmd, false, 0, respData, answer, 0)) {
        return false;
    }

    return true;
}

bool Custom_VKP80_PRINTER::feed(int aCount) {
    Q_UNUSED(aCount)
    return true;
}

bool Custom_VKP80_PRINTER::printImage() {
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    cmd.push_back(CMDCustom_VKP80::PrinterCommandFirstByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandLogoPrintSecondByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandCharacterSetThirdByte);
    cmd.push_back(ASCII::NUL);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandLogoPrintThirdByte);
    cmd.push_back(ASCII::NUL);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandLogoPrintFothByte);

    if (!this->sendCommand(cmd, false, 0, respData, answer, 50)) {
        return false;
    }

    return true;
}

bool Custom_VKP80_PRINTER::printImageI(const QString &aPixelString,
                                       uchar aWidth,
                                       bool aNeedRegisterLogo) {
    QByteArray cmd;
    QByteArray resp_data;
    bool resp_ok = false;

    if (aNeedRegisterLogo) {
        if (!registerLogo(aPixelString, aWidth)) {
            return false;
        }
    }

    cmd.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    cmd.push_back(0x2F);
    cmd.push_back(48);
    uchar nil = 0;
    cmd.append(CMDCustom_VKP80::PrinterCommandFirstByte);
    cmd.append(CMDCustom_VKP80::PrinterCommandAnotherFeedSecondByte);
    cmd.append(nil);

    return this->sendCommand(cmd, true, 100, resp_ok, resp_data, 0);
}

bool Custom_VKP80_PRINTER::registerLogo(const QString &aPixelString, uchar aWidth) {
    Q_UNUSED(aPixelString)

    if (!aWidth) {
        return true;
    }

    QByteArray cmd;
    QByteArray response;
    bool resp_data = false;

    QByteArray imageData /* = packetImage(aPixelString, aWidth)*/;
    imageData = this->getImage("2.bmp");

    //    uchar height = 0;
    //    if (aWidth) {
    //        height = (uchar)(imageData.size() / aWidth);
    //    }

    //        auto verticalSize = height / 8; // Размер по вертикали, который мы
    //        передадим принтеру if (height % 8) {
    //            verticalSize++;
    //        }

    cmd.push_back(CMDCustom_VKP80::PrinterCommandGetIDFirstByte);
    cmd.push_back(CMDCustom_VKP80::PrinterCommandLogoRegSecondByte);
    //        cmd.push_back(aWidth);
    //        cmd.push_back(verticalSize);
    //        QString str(aPixelString);
    cmd.push_back(8);
    cmd.push_back(8);

    // if(Debugger) qDebug() << "aPixelString.size()" << aPixelString.size();
    //        this->printDataToHex(imageData);
    cmd.append(imageData);
    bool res = this->sendCommand(cmd, true, 3000, resp_data, response, 3000);
    return res;
}
