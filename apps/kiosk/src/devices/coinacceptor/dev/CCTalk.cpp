// Project
#include "CCTalk.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

CCTalk::CCTalk() : BaseAcceptorDevices() {
  debugger = false;
  preDate = 0;

  checkCoin = true;
  coinAcceptorLogEnable = false;

  setBillConstData();
}

void CCTalk::setBillConstData() {
  billValue["TJ020A"] = 20;
  billValue["TJ020B"] = 20;
  billValue["TJ020F"] = 20;
  billValue["TJ020V"] = 20;
  billValue["TJ020C"] = 20;
  billValue["TJ20KA"] = 20;

  billValue["TJ025A"] = 25;
  billValue["TJ025B"] = 25;
  billValue["TJ025C"] = 25;

  billValue["TJ050A"] = 50;
  billValue["TJ050B"] = 50;
  billValue["TJ050C"] = 50;
  billValue["TJ50KA"] = 50;

  billValue["TJ100A"] = 100;
  billValue["TJ100B"] = 100;
  billValue["TJ100C"] = 100;
  billValue["TJ10KA"] = 100;

  billValue["TJ300A"] = 300;
  billValue["TJ300B"] = 300;
  billValue["TJ300C"] = 300;
  billValue["TJ30KA"] = 300;

  billValue["TJ500A"] = 500;
  billValue["TJ500B"] = 500;
  billValue["TJ500C"] = 500;
  billValue["TJ500KA"] = 500;
}

bool CCTalk::OpenPort() {
  openPort();
  return is_open;
}

void CCTalk::sendStatusTo(int sts, QString comment) {
  if (sts != status) {
    status = sts;
    emit emitStatus(status, comment);
  }

  return;
}

bool CCTalk::openPort() {
  if (devicesCreated) {
    // Если девайс для работы с портом обявлен
    is_open = false;

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
      //            if
      //            (!serialPort->setCharIntervalTimeout(AcceptorConstants::Microcoin_CharTimeOut))
      //            return false;
      if (!serialPort->setBaudRate(QSerialPort::Baud9600))
        return false;

      is_open = true;
    } else {
      is_open = false;
      statusDevices = AcceptorErrors::PortError;
    }
  } else {
    is_open = false;
    statusDevices = AcceptorErrors::DevicesError;
  }

  return is_open;
}

bool CCTalk::isItYou() {
  OpenPort();

  if (isOpened()) {
    QByteArray respData;

    if (execCommand(AcceptorCommands::ACK, respData)) {
      if (respData[0] == '\x01' && respData[1] == '\x00' &&
          respData[2] == '\x02') {

        execCommand(AcceptorCommands::Idintification, respData);

        QByteArray pn;

        for (int i = 4; i <= 6; i++) {
          pn.append(respData[i]);
        }

        pn.append(" ");

        execCommand(AcceptorCommands::RequestProductCode, respData);

        for (int i = 4; i <= 6; i++) {
          pn.append(respData[i]);
        }

        // Серийный номер
        execCommand(AcceptorCommands::RequestSerialNumber, respData);

        QByteArray serNum;
        serNum.append(respData[6]);
        serNum.append(respData[5]);
        serNum.append(respData[4]);

        bool ok;

        PartNumber = "";
        SerialNumber = "";

        PartNumber.append(pn);

        SerialNumber = QString("%1").arg(serNum.toHex().toInt(&ok, 16));
        return true;
      }
    }

    this->closePort();
  }

  return false;
}

bool CCTalk::CmdGetStatus() {
  QByteArray respData;

  if (execCommand(AcceptorCommands::Poll, respData)) {
    // на всякий случай если номинал не обнулился
    if (respData[1] == '\x0B') {
      if (respData[4] != '\x00' || respData[5] != '\x00' ||
          respData[6] != '\x00') {
        if (checkCoin) {
          clearCoin();
        }
      }
    } else {
      readPollInfo(respData);
    }
  }

  QCoreApplication::processEvents();
  return true;
}

void CCTalk::clearCoin() {
  checkCoin = false;

  QByteArray respData;

  execCommand(AcceptorCommands::Reset, respData);

  execCommand(AcceptorCommands::SetDisabled, respData);

  execCommand(AcceptorCommands::PerformSelfCheck, respData);

  execCommand(AcceptorCommands::Reset, respData);

  execCommand(AcceptorCommands::SetDisabled, respData);

  execCommand(AcceptorCommands::PerformSelfCheck, respData);

  execCommand(AcceptorCommands::PerformSelfCheck, respData);
}

ushort CCTalk::calcCRC8(const QByteArray &aData) {
  return -uchar(std::accumulate(aData.begin(), aData.end(), 0));
}

QByteArray CCTalk::makeCustomRequest(int cmd, const QByteArray &data) {
  QByteArray aCommandData;
  aCommandData.append(cmd);
  aCommandData.append(data);

  QByteArray request = aCommandData;
  uchar size = uchar(aCommandData.size() - 1);

  request.prepend(CCTalkConstruct::Address::Host);
  request.prepend(size);
  request.prepend(CCTalkConstruct::Address::CoinAcceptor);
  request.append(calcCRC8(request));

  return request;
}

bool CCTalk::execCommand(int cmdType, QByteArray &cmdResponse,
                         QByteArray data) {
  try {
    if (is_open) {
      QByteArray cmdRequest;

      switch (cmdType) {

      // перезагрузка купюрника
      case AcceptorCommands::Reset:
        cmdRequest = makeCustomRequest(CCTalkConstruct::ApReset, 0);
        break;

      case AcceptorCommands::GetNominalTable:
        cmdRequest = makeCustomRequest(CCTalkConstruct::ApRequestCoinId, data);
        break;

      case AcceptorCommands::SetEnabled: {
        data.clear();
        data.resize(2);

        data[0] = 0xFF;
        data[1] = 0xFF;

        cmdRequest =
            makeCustomRequest(CCTalkConstruct::ApModifyInhibitStatus, data);

        toCoinAcceptorLog(0, "", "Coin table\n" + billTableResp);
        toCoinAcceptorLog(0, cmdRequest, "SetEnabled");
      } break;

      case AcceptorCommands::Poll:
        cmdRequest =
            makeCustomRequest(CCTalkConstruct::ApReadBufferedBillEvents, 0);
        toCoinAcceptorLog(0, cmdRequest, "Poll");
        break;

      case AcceptorCommands::RequestProductCode:
        cmdRequest =
            makeCustomRequest(CCTalkConstruct::ApRequestProductCode, 0);
        break;

      case AcceptorCommands::RequestSerialNumber:
        cmdRequest =
            makeCustomRequest(CCTalkConstruct::ApRequestSerialNumber, 0);
        break;

      case AcceptorCommands::Idintification:
        cmdRequest =
            makeCustomRequest(CCTalkConstruct::ApRequestManufactureId, 0);
        break;

      case AcceptorCommands::SetSecurity: {
        data.clear();
        data.append(01);

        cmdRequest = makeCustomRequest(
            CCTalkConstruct::ApModifyMasterInhibitStatus, data);
      } break;

      case AcceptorCommands::ACK:
        cmdRequest = makeCustomRequest(CCTalkConstruct::ApSimplePoll, 0);
        toCoinAcceptorLog(0, cmdRequest, "SimplePoll");
        break;

      case AcceptorCommands::SetDisabled: {
        data.clear();
        data.resize(2);

        for (int i = 0; i < 2; i++) {
          data[i] = 0;
        }

        cmdRequest =
            makeCustomRequest(CCTalkConstruct::ApModifyInhibitStatus, data);
        toCoinAcceptorLog(0, cmdRequest, "SetDisabled");
      } break;

      case AcceptorCommands::Stack: {
        data.clear();
        data.append(01);
        cmdRequest = makeCustomRequest(CCTalkConstruct::ApAcceptBill, data);
      } break;

      case AcceptorCommands::PerformSelfCheck:
        cmdRequest = makeCustomRequest(CCTalkConstruct::APPerformSelfCheck, 0);
        break;
      }

      QByteArray answer;

      TResult result = processCommand(cmdRequest, answer);

      if (result == CommandResult::OK) {
        cmdResponse = answer;
        return true;
      }

      if (!result) {
        if (debugger)
          qDebug() << "result " << result;
      }

      if (result == CommandResult::Port) {
        emit emitLoging(2, "COIN ACCEPTOR", "Port error");
      }

      if (result == CommandResult::NoAnswer) {
        emit emitLoging(2, "COIN ACCEPTOR", "Empty answer");
      }

      if (result == CommandResult::Protocol) {
        emit emitLoging(2, "COIN ACCEPTOR", "Protocol error");
      }

      return false;
    }
  } catch (std::exception &e) {
    // if(Debuger) qDebug() << "Protocol CCNet: Exception : [execCommand] " <<
    // QString(e.what());
    return false;
  }

  return true;
}

TResult CCTalk::processCommand(const QByteArray &aCommandData,
                               QByteArray &aAnswerData) {
  aAnswerData.clear();

  bool nak = false;
  bool busy = false;
  int busyNAKRepeat = 0;

  do {
    if (busyNAKRepeat) {
      msleep(CCTalkConstruct::Timeouts::NAKBusy);
    }

    toDebug(QString("ccTalk: >> {%2}").arg(aCommandData.toHex().data()));

    if (!serialPort->write(aCommandData) ||
        !getAnswer(aAnswerData, aCommandData)) {
      return CommandResult::Port;
    }

    if (aAnswerData.isEmpty()) {
      return CommandResult::NoAnswer;
    } else if (!check(aAnswerData)) {
      return CommandResult::Protocol;
    }

    nak = (aAnswerData.size() == 1) && (aAnswerData[0] == CCTalkConstruct::NAK);
    busy =
        (aAnswerData.size() == 1) && (aAnswerData[0] == CCTalkConstruct::BUSY);

    if (nak || busy) {
      toDebug(QString("ccTalk: %1 in answer, %2")
                  .arg(nak ? "NAK" : "BYSY",
                       (busyNAKRepeat <= CCTalkConstruct::MaxBusyNAKRepeats)
                           ? "repeat sending the messsage"
                           : "cancel sending!"));
    }
  } while ((busy || nak) &&
           (++busyNAKRepeat < CCTalkConstruct::MaxBusyNAKRepeats));

  return (!busy && !nak) ? CommandResult::OK : CommandResult::Transport;
}

//--------------------------------------------------------------------------------
bool CCTalk::getAnswer(QByteArray &aAnswer, const QByteArray &aCommandData) {
  aAnswer.clear();

  QByteArray data;
  int length = -1;
  QElapsedTimer timer;
  timer.start();

  do {

    if (!serialPort->waitForReadyRead(150)) {
      toDebug("WaitForReadyRead false");
    }

    data = serialPort->readAll();

    aAnswer.append(data);

    for (int i = 0; i < 2; ++i) {
      length = -1;

      if (aAnswer.size() >= 2) {
        length = aAnswer[1];

        if (aAnswer.startsWith(aCommandData)) {
          aAnswer = aAnswer.mid(aCommandData.size());
        }
      }
    }
  } while ((timer.elapsed() < CCTalkConstruct::Timeouts::Reading) &&
           ((aAnswer.size() < (length + 5)) || (length == -1)));

  toDebug(QString("ccTalk: << {%2}").arg(aAnswer.toHex().data()));

  return true;
}

bool CCTalk::check(QByteArray &aAnswer) {
  if (aAnswer.size() < CCTalkConstruct::MinAnswerSize) {
    toDebug(QString("ccTalk: Too few bytes in answer = %1, need min = %2")
                .arg(aAnswer.size())
                .arg(CCTalkConstruct::MinAnswerSize));
    return false;
  }

  char destinationAddress = aAnswer[0];

  if (destinationAddress != CCTalkConstruct::Address::Host) {
    toDebug(QString("ccTalk: Wrong destination address = %1, need = %2")
                .arg(uchar(destinationAddress))
                .arg(uchar(CCTalkConstruct::Address::Host)));
    return false;
  }

  int length = aAnswer[1];
  int dataSize = aAnswer.size() - 5;

  if (length != dataSize) {
    toDebug(QString("ccTalk: Wrong length = %1, need = %2")
                .arg(length)
                .arg(dataSize));
    return false;
  }

  uchar header = uchar(aAnswer[3]);

  if (header && (header != CCTalkConstruct::NAK)) {
    toDebug(QString("ccTalk: Wrong header = %1, need = 0").arg(header));
    return false;
  }

  QByteArray answer = aAnswer.left(aAnswer.size() - 1);
  uchar dataCRC = calcCRC8(answer);
  uchar answerCRC = uchar(aAnswer[aAnswer.size() - 1]);

  if (dataCRC != answerCRC) {
    toDebug(QString("ccTalk: Wrong CRC = %1, need = %2")
                .arg(answerCRC)
                .arg(uchar(uchar(0) - dataCRC)));
    return false;
  }

  //    aAnswer = aAnswer.mid(3, 1 + aAnswer[1]);

  return true;
}

bool CCTalk::CmdRestart() {
  // if(Debuger) qDebug() << "\n-----------START CMD_RESET-----------\n";
  //    QByteArray respData;

  // this->execCommand(AcceptorCommands::Reset,true,20,20,respData);

  //    this->execCommand(AcceptorCommands::Idintification, true, 30, 30,
  //    respData);
  // if(Debuger) qDebug() << "\n----------END CMD_RESET-----------\n";
  return true;
}

void CCTalk::CmdInit() {
  QByteArray respData;
  execCommand(AcceptorCommands::Reset, respData);

  execCommand(AcceptorCommands::Reset, respData);

  execCommand(AcceptorCommands::SetDisabled, respData);

  execCommand(AcceptorCommands::PerformSelfCheck, respData);

  execCommand(AcceptorCommands::PerformSelfCheck, respData);

  QByteArray cmdResponse;

  billTable.clear();

  QString bill_table = "";

  for (int i = 1; i <= 16; i++) {

    QByteArray data;
    data.append(i);

    QByteArray nominal;

    if (execCommand(AcceptorCommands::GetNominalTable, cmdResponse, data)) {
      for (int j = 4; j <= 9; j++) {
        nominal.append(cmdResponse[j]);
      }
    }

    QString bill_value = "";
    bill_value.append(nominal);

    if (i == 5 && bill_value == "TJ50KA") {
      bill_value = "TJ500KA";
    }

    billTable[i] = bill_value.replace("......", "").trimmed();

    if (billTable[i] != "") {
      bill_table += QString("%1:%2  ").arg(i).arg(billTable[i]);
    } else {
      bill_table += QString("%1:--  ").arg(i);
    }

    if (i == 8)
      bill_table += "\n";
  }

  billTableResp = bill_table;

  msleep(500);

  emit emitBillTable(bill_table);

  QCoreApplication::processEvents();
}

void CCTalk::CmdStartPoll() {
  coinAcceptorLogEnable = true;

  stopPoll = false;

  QByteArray respData;

  coin_ev_counter = 0;
  events_in_queue = 0;

  execCommand(AcceptorCommands::SetEnabled, respData);

  while (!stopPoll) {
    if (execCommand(AcceptorCommands::Poll, respData)) {
      toCoinAcceptorLog(1, respData, "Poll Response");

      int nominal = 0;

      if (respData[1] == '\x0B') {
        nominal = readPollInfo(respData);
      }

      // Проверяем есть ли номинал и нет ли повторений
      if (nominal > 0) {
        QString vrmD = QDateTime::currentDateTime().toString("ddHHmmsszzz");

        double nowD = vrmD.toDouble();
        double sicPointDoublle = nowD - preDate;

        if (sicPointDoublle < 0) {
          sicPointDoublle *= -1;
        }

        if (sicPointDoublle > 500) {
          preDate = nowD;
          toCoinAcceptorLog(1, " ",
                            QString("Вставлена монета %1 дир.").arg(nominal));

          emit emitNominal(nominal);
        } else {
          emit emitNominalDuplicate(nominal);
        }
      }
    }
  }

  this->msleep(10);
  this->CmdStopPoll();
}

void CCTalk::CmdStopPoll() {
  QByteArray respData;

  execCommand(AcceptorCommands::Reset, respData);

  execCommand(AcceptorCommands::SetDisabled, respData);

  execCommand(AcceptorCommands::PerformSelfCheck, respData);

  execCommand(AcceptorCommands::Reset, respData);

  execCommand(AcceptorCommands::SetDisabled, respData);

  execCommand(AcceptorCommands::PerformSelfCheck, respData);

  execCommand(AcceptorCommands::PerformSelfCheck, respData);

  checkCoin = true;
  coinAcceptorLogEnable = false;

  QCoreApplication::processEvents();
}

int CCTalk::readPollInfo(QByteArray byte) {
  setBoolingDlgState(false);

  int vrmRespData = byte[4];
  int nominal = 0;

  if (byte[4] == '\x00') {
    coin_ev_counter = byte[4];
  }

  if (coin_ev_counter <= vrmRespData) {
    events_in_queue = vrmRespData - coin_ev_counter;
  } else {
    events_in_queue = 255 + vrmRespData - coin_ev_counter;
  }

  if (events_in_queue > 5) {
    events_in_queue = 5;
  }

  while (events_in_queue > 0) {

    if (byte[events_in_queue * 2 + 3] > '\x00') {
      nominal = billValue[billTable[byte[events_in_queue * 2 + 3]]];

      this->sendStatusTo(CCtalkStatus::Success::Ok, QString("0"));
      this->setBoolingDlgState(false);
    } else {

      switch (byte[events_in_queue * 2 + 4]) {
      case CCTalkConstruct::States::Accepting:
        this->sendStatusTo(CCtalkStatus::Success::Ok, QString("0"));
        this->setBoolingDlgState(false);
        break;

      case CCTalkConstruct::States::RejectCoin:
        this->sendStatusTo(CCtalkStatus::Errors::RejectCoin,
                           QString("RejectCoin"));
        this->setBoolingDlgState(false);
        break;

      case CCTalkConstruct::States::InhibitedCoin:
        this->sendStatusTo(CCtalkStatus::Errors::InhibitedCoin,
                           QString("InhibitedCoin"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::MultipleWindow:
        this->sendStatusTo(CCtalkStatus::Errors::MultipleWindow,
                           QString("MultipleWindow"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::WakeUpTimeout:
        this->sendStatusTo(CCtalkStatus::Errors::WakeUpTimeout,
                           QString("WakeUpTimeout"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::ValidationTimeout:
        this->sendStatusTo(CCtalkStatus::Errors::ValidationTimeout,
                           QString("ValidationTimeout"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CreditSensorTimeout:
        this->sendStatusTo(CCtalkStatus::Errors::CreditSensorTimeout,
                           QString("CreditSensorTimeout"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::SorterOptoTimeout:
        this->sendStatusTo(CCtalkStatus::Errors::SorterOptoTimeout,
                           QString("SorterOptoTimeout"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::SecondCloseCoinError:
        this->sendStatusTo(CCtalkStatus::Errors::SecondCloseCoinError,
                           QString("SecondCloseCoinError"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::AcceptGateNotReady:
        this->sendStatusTo(CCtalkStatus::Errors::AcceptGateNotReady,
                           QString("AcceptGateNotReady"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CreditSensorNotReady:
        this->sendStatusTo(CCtalkStatus::Errors::CreditSensorNotReady,
                           QString("CreditSensorNotReady"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::SorterNotReady:
        this->sendStatusTo(CCtalkStatus::Errors::SorterNotReady,
                           QString("SorterNotReady"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::RejectCoinNotCleared:
        this->sendStatusTo(CCtalkStatus::Errors::RejectCoinNotCleared,
                           QString("RejectCoinNotCleared"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::ValidationSensorNotReady:
        this->sendStatusTo(CCtalkStatus::Errors::ValidationSensorNotReady,
                           QString("ValidationSensorNotReady"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CreditSensorBlocked:
        this->sendStatusTo(CCtalkStatus::Errors::CreditSensorBlocked,
                           QString("CreditSensorBlocked"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::SorterOptoBlocked:
        this->sendStatusTo(CCtalkStatus::Errors::SorterOptoBlocked,
                           QString("SorterOptoBlocked"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CreditSequenceError:
        this->sendStatusTo(CCtalkStatus::Errors::CreditSequenceError,
                           QString("CreditSequenceError"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CoinGoingBackwards:
        this->sendStatusTo(CCtalkStatus::Errors::CoinGoingBackwards,
                           QString("CoinGoingBackwards"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CoinTooFast:
        this->sendStatusTo(CCtalkStatus::Errors::CoinTooFast,
                           QString("CoinTooFast"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CoinTooSlow:
        this->sendStatusTo(CCtalkStatus::Errors::CoinTooSlow,
                           QString("CoinTooSlow"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CoinOnStringMechanismActivated:
        this->sendStatusTo(CCtalkStatus::Errors::CoinOnStringMechanismActivated,
                           QString("CoinOnStringMechanismActivated"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::DCEOptoTimeout:
        this->sendStatusTo(CCtalkStatus::Errors::DCEOptoTimeout,
                           QString("DCEOptoTimeout"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::DCEOptoNotSeen:
        this->sendStatusTo(CCtalkStatus::Errors::DCEOptoNotSeen,
                           QString("DCEOptoNotSeen"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CreditSensorReachedTooEarly:
        this->sendStatusTo(CCtalkStatus::Errors::CreditSensorReachedTooEarly,
                           QString("CreditSensorReachedTooEarly"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::RejectCoinRepeatedSequentialTrip:
        this->sendStatusTo(
            CCtalkStatus::Errors::RejectCoinRepeatedSequentialTrip,
            QString("RejectCoinRepeatedSequentialTrip"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::RejectSlug:
        this->sendStatusTo(CCtalkStatus::Errors::RejectSlug,
                           QString("RejectSlug"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::RejectSensorBlocked:
        this->sendStatusTo(CCtalkStatus::Errors::RejectSensorBlocked,
                           QString("RejectSensorBlocked"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::GamesOverload:
        this->sendStatusTo(CCtalkStatus::Errors::GamesOverload,
                           QString("GamesOverload"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::MaxCoinMeterPulsesExceeded:
        this->sendStatusTo(CCtalkStatus::Errors::MaxCoinMeterPulsesExceeded,
                           QString("MaxCoinMeterPulsesExceeded"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::AcceptGateOpenNotClosed:
        this->sendStatusTo(CCtalkStatus::Errors::AcceptGateOpenNotClosed,
                           QString("AcceptGateOpenNotClosed"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::AcceptGateClosedNotOpen:
        this->sendStatusTo(CCtalkStatus::Errors::AcceptGateClosedNotOpen,
                           QString("AcceptGateClosedNotOpen"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::DataBlockRequest:
        this->sendStatusTo(CCtalkStatus::Errors::DataBlockRequest,
                           QString("DataBlockRequest"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::CoinReturnMechanismActivated:
        this->sendStatusTo(CCtalkStatus::Errors::CoinReturnMechanismActivated,
                           QString("CoinReturnMechanismActivated"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;

      case CCTalkConstruct::States::UnspecifiedAlarmCode:
        this->sendStatusTo(CCtalkStatus::Errors::UnspecifiedAlarmCode,
                           QString("UnspecifiedAlarmCode"));
        this->setBoolingDlgState(
            false); /*qDebug() << "----Returning----"; return 0;*/
        break;
      }
    }

    events_in_queue--;
  }

  coin_ev_counter = byte[4];

  if (nominal > 0) {
    this->setBoolingDlgState(true);
    return nominal;
  }

  return 0;
}

void CCTalk::setBoolingDlgState(bool sts) {
  Q_UNUSED(sts)

  //    if(sts_animate_dlg != sts){
  //        sts_animate_dlg = sts;
  //        emit this->emitAnimateStatus(sts_animate_dlg);
  //    }
}

void CCTalk::toCoinAcceptorLog(int state, QByteArray data, QString text) {
  if (coinAcceptorLogEnable) {
    emit emitCoinAcceptorLog(state, data, text);
  }
}

void CCTalk::toDebug(QString data) {
  if (debugger) {
    qDebug() << data;
  }
}
