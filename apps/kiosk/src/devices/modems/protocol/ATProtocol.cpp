#include "ATProtocol.h"
#include <qmath.h>
#include "qatutils.h"
#include <QTextCodec>
#include "qgsmcodec.h"

//--------------------------------------------------------------------------------
ATProtocol::ATProtocol(QObject *parent)  : QThread(parent)

{
    Debugger = false;

    //Соответсвие статусам сообщения
    smsTextInit << SmsTextIndex::txtErrorValidator
                << SmsTextIndex::txtErrorPrinter
                << SmsTextIndex::txtErrorBalanceAgent
                << SmsTextIndex::txtErrorSimBalance
                << SmsTextIndex::txtErrorLockTerminal
                << SmsTextIndex::txtErrorConnection;

    createDevicePort();
}

void ATProtocol::setPortName(const QString com_Name)
{
    comName = com_Name;
}

bool ATProtocol::openPort()
{
    if(devicesCreated){
        //Если девайс для работы с портом обявлен
        is_open = false;
        //    return is_open;
        //Даем девайсу название порта
        //        if(Debuger)  qDebug() << "devicePort->setDeviceName(comName); - " << comName;
        serialPort->setPortName(comName);

        //        if(Debuger) qDebug() << "After set devicePort->DeviceName(); " << devicePort->deviceName();

        if (serialPort->open(QIODevice::ReadWrite)){
            //Если Девайсу удалось открыть порт

            //Устанавливаем параметры открытия порта
            is_open = false;

            if (!serialPort->setBaudRate(QSerialPort::Baud115200)) return false;
            if (!serialPort->setDataBits(QSerialPort::Data8)) return false;
            //                if (!devicePort->setParity(AbstractSerial::ParityNone)) return false;
            //                if (!devicePort->setStopBits(AbstractSerial::StopBits1)) return false;
            if (!serialPort->setFlowControl(QSerialPort::HardwareControl)) return false;
//            if (!serialPort->setCharIntervalTimeout(40)) return false;


            //                if(Debuger) qDebug() << "\nModem " << " to Port " << devicePort->deviceName() << " open in " << devicePort->openMode();

            //                        if(Debuger) qDebug() << "\n= Defaults parameters =";
            //                        if(Debuger) qDebug() << "Device name            : " << devicePort->deviceName();
            //                        if(Debuger) qDebug() << "Baud rate              : " << devicePort->baudRate();
            //                        if(Debuger) qDebug() << "Data bits              : " << devicePort->dataBits();
            //                        if(Debuger) qDebug() << "Parity                 : " << devicePort->parity();
            //                        if(Debuger) qDebug() << "Stop bits              : " << devicePort->stopBits();
            //                        if(Debuger) qDebug() << "Flow                   : " << devicePort->flowControl();
            //                        if(Debuger) qDebug() << "Char timeout, msec     : " << devicePort->charIntervalTimeout();

            is_open = true;

        }else{
            is_open = false;
            //            statusDevices = this->PortError;
            //            if(Debuger) qDebug() << "Error opened serial device " << devicePort->deviceName();
        }
    }else{
        is_open = false;
        //        if(Debuger) qDebug() << "Error create serial device " << devicePort->deviceName();
        //        statusDevices = this->DeviceError;
    }

    return is_open;

}

bool ATProtocol::sendSMSParam(QString text)
{
    //    //Номер телефона
    //    this->numberPhoneSms    = numberPhone;

    float l = (text.length()-2) / 2.0f;

    this->GetLengthSMS = qRound(l);

    //Текст сообщения
    this->textToSendSms        = text;


    bool resp = false;
    QByteArray request;
    QByteArray response;


    //at+cmgf=1
    if(this->processCommand(ModemProtocolCommands::CmdStateSMS,request,response)){

        request.clear(); response.clear();

        this->processCommand(ModemProtocolCommands::CmdSetNumberSMS,request,response);

        request.clear(); response.clear();

        //Сам текст сообщения
        if(this->processCommand(ModemProtocolCommands::CmdTextIntersetSMS,request,response)){

            qDebug() << "\n<<======Answer CmdTextIntersetSMS true";
            resp = true;
        }
    }

    return resp;
}

bool ATProtocol::closePort()
{
    if (!isOpened()) {
        return true;
    }

//        qDebug() << "----START to CLOSE MODEM----";
//    serialPort->reset();
//        qDebug() << "----RESET MODEM OK ----";
    serialPort->close();
    is_open = false;
//        qDebug() << "----AFTER CLOSE MODEM----";
    return true;
}

//--------------------------------------------------------------------------------
void ATProtocol::packetData(const QByteArray& aCommandPacket, QByteArray& aPacket)
{
    // записываем команду
    aPacket.push_back(aCommandPacket);

    // перевод строки '\r'
    aPacket.push_back(CATProtocolCommands::CR);
}

//--------------------------------------------------------------------------------
bool ATProtocol::unpacketData(const QByteArray& aPacket, QByteArray& aData)
{
    aData.clear();
    QByteArray tempData;
    if(!aPacket.size())
    {
        qDebug() << "Protocol AT: The length of the packet is 0";
        return false;
    }

    // первыми и последними символами должны идти CR LF
    // однако в модем может в ответ повторить саму команду, поэтому это надо учесть
    QByteArray CRLF;
    CRLF.push_back(CATProtocolCommands::CR);
    CRLF.push_back(CATProtocolCommands::LF);

    int firstCRLF = aPacket.indexOf(CRLF);
    int lastCRLF = aPacket.indexOf(CRLF, aPacket.size() - CRLF.size() - 1);
    if (firstCRLF == -1 || lastCRLF == firstCRLF)
    {
        qDebug() << "Protocol AT: Invalid format of the response, need first CRLF";
        return false;
    }

    // находим символы CR LF, обрезаем
    int messageLength = aPacket.size() - 2 * CRLF.size() - firstCRLF;
    tempData = aPacket.mid(firstCRLF + CRLF.size(), messageLength);

    qDebug() << "-- tempData -- " << tempData;

    // проверка ответа на ошибки
    ATErrors::Enum message = unpacketError(tempData);
    if (message == ATErrors::OK)
    {
        aData = tempData;
        return true;
    }
    else
    {
        // отправить сообщение об ошибке в данных
        qDebug() << "Protocol AT: message is not OK";
        return false;
    }
}

//--------------------------------------------------------------------------------
ATErrors::Enum ATProtocol::unpacketError(const QByteArray& aPacket)
{
    ATErrors::Enum result = ATErrors::Unknown;
    QString strPacket(aPacket);
    if (strPacket.indexOf(ATErrors::Strings::OK) != -1)
        result = ATErrors::OK;
    else if (strPacket.indexOf(ATErrors::Strings::Connect600) != -1 ||
             strPacket.indexOf(ATErrors::Strings::Connect1200) != -1 ||
             strPacket.indexOf(ATErrors::Strings::Connect2400) != -1 ||
             strPacket.indexOf(ATErrors::Strings::Connect) != -1)
    {
        result = ATErrors::Connect;
    }
    else if (strPacket.indexOf(ATErrors::Strings::Busy) != -1)
        result = ATErrors::Busy;
    else if (strPacket.indexOf(ATErrors::Strings::Ring) != -1)
        result = ATErrors::Ring;
    else if (strPacket.indexOf(ATErrors::Strings::Error) != -1)
        result = ATErrors::Error;
    else if (strPacket.indexOf(ATErrors::Strings::NoAnswer) != -1)
        result = ATErrors::NoAnswer;
    else if (strPacket.indexOf(ATErrors::Strings::NoCarrier) != -1)
        result = ATErrors::NoCarrier;
    else if (strPacket.indexOf(ATErrors::Strings::NoDialtone) != -1)
        result = ATErrors::NoDialtone;

    return result;
}

//--------------------------------------------------------------------------------
bool ATProtocol::processCommand(ModemProtocolCommands::Enum aCommand,
                                const QByteArray& aCommandData, QByteArray& aAnswerData)
{
    int addRepeatCount = 0;
    int pauseTime = 0;
    waittimeforans = 0;
    QByteArray commandData;
    QByteArray commandPacket;

    switch(aCommand){
    case ModemProtocolCommands::GetBalance :
    {

        commandData = aCommandData;

        // вытаскиваем из данных регулярное выражение
        m_getBalanceRegExp = QRegExp(regExpBalance);

        // устанавливаем число дополнительных чтений и паузу между чтениями
        addRepeatCount = CATProtocolConstants::GetBalanceAddRepeatCount;
        pauseTime = CATProtocolConstants::GetBalancePauseTime;
        waittimeforans = 100;
    }
    break;

    case ModemProtocolCommands::GetSimNumber :
    {

        commandData = aCommandData;

        // вытаскиваем из данных регулярное выражение
        m_getBalanceRegExp = QRegExp(regExpSimNumber);

        // устанавливаем число дополнительных чтений и паузу между чтениями
        addRepeatCount = CATProtocolConstants::GetSimNumberAddRepeatCount;
        pauseTime = CATProtocolConstants::GetSimNumberPauseTime;
        waittimeforans = 1000;
    }
    break;
    case ModemProtocolCommands::CmdSetNumberSMS :
    {

        commandData = aCommandData;

        // устанавливаем число дополнительных чтений и паузу между чтениями
        addRepeatCount = 2;
        pauseTime = CATProtocolConstants::GetBalancePauseTime;
        waittimeforans = 100;
    }
    break;
    case ModemProtocolCommands::CmdTextIntersetSMS :
    {
        commandData = aCommandData;

        // устанавливаем число дополнительных чтений и паузу между чтениями
        addRepeatCount = CATProtocolConstants::SendSMSAddRepeatCount;
        pauseTime = CATProtocolConstants::GetBalancePauseTime;
        waittimeforans = 1000;
    }
    break;
    case ModemProtocolCommands::CmdDellAllSms :
    {
        commandData = aCommandData;

        // устанавливаем число дополнительных чтений и паузу между чтениями
        addRepeatCount = CATProtocolConstants::SendSMSAddRepeatCount;
        pauseTime = CATProtocolConstants::GetBalancePauseTime;
        waittimeforans = 100;
    }
    break;
    case ModemProtocolCommands::CmdGetAllSms :
    {
        commandData = aCommandData;

        // устанавливаем число дополнительных чтений и паузу между чтениями
        addRepeatCount = CATProtocolConstants::GetAllSMSRepeatCount;
        pauseTime = CATProtocolConstants::GetBalancePauseTime;
        waittimeforans = 100;
    }
    break;
    default:
    {
        commandData = aCommandData;
        waittimeforans = 100;
    }
    break;

    }

    qDebug() << "PRE if (!getCommandPacket(aCommand, commandData, commandPacket))";
    // Получаем пакет команды
    qDebug() << "aCommand - " << aCommand;
    if (!getCommandPacket(aCommand, commandData, commandPacket))
    {
        return false;
    }

    // Упаковываем все остальные байты
    QByteArray packet;
    packetData(commandPacket, packet);

    qDebug() << "PRE if (!execCommand( packet))";

    // Выполняем команду
    if (!execCommand( packet))
    {
        //                if(Debuger) qDebug() << "Protocol AT: Error executing command";
        return false;
    }

    if(aCommand == ModemProtocolCommands::CmdRestart){
        this->msleep(10000);
        return true;
    }

    //        qDebug() << "PRE if (!getAnswer(aAnswerData, addRepeatCount, pauseTime))";

    // Получаем ответ
    bool codecError = false;
    if (!getAnswer(aAnswerData, codecError, addRepeatCount, pauseTime))
    {
        // ошибка кодировки
        if(codecError){
            QByteArray request;
            QByteArray response;
            if(aCommand == ModemProtocolCommands::GetSimNumber){
                simNumberRequest = encodeGSM7bit(simNumberRequest);
                qDebug()<<"simNumberRequest = "<<simNumberRequest;
                processCommand(ModemProtocolCommands::GetSimNumber, request, response);
                return true;
            }else if(aCommand == ModemProtocolCommands::GetBalance){
                balanceRequest = encodeGSM7bit(balanceRequest);
                processCommand(ModemProtocolCommands::GetBalance, request, response);
                return true;
            }
        }else{
            return false;
        }
    }

    //        qDebug() << "PRE if (!prepareAnswer(aCommand, aAnswerData))";

    // Препарируем ответ
    if (!prepareAnswer(aCommand, aAnswerData))
    {
        //                if(Debuger) qDebug() << "Protocol AT: Error prepareAnswer";
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------
bool ATProtocol::execCommand(QByteArray& aPacket)
{
    QByteArray resp;
    this->sendCommand(aPacket,false,0,resp,0);
    return true;
}


void ATProtocol::printDataToHex(const QByteArray &data)
{

    QByteArray baTmp;
    baTmp.clear();
#if QT_VERSION >= 0x040300
    baTmp = (data.toHex()).toUpper();
#else
    quint8 n=0;
    for (int i=0;i<data.size();i++) {
        n=data.at(i);
        if ((n >= 0) && (n <= 15)) baTmp.append(QByteArray::number(0, 16).toUpper());
        baTmp.append(QByteArray::number(n, 16).toUpper());
    }
#endif

    for (int i=0;i<baTmp.size();i+=2) {
        //        if(Debuger) qDebug() << "[" << baTmp.at(i) << baTmp.at(i + 1) << "]";
    }

}

bool ATProtocol::isOpened()
{
    if(serialPort->isOpen())
        is_open = true;
    else
        is_open = false;

    return is_open;
}


bool ATProtocol::createDevicePort()
{

    serialPort = new QSerialPort(this);

    if (!serialPort) {
        devicesCreated = false;
        return false;
    }

    devicesCreated = true;
    return true;
}

bool ATProtocol::sendCommand(QByteArray dataRequest, bool getResponse, int timeResponse, QByteArray& dataResponse, int timeSleep)
{
    bool respOk = false;
    if(this->isOpened()){
        //Если девайс открыт
        respOk = false;

        serialPort->write(dataRequest);
        this->printDataToHex(dataRequest);

        if(getResponse){
            //Если нам нужен респонс
            bool ret = serialPort->waitForReadyRead(timeResponse);
            if (ret){
                //Есть ответ
                dataResponse.append(serialPort->readAll());
                this->printDataToHex(dataResponse);
                respOk = true;
            }else{
                respOk = false;
            }
        }

        //Задержка после команды
        if(timeSleep) this->msleep(timeSleep);
        return respOk;
    }
    return false;
}

bool ATProtocol::readPort(QByteArray& tempAnswer)
{
    bool bResp = serialPort->waitForReadyRead(waittimeforans);

    if(bResp){
        tempAnswer = serialPort->readAll();
        if(Debugger) qDebug() << "\n<<---------Response\n";
        QString str(tempAnswer);
        if(Debugger) qDebug() << str;
        this->printDataToHex(tempAnswer);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool ATProtocol::getAnswer(QByteArray &aData, bool &codecError, int aAddRepeatCount, int aPauseTime)
{
    QByteArray packetAnswer;

    // читаем из порта пока это необходимо
    for (int i = 0; i < aAddRepeatCount + 1; ++i)
    {
        QCoreApplication::processEvents();
        QByteArray tempAnswer;
        if (!readPort(tempAnswer))
            return false;

        // если пакет пуст и дополнительных чтений нет, то пишем сообщение об ошибке
        if (!tempAnswer.size() && !aAddRepeatCount)
        {
            // возможно, произошел дисконнект
            // в качестве ответных данных положим один пустой байт
            aData.push_back(QChar(CATProtocolConstants::EmptyByte).cell());
            return false;
        }
        packetAnswer.push_back(tempAnswer);

        // если необходима пауза - включаем её
        if (aPauseTime)
            this->msleep(aPauseTime);
    }

    // ошибка кодировки
    if(QString(packetAnswer).toLower().contains("error")){
        codecError = true;
        return false;
    }

    if (packetAnswer.size() < CATProtocolConstants::MinAnswerSize)
    {

        QString::number(CATProtocolConstants::MinAnswerSize);

        return false;
    }

    // расшифровываем пакет
    if (!unpacketData(packetAnswer, aData))
    {
        return false;
    }

    if (!aData.size())
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool ATProtocol::getCommandPacket(ModemProtocolCommands::Enum aCommand, const QByteArray& aCommandData,
                                  QByteArray& aPacketCommand)
{
    Q_UNUSED(aCommandData)

    qDebug() << "----------- aCommand ----------- " << aCommand;
    switch (aCommand)
    {
    //----------------------------------------
    case ModemProtocolCommands::GetSignalQuality :
    {
        aPacketCommand.push_back(CATProtocolCommands::SignalQuality.toLatin1());
    }
    break;

    case ModemProtocolCommands::CmdRestart :
    {
        aPacketCommand.push_back(CATProtocolCommands::Restart.toLatin1());
    }
    break;

        //----------------------------------------
    case ModemProtocolCommands::GetComment :
    {
        aPacketCommand.push_back(CATProtocolCommands::Comment.toLatin1());
    }
    break;
    case ModemProtocolCommands::GetSimNumber :
    {
        QString request = CATProtocolCommands::UssdRequest;
        request = request.arg(this->simNumberRequest);
        aPacketCommand.push_back(request.toLatin1());
    }
    break;
    //----------------------------------------
    case ModemProtocolCommands::Reset :
    {
        aPacketCommand.push_back(CATProtocolCommands::ResetSettings.toLatin1());
    }
    break;
    //----------------------------------------
    case ModemProtocolCommands::GetOperator :
    {
        aPacketCommand.push_back(CATProtocolCommands::GetOperator.toLatin1());
    }
    break;
    //----------------------------------------
    case ModemProtocolCommands::OffEcho :
    {
        aPacketCommand.push_back(CATProtocolCommands::OffEcho.toLatin1());
    }
    break;
    //----------------------------------------
    case ModemProtocolCommands::IsPin :
    {
        aPacketCommand.push_back(CATProtocolCommands::IsPin.toLatin1());
    }
    break;
    //----------------------------------------
    case ModemProtocolCommands::Identification :
    {
        aPacketCommand.push_back(CATProtocolCommands::Identification.toLatin1());
    }
    break;
    //----------------------------------------
    case ModemProtocolCommands::GetBalance :
    {

        QString request = CATProtocolCommands::UssdRequest;
        request = request.arg(this->balanceRequest);
        aPacketCommand.push_back(request.toLatin1());
    }
    break;
    case ModemProtocolCommands::CmdStateSMS :
    {
        QString request = CATProtocolCommands::SmsState;
        aPacketCommand.push_back(request.toLatin1());
    }
    break;
    case ModemProtocolCommands::CmdDellAllSms :
    {
        QString request = CATProtocolCommands::DellAllSms;
        aPacketCommand.push_back(request.toLatin1());
    }
    break;
    case ModemProtocolCommands::CmdSetNumberSMS :
    {
        QString vrm = CATProtocolCommands::SmsSend;
        QString request = vrm.arg(this->GetLengthSMS);
        aPacketCommand.push_back(request.toLatin1());
    }
    break;
    case ModemProtocolCommands::CmdTextIntersetSMS :
    {
        QString request = this->textToSendSms;
        aPacketCommand.push_back(request.toLatin1());
        aPacketCommand.push_back(0x1A);
    }
    break;
    case ModemProtocolCommands::CmdGetAllSms :
    {
        QString request = CATProtocolCommands::GetAllInputSms;
        aPacketCommand.push_back(request.toLatin1());
    }
    break;
    default:
    {
        return false;
    };
    };
    return true;
}


QString ATProtocol::getCommandString(ModemProtocolCommands::Enum aCommand)
{
    QString command;
    switch (aCommand)
    {
    case ModemProtocolCommands::GetSignalQuality : { command = ModemCmd::GetSignalQuality; } break;
    case ModemProtocolCommands::GetOperator      : { command = ModemCmd::GetOperator;      } break;
    case ModemProtocolCommands::GetBalance       : { command = ModemCmd::GetBalance;       } break;
    case ModemProtocolCommands::Identification   : { command = ModemCmd::Identification;   } break;
    case ModemProtocolCommands::OffEcho          : { command = ModemCmd::OffEcho;          } break;
    case ModemProtocolCommands::GetComment       : { command = ModemCmd::GetComment;       } break;
    case ModemProtocolCommands::GetSimNumber     : { command = ModemCmd::GetSimNumber;     } break;
    case ModemProtocolCommands::Reset            : { command = ModemCmd::Reset;            } break;
    case ModemProtocolCommands::CmdRestart       : { command = ModemCmd::CmdRestart;       } break;
    default                                      : { command = ModemCmd::Uknown; }
    };
    return command;
}

//--------------------------------------------------------------------------------
bool ATProtocol::prepareAnswer(ModemProtocolCommands::Enum aCommand, QByteArray& aAnswer)
{
    switch(aCommand)
    {
    case ModemProtocolCommands::GetOperator:
    {
        //                        if(Debuger) qDebug() << "Protocol AT: GetOperator...";
        QString operatorString(aAnswer);
        // вытаскиваем оператора из окружения кавычек
        int indexFirstQuote = operatorString.indexOf("\"");
        int indexSecondQuote = -1;
        if (indexFirstQuote != -1)
            indexSecondQuote = operatorString.indexOf("\"", indexFirstQuote + 1);

        if ((indexFirstQuote != -1) && (indexSecondQuote != -1))
            operatorString = operatorString.mid(indexFirstQuote + 1, indexSecondQuote - indexFirstQuote - 1);
        else
        {
            //                                if(Debuger) qDebug() << "Protocol AT: Invalid format of operator";
            return false;
        }
        aAnswer = operatorString.toLatin1();
        //                        if(Debuger) qDebug() << "Operator = " << operatorString;
        nowProviderSim = operatorString;
    }
    break;
    //---------------------------------------
    case ModemProtocolCommands::GetSignalQuality:
    {
        //                        if(Debuger) qDebug() << "Protocol AT: GetSignalQuality...";
        QString signalString(aAnswer);
        // вытаскиваем оператора из окружения 1) пробел 2) запятая
        int indexFirst = signalString.indexOf(" ");
        int indexSecond = -1;
        if (indexFirst != -1)
            indexSecond = signalString.indexOf(",");

        if ((indexFirst != -1) && (indexSecond != -1))
            signalString = signalString.mid(indexFirst + 1, indexSecond - indexFirst - 1);
        else
        {
            //                                if(Debuger) qDebug() << "Protocol AT: Invalid format of operator";
            return false;
        }
        double signal = signalString.toDouble();
        signal = signal > 100 ? 100 : signal;
        aAnswer = QString::number(signal).toLatin1();
        //                        if(Debuger) qDebug() << "Signal = " << aAnswer << "%";
        nowSignalQuality = "";
        nowSignalQuality.append(aAnswer);
    }
    break;

        //Comment
    case ModemProtocolCommands::GetComment:
    {
        //                        if(Debuger) qDebug() << "Protocol AT: GetComment...";
        QString responseString(aAnswer);

        int indexFirst = responseString.indexOf("\n");
        //if(Debuger) qDebug() << "indexFirst = " << indexFirst;

        nowModemComment = responseString.mid(0,indexFirst - 1);
        nowModemComment.replace("\n","");

    }
    break;

        //SimNumber
    case ModemProtocolCommands::GetSimNumber:
    {
        QString responseString(aAnswer);

        m_getBalanceRegExp.indexIn(responseString);
        nowSimNumber = m_getBalanceRegExp.cap();

        int indexFirstQuote = responseString.indexOf("+CUSD: 0,\"");
        int indexSecondQuote = -1;
        if (indexFirstQuote != -1)
            indexSecondQuote = responseString.indexOf("\"", indexFirstQuote + 10);

        responseString = responseString.mid(indexFirstQuote + 10, indexSecondQuote - indexFirstQuote - 10).trimmed();

        QString parsedResponse = responseString;

        // decodeGSM7bit
        if(nowSimNumber.trimmed() == ""){
            responseString = decodeGSM7bit(parsedResponse);

            m_getBalanceRegExp.indexIn(responseString);
            nowSimNumber = m_getBalanceRegExp.cap();
        }

        // decodeUcs2
        if(nowSimNumber.trimmed() == ""){
            responseString = decodeUcs2(parsedResponse);

            m_getBalanceRegExp.indexIn(responseString);
            nowSimNumber = m_getBalanceRegExp.cap();
        }

    }
    break;

        //---------------------------------------
    case ModemProtocolCommands::GetBalance:
    {

        QString responseString(aAnswer);

        m_getBalanceRegExp.indexIn(responseString,this->position);
        nowSimBalance = m_getBalanceRegExp.cap(this->position);

        int indexFirstQuote = responseString.indexOf("+CUSD: 0,\"");
        int indexSecondQuote = -1;
        if (indexFirstQuote != -1)
            indexSecondQuote = responseString.indexOf("\"", indexFirstQuote + 10);

        responseString = responseString.mid(indexFirstQuote + 10, indexSecondQuote - indexFirstQuote - 10).trimmed();

        QString parsedResponse = responseString;

        // decodeGSM7bit
        if(nowSimBalance.trimmed() == ""){
            responseString = decodeGSM7bit(parsedResponse);

            m_getBalanceRegExp.indexIn(responseString,this->position);
            nowSimBalance = m_getBalanceRegExp.cap(this->position);
        }

        // decodeUcs2
        if(nowSimBalance.trimmed() == ""){
            responseString = decodeUcs2(parsedResponse);
            //                        responseString = "Tavozun: -6.10 TJS. 0.75 TJS/ruz";
            m_getBalanceRegExp.indexIn(responseString,this->position);
            nowSimBalance = m_getBalanceRegExp.cap(this->position);
        }

    }
    break;
    case ModemProtocolCommands::CmdGetAllSms:
    {

        QString smsString(aAnswer);
        qDebug() << "_______GET ALL SMS_______";
        qDebug() << smsString;

    }
    break;
    default:
        break;
    };
    return true;
}

//--------------------------------------------------------------------------------
bool ATProtocol::getStatusInfo(SModemStatusInfo& aStatusInfo)
{
    QByteArray commandData;
    QByteArray answerData;

    m_state = ModemStates::Unknown;
    m_error = ModemErrors::Unknown;

    // Статус пока будем определять только по идентификации, чтобы определить наличие коннекта с модемом
    // TO DO в дальнейшем необходимо доделать
    bool isConnect = true;
    if (!processCommand(ModemProtocolCommands::Identification, commandData, answerData))
    {
        //if(Debuger) qDebug() << "Protocol AT: error processing command \"Identification\"";


        // ошибка, возможно произошла из-за дисконнекта.
        // проверим данные на пустой байт
        if (answerData.size() && answerData.at(0) == CATProtocolConstants::EmptyByte)
        {
            // так и есть, уставливаем статус в "недоступен"
            m_state = ModemStates::Error;
            m_error = ModemErrors::NotAvailable;
            isConnect = false;
            //if(Debuger) qDebug() << "Protocol AT: NotAvailabled";
        }
        else
            return false;
    }
    else
        m_state = ModemStates::Initialize;

    aStatusInfo.state    = m_state;
    aStatusInfo.error    = m_error;

    return isConnect;
}


QString ATProtocol::octet(QString hexString)
{
    bool ok;

    return QString("%1").arg(hexString.toULongLong(&ok, 16), 8, 2, QChar('0'));
}

QString ATProtocol::encodeUcs2(QString msg)
{

    QByteArray encodedText = QAtUtils::codec("ucs2")->fromUnicode(msg);

    return encodedText;
    //    QByteArray hex = QByteArray::fromHex(msg.toLocal8Bit());

    //    QString str;
    //    for (int i = 0; i < hex.size(); i += 2) {
    //        ushort u = (hex[i] << 8) | hex[i + 1];
    //        str.append(QString::fromUtf16(&u, 1));
    //    }

    //    return str;
}

QString ATProtocol::decodeUcs2(QString hexString){

    QByteArray input = hexString.toUtf8();
    QString decodedText = QAtUtils::codec("ucs2")->toUnicode(input);

    return decodedText;
}

QString ATProtocol::encodeGSM7bit(QString msg)
{
    QByteArray smsBytes = msg.toLocal8Bit();

    //Get bytes for this SMS string
    uchar mask = 0;
    uchar shiftedMask = 0;
    uchar bitsRequired = 0;
    int encodedMessageIndex = 0;
    int shiftCount = 0;

    //calculating new encoded message length

    int arrayLength = (int)(msg.length() - qFloor(((double)msg.length() / 8)));

    QByteArray encodedMessage;
    encodedMessage.resize(arrayLength);

    for (int i = 0; i < smsBytes.length(); i++)
    {
        mask = (uchar)((mask * 2) + 1);
        if (i < smsBytes.length()-1)
        {
            bitsRequired = (uchar)(smsBytes[i + 1] & mask);
            shiftedMask = (uchar)(bitsRequired << (7 - shiftCount));
            encodedMessage[encodedMessageIndex] = (uchar)(smsBytes[i] >> shiftCount);
            encodedMessage[encodedMessageIndex] = (uchar)(encodedMessage[encodedMessageIndex] | shiftedMask);
        } else {
            //last byte
            encodedMessage[encodedMessageIndex] = (uchar)(smsBytes[i] >> shiftCount);
        }
        encodedMessageIndex++;
        shiftCount++;
        //reseting the cycle when 1 ASCII is completely packed
        if (shiftCount == 7)
        {
            i++;
            mask = 0;
            shiftCount = 0;
        }
    }

    return encodedMessage.toHex();
}

QString ATProtocol::decodeGSM7bit(QString hexString) {

    QString septets = "";
    for(int i = hexString.length()-2; i >= 0; i-=2){
        QString reverse = hexString.mid(i, 2);
        septets += octet(reverse);
    }

    bool ok;

    QString decodedText = "";

    for(int i = septets.length()-7; i >= 0; i-=7){
        QString reverse = septets.mid(i, 7);
        int num = reverse.toInt(&ok, 2);

        if(num != 27){
            decodedText += QGsmCodec::singleToUnicode(num);
        }
    }

    return decodedText.trimmed();
}
