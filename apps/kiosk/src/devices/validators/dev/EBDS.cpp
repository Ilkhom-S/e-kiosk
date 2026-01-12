// Project
#include "EBDS.h"

#include <QtCore/QBitArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtMath>

EBDS::EBDS(QObject *parent) : BaseValidatorDevices(parent) {
  preDateTime = QDateTime::currentDateTime().addSecs(-1);

  validatorLogEnable = false;
  mACK = false;
  mEnabled = false;
  hasDBError = false;
  escrowed = false;
  maxSumReject = false;
  nominalSum = 0;
}

bool EBDS::OpenPort() {
  this->openPort();
  return is_open;
}

void EBDS::sendStatusTo(int sts, QString comment) {
  if (sts != status) {
    status = sts;
    emit this->emitStatus(status, comment);
  }
  return;
}

bool EBDS::openPort() {
  if (devicesCreated) {
    is_open = false;
    serialPort->setPortName(comName);

    if (serialPort->open(QIODevice::ReadWrite)) {
      is_open = false;

      if (!serialPort->setDataBits(QSerialPort::Data7))
        return false;
      if (!serialPort->setParity(QSerialPort::EvenParity))
        return false;
      if (!serialPort->setStopBits(QSerialPort::OneStop))
        return false;
      if (!serialPort->setFlowControl(QSerialPort::NoFlowControl))
        return false;
      //            if
      //            (!serialPort->setCharIntervalTimeout(ValidatorConstants::EBDS_CharTimeOut))
      //            return false;
      if (!serialPort->setBaudRate(QSerialPort::Baud9600))
        return false;

      is_open = true;
    } else {
      is_open = false;
      statusDevices = ValidatorErrors::PortError;
    }
  } else {
    is_open = false;
    statusDevices = ValidatorErrors::DevicesError;
  }

  return is_open;
}

bool EBDS::isItYou() {

  OpenPort();

  if (isOpened()) {
    QByteArray respData;

    if (execCommand(ValidatorCommands::Idintification, respData)) {
      auto type = QString::fromUtf8(respData).trimmed();
      if (!type.isEmpty()) {
        parseIdentification(respData);
        return true;
      }
    }

    this->closePort();
  }

  return false;
}

bool EBDS::CmdGetStatus() {

  QByteArray respData;

  this->execCommand(ValidatorCommands::Poll, respData);

  if (!respData.isEmpty()) {
    if (checkBit(respData[0], EBDSConstruct::State_0::Escrowed)) {

      int escrowNominal = getNominal(respData);

      if (escrowNominal > 0) {
        emit emitLog(
            0, "EBDS",
            QString("Определена купюра %1 сум (Escrow)").arg(escrowNominal));

        stackBill();
      } else {
        returnBill();
      }

      this->execCommand(ValidatorCommands::Poll, respData);
    }

    int nominal = 0;
    nominal = this->readPollInfo(respData);

    // Проверяем есть ли номинал и нет ли повторений
    if (nominal > 0) {
      emit this->emitNominal(nominal);
    }
  }

  QCoreApplication::processEvents();
  return true;
}

uchar EBDS::calcCRC(const QByteArray &aData) {
  uchar sum = aData[1];

  for (int i = 2; i < aData.size() - 1; ++i) {
    sum ^= static_cast<uchar>(aData[i]);
  }

  return sum;
}

QByteArray EBDS::makeCustomRequest(const QByteArray &commandData) {
  QByteArray request;

  request.append(EBDSConstruct::Prefix);
  request.append(char(4 + commandData.size()));
  request.append(commandData[0] | char(mACK));
  request.append(commandData.mid(1));
  request.append(EBDSConstruct::Postfix);
  request.append(calcCRC(request));

  mACK = !mACK;

  return request;
}

QByteArray EBDS::pollRequest(const char aAction) {
  QByteArray commandData;

  commandData += EBDSConstruct::Commands::Host2Validator;

  commandData.append(mEnabled ? '\x7F' : '\x00');
  commandData.append(EBDSConstruct::Byte1 | aAction);
  commandData.append(EBDSConstruct::Byte2);

  return makeCustomRequest(commandData);
}

bool EBDS::execCommand(int cmdType, QByteArray &cmdResponse) {
  try {
    if (is_open) {

      QByteArray cmdRequest;
      bool needAnswer = true;

      switch (cmdType) {

      // перезагрузка купюрника
      case ValidatorCommands::Reset:
        cmdRequest = this->makeCustomRequest(EBDSConstruct::Commands::Reset);
        this->toLogingValidator(0, cmdRequest, "Reset");
        needAnswer = false;
        break;

      case ValidatorCommands::GetNominalTable:
        //                    cmdRequest =
        //                    this->makeCustomRequest(EBDSConstruct::PABillValidator,EBDSConstruct::CCGetBillTable,0);
        break;

      case ValidatorCommands::SetEnabled: {
        mEnabled = true;
        cmdRequest = pollRequest();
        this->toLogingValidator(0, cmdRequest, "SetEnabled");
      } break;

      case ValidatorCommands::Poll:
        cmdRequest = pollRequest();
        this->toLogingValidator(0, cmdRequest, "Poll");
        break;

      case ValidatorCommands::Idintification:
        cmdRequest = this->makeCustomRequest(EBDSConstruct::Commands::GetType);
        this->toLogingValidator(0, cmdRequest, "Idintification");
        break;

      case ValidatorCommands::SetSecurity:
        break;

      case ValidatorCommands::ACK:
        break;

      case ValidatorCommands::SetDisabled: {
        mEnabled = false;
        cmdRequest = pollRequest();
        this->toLogingValidator(0, cmdRequest, "SetDisabled");
      } break;

      case ValidatorCommands::Return:
        cmdRequest = pollRequest(EBDSConstruct::Return);
        this->toLogingValidator(0, cmdRequest, "Return bill");
        break;

      case ValidatorCommands::Stack:
        cmdRequest = pollRequest(EBDSConstruct::Stack);
        this->toLogingValidator(0, cmdRequest, "Stack bill");
        break;
      }

      QByteArray answer;

      TResult result = processCommand(cmdRequest, answer, needAnswer);

      if (result == CommandResult::OK) {
        if (answer.count() == 0) {
          emit emitLog(2, "VALIDATOR", "Empty answer");
          return false;
        }

        cmdResponse = answer;
        return true;
      }

      if (!result) {
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
    if (debugger)
      qDebug() << "Protocol CCNet: Exception : [execCommand] "
               << QString(e.what());
    return false;
  }

  return true;
}

TResult EBDS::processCommand(QByteArray &aCommandData, QByteArray &aAnswerData,
                             bool aNeedAnswer) {
  if (isOpened()) {
    serialPort->write(aCommandData);
    serialPort->waitForBytesWritten(100);
  } else {
    return CommandResult::Port;
  }

  if (!aNeedAnswer) {
    return CommandResult::OK;
  }

  //    this->msleep(EBDSConstruct::AnswerTimeout);

  if (!getAnswer(aAnswerData)) {
    return CommandResult::Port;
  } else if (aAnswerData.isEmpty()) {
    return CommandResult::NoAnswer;
  } else if (!check(aCommandData, aAnswerData)) {
    return CommandResult::Protocol;
  }

  aAnswerData = aAnswerData.mid(2, aAnswerData.size() - 4);
  aAnswerData[0] = aAnswerData[0] & ~EBDSConstruct::ACKMask;

  if (!aAnswerData.isEmpty()) {
    int index =
        aAnswerData.startsWith(EBDSConstruct::Commands::Extended) ? 2 : 1;
    aAnswerData = aAnswerData.mid(index);
  }

  return CommandResult::OK;
}

bool EBDS::getAnswer(QByteArray &aAnswer) {
  aAnswer.clear();
  QByteArray data;
  uchar length = 0;

  QElapsedTimer clockTimer;
  clockTimer.restart();

  do {
    data.clear();

    if (!serialPort->waitForReadyRead(200)) {
      if (debugger)
        qDebug() << "waitForReadyRead false";
    }

    // Есть ответ
    data = serialPort->readAll();

    aAnswer.append(data);

    if (aAnswer.size() > 1) {
      length = aAnswer[1];
    }
  } while ((clockTimer.elapsed() < EBDSConstruct::AnswerTimeout) &&
           ((aAnswer.size() < length) || !length));

  if (debugger)
    qDebug() << QString("EBDS: << {%1}").arg(aAnswer.toHex().data());

  return true;
}

bool EBDS::check(const QByteArray &cmdRequest, const QByteArray &cmdResponse) {
  // проверяем первый байт
  if (cmdResponse[0] != EBDSConstruct::Prefix) {
    emit emitLog(2, "EBDS", "Invalid first byte (prefix)");
    return false;
  }

  // проверяем последний байт
  if (cmdResponse.right(2)[0] != EBDSConstruct::Postfix) {
    emit emitLog(2, "EBDS", "Invalid last byte (postfix)");
    return false;
  }

  // вытаскиваем и проверяем длину сообщения
  int length = cmdResponse[1];

  if (length != cmdResponse.size()) {
    emit emitLog(2, "EBDS", "Invalid length of the message");
    return false;
  }

  // проверяем контрольную сумму
  if (calcCRC(cmdResponse.left(cmdResponse.size() - 1)) !=
      cmdResponse[length - 1]) {
    emit emitLog(2, "EBDS", "Invalid CRC of the message");
    return false;
  }

  char commandACK = cmdRequest[2] & EBDSConstruct::ACKMask;
  char answerACK = cmdResponse[2] & EBDSConstruct::ACKMask;

  // проверяем тип сообщения и ACK
  if (commandACK != answerACK) {
    emit emitLog(2, "EBDS", "Invalid ACK of the message");
    return false;
  }

  return true;
}

bool EBDS::CmdRestart() {
  //    CmdGetStatus();
  //    msleep(500);

  //    CmdGetStatus();
  //    msleep(500);

  //    CmdGetStatus();
  //    msleep(500);

  //    QByteArray respData;

  //    this->execCommand(ValidatorCommands::Reset,false,200,2000,respData);
  return true;
}

void EBDS::parseIdentification(QByteArray respData) {
  partNumber = QString::fromUtf8(respData);

  respData.clear();
  QByteArray cmdRequest =
      this->makeCustomRequest(EBDSConstruct::Commands::GetVariantName);
  TResult result = processCommand(cmdRequest, respData, true);

  if (result == CommandResult::OK) {
    partNumber += "/" + QString::fromUtf8(respData);
  }

  cmdRequest =
      this->makeCustomRequest(EBDSConstruct::Commands::GetVariantVersion);
  result = processCommand(cmdRequest, respData, true);

  if (result == CommandResult::OK) {
    partNumber += "/" + QString::fromUtf8(respData.mid(0, 9));
  }

  cmdRequest =
      this->makeCustomRequest(EBDSConstruct::Commands::GetSerialNumber);

  result = processCommand(cmdRequest, respData, true);

  if (result == CommandResult::OK) {
    serialNumber = QString::fromUtf8(respData);
  }
}

void EBDS::CmdStartPoll() {
  validatorLogEnable = true;

  preDateTime = QDateTime::currentDateTime().addSecs(-1);
  ;

  stopPoll = false;
  QByteArray respData;

  //    Активируем bill
  this->execCommand(ValidatorCommands::SetEnabled, respData);
  this->toLogingValidator(1, respData, "SetEnabled Response");

  while (!stopPoll) {

    this->execCommand(ValidatorCommands::Poll, respData);
    this->toLogingValidator(1, respData, "Poll Response");

    if (!respData.isEmpty()) {

      if (checkBit(respData[0], EBDSConstruct::State_0::Escrowed)) {

        escrowed = true;

        int escrowNominal = getNominal(respData);

        if (escrowNominal > 0) {
          if (hasDBError) {
            this->execCommand(ValidatorCommands::Return, respData);
            emit emitLog(0, "EBDS",
                         QString("Возвращаем купюру %1 сум, из за ошибки БД")
                             .arg(escrowNominal));
            this->setReturnNominalState(true);
          } else if (maxSumReject && (nominalSum + escrowNominal > maxSum)) {
            returnBill();
            emit emitLog(0, "EBDS",
                         QString("Возвращаем купюру %1 сум, так как достигнута "
                                 "максимальная сумма %2")
                             .arg(escrowNominal)
                             .arg(maxSum));
            this->setReturnNominalState(true);
          } else {
            nominalSum += escrowNominal;
            emit emitLog(0, "EBDS",
                         QString("Определена купюра %1 сум (Escrow)")
                             .arg(escrowNominal));
            stackBill();
          }
        } else {
          returnBill();
        }

        this->execCommand(ValidatorCommands::Poll, respData);
        this->toLogingValidator(1, respData, "Poll Response");
      }

      int nominal = readPollInfo(respData);

      // Проверяем есть ли номинал и нет ли повторений
      if (nominal > 0) {
        QDateTime now = QDateTime::currentDateTime();

        qint64 sicPoint = preDateTime.msecsTo(now);

        if (sicPoint < 0) {
          sicPoint = 1000;
        }

        if (sicPoint > 500 && escrowed) {
          this->toLogingValidator(
              1, " ", QString("Вставлена купюра %1 сум").arg(nominal));
          escrowed = false;

          emit this->emitNominal(nominal);
        } else {
          emit emitNominalDuplicate(nominal);
        }

        preDateTime = now;
      }
    }
  }

  this->msleep(10);
  this->CmdStopPoll();
}

void EBDS::CmdStopPoll() {
  QByteArray respData;
  this->execCommand(ValidatorCommands::SetDisabled, respData);
  this->toLogingValidator(1, respData, "SetDisabled Response");

  validatorLogEnable = false;
  nominalSum = 0;

  QCoreApplication::processEvents();
}

void EBDS::returnBill() {
  QByteArray respData;
  this->execCommand(ValidatorCommands::Return, respData);
  this->toLogingValidator(1, respData, "Return bill Response");
}

void EBDS::stackBill() {
  QByteArray respData;
  this->execCommand(ValidatorCommands::Stack, respData);
  this->toLogingValidator(1, respData, "Stack bill Response");

  readPollInfo(respData);

  emit emitLog(0, "EBDS", QString("Отправляем команду на укладку (Stack)"));
}

int EBDS::getNominal(QByteArray respData) {
  if (respData.size() < EBDSConstruct::NominalSize) {
    //        qDebug() << QString("Log << Too small answer size = %1 for
    //        nominal, need %2
    //        minimum").arg(respData.size()).arg(EBDSConstruct::NominalSize);
    return 0;
  }

  int nominal = respData.mid(10, 3).toInt() *
                int(qPow(10, respData.mid(13, 3).toDouble()));
  QString currency = QString::fromUtf8(respData.mid(7, 3));

  if (nominal < 1000 || nominal > 200000 ||
      !currency.startsWith("UZ", Qt::CaseInsensitive)) {
    emit emitLog(0, "EBDS",
                 QString("Не поддерживаемая купюра %1").arg(nominal));
    return 0;
  }

  return nominal;
}

int EBDS::readPollInfo(QByteArray byte) {
  QByteRef byte0 = byte[0];
  QByteRef byte1 = byte[1];

  if (checkBit(byte0, EBDSConstruct::State_0::Stacked)) {

    this->setBoolingDlgState(false);
    this->setReturnNominalState(false);

    int nominal = getNominal(byte);

    return nominal;
  }

  if (byte0 == '\x01') {
    if (checkBit(byte1, EBDSConstruct::State_1::LRCPresent)) {
      this->sendStatusTo(VStatus::Success::Ok, QString("Idling"));
    } else {
      this->sendStatusTo(VStatus::Errors::BadStackerPosition,
                         QString("Открыта касета купюроприемника"));
      this->setBoolingDlgState(false);
      return 0;
    }
  }

  if (checkBit(byte0, EBDSConstruct::State_0::Accepting)) {
    this->sendStatusTo(VStatus::Success::Ok, QString("Accepting"));
    this->setBoolingDlgState(true);
    this->setBoolingDlgState(false); /*qDebug() << "----Accepting----";*/
    return 0;
  }

  if (checkBit(byte0, EBDSConstruct::State_0::Stacking)) {
    this->sendStatusTo(VStatus::Success::Ok,
                       QString("Stacking")); /*qDebug() << "----Stacking----";*/
    return 0;
  }

  if (checkBit(byte0, EBDSConstruct::State_0::Returning)) {
    this->sendStatusTo(VStatus::Success::Ok, QString("Returning"));
    this->setBoolingDlgState(false); /*qDebug() << "----Returning----";*/
    return 0;
  }

  if (checkBit(byte0, EBDSConstruct::State_0::Returned)) {
    emit emitLog(VStatus::Success::Ok, "EBDS",
                 QString("Купюра возвращена (Returned)"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte1, EBDSConstruct::State_1::Cheated)) {
    this->sendStatusTo(VStatus::Warning::Cheated,
                       QString("Попытка мошенничество (Cheated)"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte1, EBDSConstruct::State_1::Rejected)) {
    this->sendStatusTo(VStatus::Warning::Rejected,
                       QString("Купюра отклонена (Rejected)"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte1, EBDSConstruct::State_1::Jammed)) {
    this->sendStatusTo(VStatus::Errors::ValidatorJammed,
                       QString("Ошибка! Замятие купюры в купюроприемнике"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte1, EBDSConstruct::State_1::CassetteFull)) {
    this->sendStatusTo(VStatus::Errors::StackerFull,
                       QString("Переполнение бокса (Сделайте инкасацию)"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte1, EBDSConstruct::State_1::Paused)) {
    this->sendStatusTo(VStatus::Success::Ok,
                       QString("Thus Bill Validator stops motion. Paused"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte1, EBDSConstruct::State_1::Calibration)) {
    this->sendStatusTo(VStatus::Warning::Calibration,
                       QString("Калибровка (Calibration)"));
    this->setBoolingDlgState(false);
    return 0;
  }

  QByteRef byte2 = byte[2];

  if (checkBit(byte2, EBDSConstruct::State_2::PowerUp)) {
    this->sendStatusTo(VStatus::Success::Ok,
                       QString("Идет питание на Купюроприемник.(11)"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte2, EBDSConstruct::State_2::InvalidCommand)) {
    this->sendStatusTo(VStatus::Warning::InvalidCommand,
                       QString("Неверная команда (Invalid Command)"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte2, EBDSConstruct::State_2::Failure)) {
    this->sendStatusTo(
        VStatus::Errors::Failure,
        QString("Ошибка! Инициализации купюроприемника.(Failure)"));
    this->setBoolingDlgState(false);
    return 0;
  }

  QByteRef byte3 = byte[3];

  if (checkBit(byte3, EBDSConstruct::State_3::NoPushMode)) {
    this->sendStatusTo(VStatus::Warning::NoPushMode, QString("NoPush Mode"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte3, EBDSConstruct::State_3::FlashDownload)) {
    this->sendStatusTo(VStatus::Warning::FlashDownload,
                       QString("Flash Download"));
    this->setBoolingDlgState(false);
    return 0;
  }

  if (checkBit(byte3, EBDSConstruct::State_3::PreStack)) {
    this->sendStatusTo(VStatus::Warning::PreStack, QString("Pre Stack"));
    this->setBoolingDlgState(false);
    return 0;
  }

  return 0;
}

bool EBDS::checkBit(QByteRef bytes, int bit) {
  if (bit < 0 || bit > 7) {
    return false;
  }

  QBitArray bits(8);

  for (int b = 0; b < 8; b++) {
    bits.setBit(b, bytes & (1 << (7 - b)));
  }

  int i = 7 - bit;
  return bits.at(i);
}

void EBDS::setBoolingDlgState(bool sts) {
  Q_UNUSED(sts)
  //    if(sts_animate_dlg != sts){
  //        sts_animate_dlg = sts;
  //        emit this->emitAnimateStatus(sts_animate_dlg);
  //    }
}

void EBDS::setReturnNominalState(bool sts) {
  emit this->emitReturnNominalStatus(sts);
}

void EBDS::toLogingValidator(int status, QByteArray data, QString text) {
  if (validatorLogEnable) {
    emit emitValidatorLog(status, data, text);
  }
}
