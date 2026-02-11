#include "CCNetSM.h"

#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/qendian.h>

#include <algorithm>

#include "CCNetFirmware.h"
#include "CashPayment.h"

using SendFirm_WareDataByPathFunc = int (*)(int, char *);
using GetDataStatusFunc = long (*)();

CCNetSm::CCNetSm(QObject *parent)
    : BaseValidatorDevices(parent), validatorLogEnable(false), maxSum_Reject(false),
      hasDBError(false), escrowed(false), firmwareUpdating(false), nominalSum(0) {
    preDateTime = QDateTime::currentDateTime().addSecs(-1);
}

bool CCNetSm::OpenPort() {
    this->openPort();
    return is_open;
}

void CCNetSm::sendStatusTo(int sts, QString comment) {
    //    qDebug() << "CCNetSm - STS - " << sts;
    if (sts != status) {
        //        qDebug() << "CCNetSm - STS - " << sts;
        status = sts;

        emit this->emitStatus(status, comment);
    }
}

bool CCNetSm::openPort() {
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
            //            if
            //            (!serialPort->setCharIntervalTimeout(ValidatorConstants::CCNetSm_CharTimeOut))
            //            return false;
            if (!serialPort->setBaudRate(QSerialPort::Baud9600)) {
                return false;
            }

            is_open = true;
        } else {
            is_open = false;
            statusDevices = ValidatorErrors::PortError;
            // if(Debugger) qDebug() << "Error opened serial device " <<
            // devicePort->deviceName();
        }
    } else {
        is_open = false;
        // if(Debugger) qDebug() << "Error create serial device " <<
        // devicePort->deviceName();
        statusDevices = ValidatorErrors::DevicesError;
    }

    return is_open;
}

bool CCNetSm::isItYou() {

    this->OpenPort();
    if (this->isOpened()) {
        QByteArray respData;

        this->execCommand(ValidatorCommands::Poll, respData);

        if (respData[0] == '\x02' && respData[1] == '\x03' && respData[2] == '\x06' &&
            respData[5] != '\x81') {
            this->execCommand(ValidatorCommands::Idintification, respData);

            this->ParsIdentification(respData);
            return true;
        }
        if (checkBootloader()) {
            PartNumber = "BOOTLDR";
            SerialNumber = "";
            return true;
        }

        this->closePort();
        return false;

    } 
        return false;
   

    return false;
}

bool CCNetSm::CmdGetStatus() {

    QByteArray respData;

    this->execCommand(ValidatorCommands::Poll, respData);

    if (respData[3] == CCNetConstruct::States::PowerUp ||
        respData[3] == CCNetConstruct::States::PowerUpInValidator ||
        respData[3] == CCNetConstruct::States::PowerUpInStacker) {
        this->CmdRestart();

        this->execCommand(ValidatorCommands::Poll, respData);
    }

    if (respData[3] == CCNetConstruct::States::Escrow) {
        this->execCommand(ValidatorCommands::Stack, respData);
        this->execCommand(ValidatorCommands::Poll, respData);
    }

    int nominal = 0;
    nominal = this->readPollInfo(respData);

    if (nominal > 0) {
        emit this->emitNominal(nominal);
    }

    QCoreApplication::processEvents();
    return true;
}

ushort CCNetSm::calcCRC16(const QByteArray &aData) {
    ushort crc = 0;

    for (char i : aData) {
        ushort byteCRC = 0;
        ushort value = uchar(crc ^ i);

        for (int j = 0; j < 8; ++j) {
            ushort data = byteCRC >> 1;
            byteCRC = (((byteCRC ^ value) & 1) != 0) ? (data ^ 0x8408) : data;
            value = value >> 1;
        }

        crc = byteCRC ^ (crc >> 8);
    }

    return crc;
}

QByteArray CCNetSm::makeCustom_Request(int adr, int cmd, const QByteArray &data) {

    QByteArray request;

    request.append(CCNetConstruct::Sync);
    request.append(adr);
    request.append('\0');
    request.append(cmd);
    request.append(data);

    request[2] = request.size() + 2;

    ushort crc = calcCRC16(request);
    request.append(uchar(crc));
    request.append(uchar(crc >> 8));

    return request;
}

bool CCNetSm::execCommand(ValidatorCommands::Enum cmdType, QByteArray &cmdResponse) {
    try {
        if (is_open) {

            QByteArray cmdRequest;
            QByteArray btmData;

            switch (cmdType) {

            case ValidatorCommands::Reset:
                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCReset, nullptr);
                break;

            case ValidatorCommands::GetNominalTable:
                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCGetBillTable, nullptr);
                break;

            case ValidatorCommands::SetEnabled: {
                btmData.clear();
                btmData.resize(6);

                btmData[0] = '\xFF';
                btmData[1] = '\xFF';
                btmData[2] = '\xFF';

                btmData[3] = '\xFF';
                btmData[4] = '\xFF';
                btmData[5] = '\xFF';

                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCEnableBillTypes, btmData);
            } break;

            case ValidatorCommands::Poll:
                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCPoll, nullptr);
                break;

            case ValidatorCommands::Idintification:
                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCIdentification, nullptr);
                break;

            case ValidatorCommands::SetSecurity: {
                btmData.clear();
                btmData.resize(6);

                for (int i = 0; i < 3; i++) {
                    btmData[i] = '\xFF';
                }

                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCSetSecurity, btmData);
            } break;

            case ValidatorCommands::ACK:
                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCAck, nullptr);
                break;

            case ValidatorCommands::SetDisabled: {
                btmData.clear();
                btmData.resize(6);
                for (int i = 0; i < 6; i++) {
                    btmData[i] = 0x00;
                }

                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCEnableBillTypes, btmData);
            } break;

            case ValidatorCommands::Return:
                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCReturn, nullptr);
                break;

            case ValidatorCommands::Stack:
                cmdRequest = this->makeCustom_Request(
                    CCNetConstruct::PABillValidator, CCNetConstruct::CCStack, nullptr);
                break;

            default:
                break;
            }

            auto cmd = cmdName(cmdType);
            toValidatorLog(0, cmdRequest, cmd);

            QByteArray answer;

            TResult result = processCommand(cmdRequest, answer);

            if (result == CommandResult::OK) {
                cmdResponse = answer;
                return true;
            }

            if (result == 0u) {
                qDebug() << "result " << result;
            }

            if (result == CommandResult::Port) {
                emit emitLog(2, "VALIDATOR", "Port error");
            }

            if (result == CommandResult::NoAnswer) {
                emit emitLog(2, "VALIDATOR", "Empty answer");
            }

            return false;
        }
    } catch (std::exception &e) {
        qDebug() << "Protocol CCNet: Exception : [execCommand] " << QString(e.what());
        return false;
    }

    return true;
}

TResult CCNetSm::processCommand(const QByteArray &aCommandData, QByteArray &aAnswerData) {
    // Выполняем команду
    int nakCounter = 1;
    int checkingCounter = 1;

    do {
        aAnswerData.clear();

        if (isOpened()) {
            serialPort->write(aCommandData);
            serialPort->waitForBytesWritten(50);
        }

        // Задержка после команды
        //        this->msleep(150);

        TResult result = getAnswer(aAnswerData);

        CCNetSm::msleep(50);

        if (result == CommandResult::Transport) {
            nakCounter++;
        } else if (result == CommandResult::Protocol) {
            checkingCounter++;
        } else {
            return result;
        }
    }

    while ((nakCounter <= CCNetConstruct::MaxRepeatPacket) &&
           (checkingCounter <= CCNetConstruct::MaxRepeatPacket));

    return (checkingCounter <= CCNetConstruct::MaxRepeatPacket) ? CommandResult::Transport
                                                                : CommandResult::Protocol;
}

TResult CCNetSm::getAnswer(QByteArray &aAnswerData) {
    QList<QByteArray> answers;

    if (!readAnswers(answers, CCNetConstruct::poolingTimeout)) {
        return CommandResult::Port;
    }

    if (answers.isEmpty()) {
        return CommandResult::NoAnswer;
    }

    int index = -1;
    QStringList logs;
    aAnswerData = answers[0];

    for (int i = 0; i < answers.size(); ++i) {
        logs << check(answers[i]);

        if (logs.last().isEmpty()) {
            index = i;
        }
    }

    //    for (int i = 0; i < answers.size(); ++i) {
    //        qDebug() << QString("CCNet: <<
    //        {%1}").arg(answers[i].toHex().data());
    //    }

    if (index == -1) {
        if (debugger) {
            qDebug() << "CCNet: Answer does not contains any logic data or it is "
                        "incomplete answer";
        }
        return CommandResult::Protocol;
    }

    aAnswerData = answers[index];

    if (aAnswerData[3] == CCNetConstruct::NAK) {
        if (debugger) {
            qDebug() << "CCNet: Answer contains NAK, attemp to repeat command";
        }
        return CommandResult::Transport;
    }

    sendACK();

    return CommandResult::OK;
}

QString CCNetSm::check(const QByteArray &aAnswer) {
    // минимальный размер ответа
    if (aAnswer.size() < CCNetConstruct::MinAnswerSize) {
        return QString("CCNet: Invalid answer length = %1, need %2 minimum")
            .arg(aAnswer.size())
            .arg(CCNetConstruct::MinAnswerSize);
    }

    // первый байт
    char prefix = aAnswer[0];

    if (prefix != 0x02) {
        return QString("CCNet: Invalid prefix = %1, need = %2").arg(prefix).arg(0x02);
    }

    // адрес
    char address = aAnswer[1];

    if (address != 0x03) {
        return QString("CCNet: Invalid address = %1, need = %2").arg(address).arg(0x03);
    }

    // длина
    int length = uchar(aAnswer[2]);

    if (length != aAnswer.size()) {
        return QString("CCNet: Invalid length = %1, need %2").arg(aAnswer.size()).arg(length);
    }

    // CRC
    ushort answerCRC = calcCRC16(aAnswer.left(length - 2));
    ushort crc = qToBigEndian(aAnswer.right(2).toHex().toUShort(nullptr, 16));

    if (crc != answerCRC) {
        return QString("CCNet: Invalid CRC = %1, need %2").arg(crc).arg(answerCRC);
    }

    return "";
}

bool CCNetSm::readAnswers(QList<QByteArray> &aAnswers, int aTimeout) {
    QByteArray answer;

    int length = 0;
    int index = 0;

    QElapsedTimer clockTimer;
    clockTimer.restart();

    do {
        QByteArray answerData;

        if (!serialPort->waitForReadyRead(150)) {
            if (debugger) {
                qDebug() << "waitForReadyRead false";
            }
        }

        // Есть ответ
        answerData = serialPort->readAll();

        answer.append(answerData);
        int begin = index;
        int lastBegin = 0;

        do {
            lastBegin = begin;
            begin = answer.indexOf(0x02, begin);

            if (begin == -1) {
                break;
            }
            if (answer.size() > 2) {
                if (begin < answer.size()) {
                    index = begin;
                }

                length = uchar(answer[begin + 2]);
                begin += length;

                if (begin < answer.size()) {
                    index = begin;
                }
            }
        } while (lastBegin != begin);
    } while ((clockTimer.elapsed() < aTimeout) &&
             ((answer.mid(index).size() != length) || (length == 0)));

    if (answer.isEmpty()) {
        if (debugger) {
            qDebug() << "CCNet: << {}";
        }
        return true;
    }

    int size = answer.size();
    int begin = answer.indexOf(0x02);

    if (begin == -1) {
        begin = size;
    }

    if (begin != 0) {
        aAnswers << answer.mid(0, begin);
    }

    do {
        int next = size;
        int shiftLength = -1;

        if (size >= (begin + 3)) {
            shiftLength = uchar(answer[begin + 2]);
            next = begin + shiftLength;
        }

        aAnswers << answer.mid(begin, next - begin);

        int shift = (shiftLength <= 0) ? 1 : shiftLength;
        begin = answer.indexOf(0x02, begin + shift);
    } while (begin != -1);

    return true;
}

bool CCNetSm::sendACK() {
    QByteArray cmdRequest =
        this->makeCustom_Request(CCNetConstruct::PABillValidator, CCNetConstruct::CCAck, nullptr);

    if (isOpened()) {
        serialPort->write(cmdRequest);
        serialPort->waitForBytesWritten(50);
        return true;
    }

    return false;
}

bool CCNetSm::CmdRestart() {
    QByteArray respData;

    this->execCommand(ValidatorCommands::Reset, respData);
    toValidatorLog(1, respData, "Reset Response");

    this->execCommand(ValidatorCommands::Poll, respData);
    toValidatorLog(1, respData, "Poll Response");

    this->execCommand(ValidatorCommands::GetNominalTable, respData);
    toValidatorLog(1, respData, "GetNominalTable Response");

    this->execCommand(ValidatorCommands::SetSecurity, respData);
    toValidatorLog(1, respData, "SetSecurity Response");

    this->execCommand(ValidatorCommands::Idintification, respData);
    toValidatorLog(1, respData, "Idintification Response");

    this->ParsIdentification(respData);

    return true;
}

void CCNetSm::ParsIdentification(QByteArray respData) {
    QByteArray pn;
    QByteArray sn;

    for (int i = 3; i <= 17; i++) {
        pn.append(respData[i]);
    }

    for (int i = 18; i <= 29; i++) {
        sn.append(respData[i]);
    }

    if (pn.startsWith("C100")) {
        pn = pn.left(5) + sn.trimmed();
    }

    PartNumber = "";
    SerialNumber = "";

    PartNumber.append(pn);
    SerialNumber.append(sn);
}

void CCNetSm::CmdStartPoll() {
    validatorLogEnable = true;

    preDateTime = QDateTime::currentDateTime().addSecs(-1);

    stopPoll = false;
    QByteArray respData;

    // Активируем bill
    this->execCommand(ValidatorCommands::SetEnabled, respData);
    toValidatorLog(1, respData, "SetEnabled Response");

    while (!stopPoll) {

        this->execCommand(ValidatorCommands::Poll, respData);
        toValidatorLog(1, respData, "Poll Response");

        if (respData[3] == CCNetConstruct::States::PowerUp ||
            respData[3] == CCNetConstruct::States::PowerUpInValidator ||
            respData[3] == CCNetConstruct::States::PowerUpInStacker ||
            respData[3] == CCNetConstruct::Errors::Pause) {

            this->CmdRestart();
            continue;
        }

        if (respData[3] == CCNetConstruct::States::Disabled) {
            this->execCommand(ValidatorCommands::SetEnabled, respData);
            toValidatorLog(1, respData, "SetEnabled Response");
            continue;
        }

        if (respData[3] == CCNetConstruct::States::Escrow) {

            int escrowNominal = getNominal(respData[4]);

            escrowed = true;

            emit emitLog(0, "CCNET", QString("Определена купюра %1 смн").arg(escrowNominal));

            if (escrowNominal > 0) {
                if (hasDBError) {
                    this->execCommand(ValidatorCommands::Return, respData);
                    emit emitLog(
                        0,
                        "CCNET",
                        QString("Возвращаем купюру %1 смн, из за ошибки БД").arg(escrowNominal));
                    this->setReturnNominalState(true);
                } else if (maxSum_Reject && (nominalSum + escrowNominal > maxSum)) {
                    this->execCommand(ValidatorCommands::Return, respData);
                    emit emitLog(0,
                                 "CCNET",
                                 QString("Возвращаем купюру %1 смн, так как достигнута "
                                         "максимальная сумма %2")
                                     .arg(escrowNominal)
                                     .arg(maxSum));
                    this->setReturnNominalState(true);
                } else {
                    nominalSum += escrowNominal;

                    this->execCommand(ValidatorCommands::Stack, respData);
                    emit emitLog(0, "CCNET", QString("Отправляем команду на укладку (Stack)"));
                }
            } else {
                this->execCommand(ValidatorCommands::Return, respData);
            }

            this->execCommand(ValidatorCommands::Poll, respData);
            toValidatorLog(1, respData, "Poll Response");
        }

        int nominal = 0;
        nominal = this->readPollInfo(respData);

        // Проверяем есть ли номинал и нет ли повторений
        if (nominal > 0) {
            QDateTime now = QDateTime::currentDateTime();

            qint64 sicPoint = preDateTime.msecsTo(now);

            if (sicPoint < 0) {
                sicPoint = 1000;
            }

            if (sicPoint > 500 && escrowed) {
                toValidatorLog(1, " ", QString("Вставлена купюра %1 смн.").arg(nominal));
                escrowed = false;

                emit this->emitNominal(nominal);
            } else {
                emit emitNominalDuplicate(nominal);
            }

            preDateTime = now;
        }
    }

    CCNetSm::msleep(10);
    this->CmdStopPoll();
}

void CCNetSm::CmdStopPoll() {
    QByteArray respData;
    this->execCommand(ValidatorCommands::SetDisabled, respData);
    toValidatorLog(1, respData, "SetDisabled Response");
    validatorLogEnable = false;
    nominalSum = 0;
}

int CCNetSm::getNominal(const uchar nom) {
    switch (nom) {
    case CCNetConstruct::Nominal_TJ::nom_1:
        return 1;
        break;
    case CCNetConstruct::Nominal_TJ::nom_3:
        return 3;
        break;
    case CCNetConstruct::Nominal_TJ::nom_5:
        return 5;
        break;
    case CCNetConstruct::Nominal_TJ::nom_10:
        return 10;
        break;
    case CCNetConstruct::Nominal_TJ::nom_20:
        return 20;
        break;
    case CCNetConstruct::Nominal_TJ::nom_50:
        return 50;
        break;
    case CCNetConstruct::Nominal_TJ::nom_100:
        return 100;
        break;
    case CCNetConstruct::Nominal_TJ::nom_200:
        return 200;
        break;
    case CCNetConstruct::Nominal_TJ::nom_500:
        return 500;
        break;
    }

    return 0;
}

int CCNetSm::readPollInfo(QByteArray byte) {
    if (byte[3] == CCNetConstruct::States::Stacked) {
        this->setBoolingDlgState(false);
        this->setReturnNominalState(false);

        int nominal = getNominal(byte[4]);
        if (nominal > 0) {
            return nominal;
        }
    }

    if (byte[3] == CCNetConstruct::States::Returned) {
        emit emitLog(0, "CCNET", QString("Купюра возвращена"));
        this->setBoolingDlgState(false);
        return 0;
    }

    if (byte[3] == CCNetConstruct::States::Rejecting) {
        this->setBoolingDlgState(false);
        switch (byte[4]) {
        // Функциональная ошибка ввода
        case CCNetConstruct::Rejects::Operation:
            this->sendStatusTo(VStatus::Warning::Operation,
                               QString("Функциональная ошибка ввода.(6A)"));
            return 0;
            break;

        // Застряла купюра в купюраприемнике
        case CCNetConstruct::Rejects::PreviouslyBillInHead:
            this->sendStatusTo(VStatus::Warning::PreviouslyBillInHead,
                               QString("Застряла купюра в купюраприемнике.(62)"));
            return 0;
            break;

        // Rejecting due to Insertion.
        case CCNetConstruct::Rejects::Insertion:
            this->sendStatusTo(VStatus::Warning::Insertion,
                               QString("Rejecting due to Insertion.(60)"));
            return 0;
            break;

        // Rejecting due to Magnetic.
        case CCNetConstruct::Rejects::Dielectric:
            this->sendStatusTo(VStatus::Warning::Dielectric,
                               QString("Rejecting due to Magnetic.(61)"));
            return 0;
            break;

        // Rejecting due to Multiplying.
        case CCNetConstruct::Rejects::Compensation:
            this->sendStatusTo(VStatus::Warning::Compensation,
                               QString("Rejecting due to Multiplying.(63)"));
            return 0;
            break;

        // Rejecting due to Conveying.
        case CCNetConstruct::Rejects::BillTransport:
            this->sendStatusTo(VStatus::Warning::BillTransport,
                               QString("Rejecting due to Conveying. (64)"));
            return 0;
            break;

        // Rejecting due to Identification.
        case CCNetConstruct::Rejects::Identification:
            this->sendStatusTo(VStatus::Warning::Identification,
                               QString("Rejecting due to Identification.(65)"));
            return 0;
            break;

        // Rejecting due to Verification.
        case CCNetConstruct::Rejects::Verification:
            this->sendStatusTo(VStatus::Warning::Verification,
                               QString("Rejecting due to Verification.(66)"));
            return 0;
            break;

        // Rejecting due to Optic.
        case CCNetConstruct::Rejects::Optical:
            this->sendStatusTo(VStatus::Warning::Optical, QString("Rejecting due to Optic.(67)"));
            return 0;
            break;

        // Rejecting due to Inhibit.
        case CCNetConstruct::Rejects::Inhibit:
            this->sendStatusTo(VStatus::Warning::Inhibit, QString("Rejecting due to Inhibit.(68)"));
            return 0;
            break;

        // Rejecting due to Capacity.
        case CCNetConstruct::Rejects::Capacitance:
            this->sendStatusTo(VStatus::Warning::Capacitance,
                               QString("Rejecting due to Capacity.(69)"));
            return 0;
            break;

        // Rejecting due to Length.
        case CCNetConstruct::Rejects::Length:
            this->sendStatusTo(VStatus::Warning::Length, QString("Rejecting due to Length.(6C)"));
            return 0;
            break;

        // Rejecting due to unrecognised.
        // Bill taken was treated as a barcode but no reliable data can.
        case CCNetConstruct::Rejects::Unrecognised:
            this->sendStatusTo(VStatus::Warning::Unrecognised,
                               QString("Rejecting due to unrecognised.(92)"));
            return 0;
            break;

        // Rejecting due to UV.
        // Banknote UV properties do not meet the predefined criteria.
        case CCNetConstruct::Rejects::UV:
            this->sendStatusTo(VStatus::Warning::UV, QString("Rejecting due to UV.(6D)"));
            return 0;
            break;

        // Rejecting due to incorrect number of characters in barcode.
        // Barcode data was read (at list partially) but is inconsistent.
        case CCNetConstruct::Rejects::IncorrectBarcode:
            this->sendStatusTo(VStatus::Warning::IncorrectBarcode,
                               QString("Rejecting due to incorrect number of "
                                       "characters in barcode.(93)"));
            return 0;
            break;

        // Rejecting due to unknown barcode start sequence.
        // Barcode was not read as no synchronization was established.
        case CCNetConstruct::Rejects::UnknownBarcode:
            this->sendStatusTo(VStatus::Warning::UnknownBarcode,
                               QString("Rejecting due to unknown barcode start sequence.(94)"));
            return 0;
            break;

        // Rejecting due to unknown barcode stop sequence.
        // Barcode was read but trailing data is corrupt..
        case CCNetConstruct::Rejects::CorruptedTrailingData:
            this->sendStatusTo(VStatus::Warning::CorruptedTrailingData,
                               QString("Rejecting due to unknown barcode stop sequence.(95)"));
            return 0;
            break;
        }
    }

    // Замятие купюры
    switch (byte[3]) {

    // Ошибка! Замятие купюры в купюроприемнике
    case CCNetConstruct::Errors::ValidatorJammed:
        this->sendStatusTo(VStatus::Errors::ValidatorJammed,
                           QString("Ошибка! Замятие купюры в купюроприемнике.(43)"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    // Переполнение стекера
    case CCNetConstruct::Errors::StackerFull:
        this->sendStatusTo(VStatus::Errors::StackerFull,
                           QString("Переполнение стекера (Сделайте инкасацию).(41)"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    // открыта касета
    case CCNetConstruct::Errors::BadStackerPosition:
        this->sendStatusTo(VStatus::Errors::BadStackerPosition,
                           QString("Открыта касета купюроприемника.(42)"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    // Замятие купюры в боксе
    case CCNetConstruct::Errors::StackerJammed:
        this->sendStatusTo(VStatus::Errors::StackerJammed, QString("Замятие купюры в боксе.(44)"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    // Bill Validator sends this event if the intentions of the user to deceive
    // the Bill Validator are detected.
    case CCNetConstruct::Errors::Cheated:
        this->sendStatusTo(VStatus::Warning::Cheated, QString("Попытка мошенничество (45)"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    // When the user tries to insert a second bill when the previous bill is in
    // the Bill Validator but has not been stacked. Thus Bill Validator stops
    // motion of the second bill until the second bill is removed.
    case CCNetConstruct::Errors::Pause:
        this->sendStatusTo(VStatus::Warning::Paused,
                           QString("Купюроприемник перешел в режим паузы.(46)"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    // Купюраприемник не активен для приема денег
    case CCNetConstruct::States::Disabled:
        this->sendStatusTo(VStatus::Success::Ok, QString("19"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    // Stacking
    case CCNetConstruct::States::Stacking:
        this->sendStatusTo(VStatus::Success::Ok, QString("17")); /*qDebug() << "----Stacking----";*/
        return 0;
        break;

    // Accepting
    case CCNetConstruct::States::Accepting:
        this->sendStatusTo(VStatus::Success::Ok, QString("15"));
        this->setBoolingDlgState(true);
        this->setReturnNominalState(false); /*qDebug() << "----Accepting----";*/
        return 0;
        break;

    // Returning
    case CCNetConstruct::States::Returning:
        this->sendStatusTo(VStatus::Success::Ok, QString("18"));
        this->setBoolingDlgState(false); /*qDebug() << "----Returning----";*/
        return 0;
        break;

        //        //Idling
        //        case CCNetConstruct::States::Idling:
        //        this->sendStatusTo(0,QString("18")); qDebug() << "----Idling----";
        //        return 0; break;

    // Идет питание на Купюроприемник.
    case CCNetConstruct::States::PowerUpInValidator:
        this->sendStatusTo(VStatus::Success::Ok, QString("Идет питание на Купюроприемник.(11)"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    case CCNetConstruct::States::Initialize:
        this->sendStatusTo(VStatus::Success::Ok,
                           QString("Идет инициализация купюроприёмника.(13)"));
        this->setBoolingDlgState(false);
        return 0;
        break;

    // Тут надо проверить неисправности
    //  Ошибка! Откройте купюроприемник и прочистите содержимое...
    case CCNetConstruct::Errors::Failure:
        //            this->sendStatusTo(VStatus::Errors::Failure,QString("Ошибка!
        //            Инициализации купюроприемника.(47)"));
        this->setBoolingDlgState(false);
        //            return 0;
        switch (byte[4]) {

        case CCNetConstruct::Failures::StackerMotor:
            this->sendStatusTo(VStatus::Errors::StackerMotor,
                               QString("Drop Cassette Motor failure.(50))"));
            return 0;
            break;

        case CCNetConstruct::Failures::TransportMotorSpeed:
            this->sendStatusTo(VStatus::Errors::TransportMotorSpeed,
                               QString("Transport Motor Speed Failure.(51)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::TransportMotor:
            this->sendStatusTo(VStatus::Errors::TransportMotor,
                               QString("Transport Motor failure.(52)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::AligningMotor:
            this->sendStatusTo(VStatus::Errors::AligningMotor,
                               QString("Aligning Motor Failure.(53)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::InitialStackerStatus:
            this->sendStatusTo(VStatus::Errors::InitialStackerStatus,
                               QString("Initial Cassette Status Failure.(54)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::Optical:
            this->sendStatusTo(VStatus::Errors::Optical,
                               QString("One of the optic sensors has failed to "
                                       "provide its response.(55)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::Inductive:
            this->sendStatusTo(VStatus::Errors::Inductive,
                               QString("Inductive sensor failed to respond.(56)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::Cassette1MotorF:
            this->sendStatusTo(VStatus::Errors::Cassette1MotorF,
                               QString("Cassette 1 Motor Failure.(57)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::Cassette2MotorF:
            this->sendStatusTo(VStatus::Errors::Cassette2MotorF,
                               QString("Cassette 2 Motor Failure.(58)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::Cassette3MotorF:
            this->sendStatusTo(VStatus::Errors::Cassette3MotorF,
                               QString("Cassette 3 Motor Failure.(59)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::BillToBillTransport:
            this->sendStatusTo(VStatus::Errors::BillToBillTransport,
                               QString("Bill-to-Bill unit Transport Failure.(5A)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::SwitchMotor1:
            this->sendStatusTo(VStatus::Errors::SwitchMotor1,
                               QString("Switch Motor 1 Failure.(5B)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::SwitchMotor2:
            this->sendStatusTo(VStatus::Errors::SwitchMotor2,
                               QString("Switch Motor 2 Failure.(5C)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::DispenserMotor1:
            this->sendStatusTo(VStatus::Errors::DispenserMotor1,
                               QString("Dispenser Motor 1 Failure.(5D)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::DispenserMotor2:
            this->sendStatusTo(VStatus::Errors::DispenserMotor1,
                               QString("Dispenser Motor 2 Failure.(5E)))"));
            return 0;
            break;

        case CCNetConstruct::Failures::Capacitance:
            this->sendStatusTo(VStatus::Errors::Capacitance,
                               QString("Capacitance sensor failed to respond.(5F)))"));
            return 0;
            break;
        }
        break;

    case CCNetConstruct::Errors::BillJammed:
        this->setBoolingDlgState(false);

        switch (byte[4]) {
        case CCNetConstruct::BillJammed::JCassette1:
            this->sendStatusTo(VStatus::Errors::JCassette1,
                               QString("Bill Jammed in Cassette 1.(70))"));
            return 0;
            break;

        case CCNetConstruct::BillJammed::JCassette2:
            this->sendStatusTo(VStatus::Errors::JCassette2,
                               QString("Bill Jammed in Cassette 2.(71))"));
            return 0;
            break;

        case CCNetConstruct::BillJammed::JCassette3:
            this->sendStatusTo(VStatus::Errors::JCassette3,
                               QString("Bill Jammed in Cassette 3.(72))"));
            return 0;
            break;

        case CCNetConstruct::BillJammed::JTransportPath:
            this->sendStatusTo(VStatus::Errors::JTransportPath,
                               QString("Bill Jammed in Transport Path.(73))"));
            return 0;
            break;

        case CCNetConstruct::BillJammed::JSwitch:
            this->sendStatusTo(VStatus::Errors::JSwitch, QString("Bill Jammed in Switch.(74))"));
            return 0;
            break;

        case CCNetConstruct::BillJammed::JDispenser:
            this->sendStatusTo(VStatus::Errors::JDispenser,
                               QString("Bill Jammed in Dispenser.(75))"));
            return 0;
            break;
        }
        break;
    }

    return 0;
}

QByteArray CCNetSm::fwCmdRequest(const QByteArray &data, quint8 command) {
    QByteArray packet;

    if (command > 0) {
        packet.resize(CCNetConstruct::FwPacketSize);
        packet.fill('\x00', CCNetConstruct::FwPacketSize);

        packet[0] = 0x02;
        packet[1] = 0x01;
        packet[2] = command;

        for (int i = 0; i < data.size(); ++i) {
            packet[4 + i] = data[i];
        }

        ushort crc = calcCRC16(packet.left(4));
        packet[4] = uchar(crc);
        packet[5] = uchar(crc >> 8);
    } else {
        packet.resize(6);
        for (int i = 0; i < data.size(); ++i) {
            packet[i] = data[i];
        }

        ushort crc = calcCRC16(packet.left(4));
        packet[4] = uchar(crc);
        packet[5] = uchar(crc >> 8);
    }

    return packet;
}

QByteArray CCNetSm::fwPacketRequest(quint8 command, quint8 address, const QByteArray &data) {
    int packetSize = CCNetConstruct::FwPacketSize;
    QByteArray packet(packetSize, 0);

    packet[0] = 0x02;
    packet[1] = 0x01;
    packet[2] = command;
    packet[3] = address;

    for (int i = 0; i < data.size() && (4 + i) < packetSize - 2; ++i) {
        packet[4 + i] = data[i];
    }

    ushort crc = calcCRC16(packet.left(packetSize - 2));
    packet[packetSize - 2] = uchar(crc);
    packet[packetSize - 1] = uchar(crc >> 8);

    return packet;
}

QByteArray CCNetSm::fwPacketUpdRequest(quint16 adr, const QByteArray &data) {
    const int packetSize = 72;

    QByteArray packet(packetSize, 0);

    auto addrHi = static_cast<quint8>((adr >> 8) & 0xFF);
    auto addrLo = static_cast<quint8>(adr & 0xFF);

    packet[0] = 0x02;
    packet[1] = 0x03;
    packet[2] = packetSize;
    packet[3] = 0xAA;
    packet[4] = addrHi;
    packet[5] = addrLo;

    for (int i = 0; i < data.size() && (6 + i) < packetSize - 2; ++i) {
        packet[6 + i] = data[i];
    }

    ushort crc = calcCRC16(packet.left(packetSize - 2));
    packet[packetSize - 2] = uchar(crc);
    packet[packetSize - 1] = uchar(crc >> 8);

    return packet;
}

bool CCNetSm::fwCmdExec(const QByteArray aCommandData) {
    if (isOpened()) {
        serialPort->write(aCommandData);
        serialPort->waitForBytesWritten(150);
    }

    // Задержка после команды
    //    msleep(150);

    QByteArray aAnswerData;

    QElapsedTimer clockTimer;
    clockTimer.restart();

    do {
        serialPort->waitForReadyRead(150);

        // Есть ответ
        QByteArray answerData = serialPort->readAll();

        aAnswerData.append(answerData);

    } while ((clockTimer.elapsed() < 2000) && QString::fromUtf8(aAnswerData) != "OK");

    return QString::fromUtf8(aAnswerData) == "OK";
}

bool CCNetSm::serviceModeSwitch() {
    QByteArray data;
    data.resize(6);

    data[0] = 0x02;
    data[1] = 0x03;
    data[2] = 0x06;
    data[3] = 0x88;
    data[4] = 0x00;
    data[5] = 0x00;

    QByteArray cmdRequest = fwCmdRequest(data);
    bool result = fwCmdExec(cmdRequest);

    return result;
}

bool CCNetSm::resetValidator() {
    QByteArray data;
    data.resize(6);

    data[0] = 0x02;
    data[1] = 0x03;
    data[2] = 0x06;
    data[3] = 0xBB;
    data[4] = 0x00;
    data[5] = 0x00;

    QByteArray cmdRequest = fwCmdRequest(data);

    bool result = fwCmdExec(cmdRequest);

    return result;
}

bool CCNetSm::unlockValidator() {
    QByteArray packet(64, 0);

    ushort adr = 0;
    bool res = false;

    auto bloader = QByteArray(reinterpret_cast<const char *>(CCNetFirmware::bloader),
                              sizeof(CCNetFirmware::bloader));

    for (int i = 0; i < 16; i++) {
        packet = bloader.mid(i * 64, 64);

        adr = (ushort)(0x3000 + (i * 0x0200));

        res = fwCmdExec(fwPacketUpdRequest(adr, packet));

        if (res) {
            emit emitLog(0, "FIRMWARE_CCNET", QString("page %1 unlocked!").arg(i));
        } else {
            emit emitLog(2, "FIRMWARE_CCNET", "The device not answer!");
            return false;
        };

        msleep(200);
    }

    auto ldr1A00 = QByteArray(reinterpret_cast<const char *>(CCNetFirmware::ldr1A00),
                              sizeof(CCNetFirmware::ldr1A00));
    adr = (ushort)(0x1A00);
    res = fwCmdExec(fwPacketUpdRequest(adr, ldr1A00));

    if (res) {
        emit emitLog(0, "FIRMWARE_CCNET", "page 0x1A00 unlocked!");
    } else {
        emit emitLog(2, "FIRMWARE_CCNET", "The device not answer!");
        return false;
    };

    msleep(200);

    auto ldr1C00 = QByteArray(reinterpret_cast<const char *>(CCNetFirmware::ldr1C00),
                              sizeof(CCNetFirmware::ldr1C00));
    adr = (ushort)(0x1C00);
    res = fwCmdExec(fwPacketUpdRequest(adr, ldr1C00));

    if (res) {
        emit emitLog(0, "FIRMWARE_CCNET", "page 0x1C00 unlocked!");
    } else {
        emit emitLog(2, "FIRMWARE_CCNET", "The device not answer!");
        return false;
    };

    msleep(200);

    auto ldr1E00 = QByteArray(reinterpret_cast<const char *>(CCNetFirmware::ldr1E00),
                              sizeof(CCNetFirmware::ldr1E00));
    adr = (ushort)(0x1E00);
    res = fwCmdExec(fwPacketUpdRequest(adr, ldr1E00));

    if (res) {
        emit emitLog(0, "FIRMWARE_CCNET", "page 0x1E00 unlocked!");
    } else {
        emit emitLog(2, "FIRMWARE_CCNET", "The device not answer!");
        return false;
    };

    msleep(200);

    auto ldrFE00 = QByteArray(reinterpret_cast<const char *>(CCNetFirmware::ldrFE00),
                              sizeof(CCNetFirmware::ldrFE00));
    adr = (ushort)(0xFFC0);
    res = fwCmdExec(fwPacketUpdRequest(adr, ldrFE00));

    if (res) {
        emit emitLog(0, "FIRMWARE_CCNET", "page 0xFE00 unlocked!");
    } else {
        emit emitLog(2, "FIRMWARE_CCNET", "The device not answer!");
        return false;
    };

    resetValidator();

    return true;
}

bool CCNetSm::checkBootloader() {
    QByteArray data;
    data.resize(6);

    data[0] = 0x02;
    data[1] = 0x01;
    data[2] = 0x99;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;

    QByteArray cmdRequest = fwCmdRequest(data, '\x99');

    bool result = fwCmdExec(cmdRequest);

    return result;
}

QByteArray CCNetSm::firmwareGet(QString version) {
    if (version.isEmpty()) {
        return {};
    }

    QByteArray data;

    QFile file(QString("assets/firmware/cashcode/%1.txt").arg(version));

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll().trimmed();

        // Remove spaces and split by comma
        QStringList hexList = content.remove(' ').remove('\n').split(',', Qt::SkipEmptyParts);

        for (const QString &hexStr : hexList) {
            if (hexStr.startsWith("0x", Qt::CaseInsensitive)) {

                bool ok = false;
                uchar byte = hexStr.mid(2).toUInt(&ok, 16);

                if (ok) {
                    data.append(byte);
                }
            }
        }

        file.close();
    }

    return data;
}

bool CCNetSm::CmdFirmwareUpdate(QString version) {
    if (firmwareUpdating) {
        emit emitLog(1, "FIRMWARE_CCNET", "Идет обновление прошивки");
        return false;
    }

    if (version.startsWith("C100")) {
        fwUpdateC100(version);
        return true;
    }

    if (version.isEmpty()) {
        QDir directory("assets/firmware/cashcode");
        QStringList firmwareList = directory.entryList(QStringList() << "*.txt", QDir::Files);

        version = !firmwareList.empty() ? firmwareList.last().remove(".txt") : "";
    }

    QByteArray fwData = firmwareGet(version);

    if (fwData.length() < 57344) {
        emit emitFirmwareUpdate("cancel");
        emit emitLog(2, "FIRMWARE_CCNET", "Неверный файл прошивки!");
        return false;
    }

    emit emitFirmwareUpdate("start");

    // Застряла прошивка
    if (checkBootloader()) {
        bool result = firmwareUpdate(fwData);
        return result;
    } // Переводим в режим сервиса
    if (!serviceModeSwitch()) {
        emit emitLog(2, "FIRMWARE_CCNET", "Ошибка входа в сервисный режим");
        emit emitFirmwareUpdate("cancel");
        return false;
    }

    emit emitLog(0, "FIRMWARE_CCNET", "Вход в сервисный режим ОК");

    // Загружаем бутлоадер
    if (!unlockValidator()) {
        emit emitLog(0, "FIRMWARE_CCNET", "Выход из сервисного режима");
        resetValidator();
        msleep(1000);
        emit emitFirmwareUpdate("cancel");
        return false;
    }

    msleep(1000);

    if (checkBootloader()) {
        bool result = firmwareUpdate(fwData);
        return result;
    }         emit emitLog(0, "FIRMWARE_CCNET", "Bootloader false");
        resetValidator();
        msleep(1000);
        emit emitFirmwareUpdate("error");
        return false;
   
}

bool CCNetSm::firmwareUpdate(const QByteArray fw) {
    QByteArray packet(512, 0);
    double prg = 0;

    firmwareUpdating = true;

    for (int page = 0; page < 112; page++) {
        for (int i = 0; i < 512; i++) {
            packet[i] = fw[i + (page * 512)];
        }

        bool res = fwCmdExec(fwPacketRequest(0xAA, 0x10 + page, packet));

        if (res) {
            prg = 0.91 * page;
            prg = std::min<double>(prg, 100);

            emit emitLog(0, "FIRMWARE_CCNET", QString("Обновление прошивки %1%").arg(qRound(prg)));
        } else {
            firmwareUpdating = false;
            emit emitLog(2, "FIRMWARE_CCNET", "Ошибка обновления прошивки!");
            emit emitFirmwareUpdate("error");
            return false;
        }
    }

    emit emitLog(0, "FIRMWARE_CCNET", "Прошивка успешно записана!");

    firmwareUpdating = false;

    // Reset
    QByteArray data;
    data.resize(6);

    data[0] = 0x02;
    data[1] = 0x01;
    data[2] = 0xBB;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;

    QByteArray cmdRequest = fwCmdRequest(data, '\xBB');

    fwCmdExec(cmdRequest);

    emit emitLog(0, "FIRMWARE_CCNET", "Перезагрузка ...");

    msleep(2000);

    emit emitFirmwareUpdate("success");

    return true;
}

bool CCNetSm::fwUpdateC100(QString version) {
    if (version.isEmpty()) {
        return false;
    }

    m_lib.setFileName("assets/firmware/c100/CashPayment");

    if (!m_lib.load()) {
        emit emitLog(
            2, "FIRMWARE_CCNET", QString("Не удалось загрузить DLL: %1").arg(m_lib.errorString()));
        emit emitFirmwareUpdate("cancel");
        return false;
    }

    QFile file(QString("assets/firmware/c100/%1.dat").arg(version));
    QFileInfo fileInfo(file);

    QString path = fileInfo.absoluteFilePath();
    QString port = com_Name;
    QByteArray filePathBytes = path.toUtf8();

    int state = 0;
    int nPort = port.remove("COM").toInt();
    char *filePath = filePathBytes.data();

    auto sendFirmWareDataByPathFunc =
        (SendFirm_WareDataByPathFunc)m_lib.resolve("SendFirm_WareDataByPath");
    if (!sendFirmWareDataByPathFunc) {
        emit emitLog(
            2, "FIRMWARE_CCNET", QString("Не найдена функция: %1").arg(m_lib.errorString()));
        emit emitFirmwareUpdate("cancel");
        return false;
    }

    // Close serial port
    closePort();

    msleep(2000);

    state = sendFirmWareDataByPathFunc(nPort, filePath);

    if (state > 0) {
        emit emitFirmwareUpdate("start");

        firmwareUpdating = true;

        while (true) {

            auto getDataStatus = (GetDataStatusFunc)m_lib.resolve("GetDataStatus");
            if (!getDataStatus) {
                emit emitLog(2,
                             "FIRMWARE_CCNET",
                             QString("Не найдена функция: %1").arg(m_lib.errorString()));
                fwCancelC100();
                return false;
            }

            long downFileSize = getDataStatus();

            if (downFileSize == 0) {
                emit emitLog(0, "FIRMWARE_CCNET", "Обновление прошивки 100%");
                emit emitLog(0, "FIRMWARE_CCNET", "Прошивка успешно записана!");

                msleep(2000);

                emit emitFirmwareUpdate("success");
                firmwareUpdating = false;
                break;
            }
            if (downFileSize > 0) {
                emit emitLog(0,
                             "FIRMWARE_CCNET",
                             QString("Обновление прошивки %1%").arg(downFileSize * 100 / state));
            } else {
                emit emitLog(2, "FIRMWARE_CCNET", "Ошибка обновления");

                fwCancelC100();
                return false;
            }

            msleep(1000);
        }

        return true;
    }

    if (state == -1099) {
        emit emitLog(2, "FIRMWARE_CCNET", QString("Отсутствует файл прошивки %1").arg(path));
    } else if (state == -1100) {
        emit emitLog(2, "FIRMWARE_CCNET", "Загружается файл прошивки");
    } else if (state == -1102) {
        emit emitLog(2, "FIRMWARE_CCNET", "Размер файла слишком большой");
    } else if (state == -1103) {
        emit emitLog(2, "FIRMWARE_CCNET", "Ошибка связи");
    } else if (state == -1104) {
        emit emitLog(2, "FIRMWARE_CCNET", "Версия прошивки одинаковая");
    } else if (state == -1106) {
        emit emitLog(2, "FIRMWARE_CCNET", "Ошибка запуска потока");
    }

    msleep(1000);

    fwCancelC100();

    return false;
}

void CCNetSm::fwCancelC100() {
    // Open serial port
    OpenPort();

    emit emitFirmwareUpdate("cancel");
    firmwareUpdating = false;
}

void CCNetSm::setBoolingDlgState(bool sts) {
    Q_UNUSED(sts)
    //    if(sts_animate_dlg != sts){
    //        sts_animate_dlg = sts;
    //        emit this->emitAnimateStatus(sts_animate_dlg);
    //    }
}

void CCNetSm::setReturnNominalState(bool sts) {
    emit this->emitReturnNominalStatus(sts);
}

void CCNetSm::toValidatorLog(int state, QByteArray data, QString text) {
    if (validatorLogEnable) {
        emit emitValidatorLog(state, data, text);
    }
}
