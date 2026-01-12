
#include "CustomTG2480.h"

TG2480_PRINTER::TG2480_PRINTER(QObject *parent) : BasePrinterDevices(parent) {
  //    printer_name = "Custom-VKP80";
}

bool TG2480_PRINTER::OpenPrinterPort() {
  this->openPort();
  return is_open;
}

bool TG2480_PRINTER::openPort() {
  if (devicesCreated) {
    // Если девайс для работы с портом обявлен
    is_open = false;
    //    return is_open;
    // Даем девайсу название порта
    // if(Debuger)  qDebug() << "devicePort->setDeviceName(comName); - " <<
    // comName;
    serialPort->setPortName(comName);

    if (serialPort->open(QIODevice::ReadWrite)) {
      // Если Девайсу удалось открыть порт

      // Устанавливаем параметры открытия порта
      is_open = false;

      if (!serialPort->setDataBits(QSerialPort::Data8))
        return false;
      //                if (!devicePort->setParity(AbstractSerial::ParityNone))
      //                return false; if
      //                (!devicePort->setStopBits(AbstractSerial::StopBits1))
      //                return false; if
      //                (!devicePort->setFlowControl(AbstractSerial::FlowControlUndefined))
      //                return false; if
      //                (!serialPort->setCharIntervalTimeout(CMDTG2480::charTimeOut))
      //                return false;
      if (!serialPort->setBaudRate(QSerialPort::Baud19200))
        return false;

      // if(Debuger) qDebug() << "\nPrinter " << CMDTG2480::DeviceName << " to
      // Port " << devicePort->deviceName() << " open in " <<
      // devicePort->openMode();

      // if(Debuger) qDebug() << "\n= Defaults parameters =";
      // if(Debuger) qDebug() << "Device name            : " <<
      // devicePort->deviceName(); if(Debuger) qDebug() << "Baud rate : " <<
      // devicePort->baudRate(); if(Debuger) qDebug() << "Data bits : " <<
      // devicePort->dataBits(); if(Debuger) qDebug() << "Parity : " <<
      // devicePort->parity(); if(Debuger) qDebug() << "Stop bits              :
      // " << devicePort->stopBits(); if(Debuger) qDebug() << "Flow : " <<
      // devicePort->flowControl(); if(Debuger) qDebug() << "Char timeout, msec
      // : " << devicePort->charIntervalTimeout();

      is_open = true;

    } else {
      is_open = false;
      //            statusDevices = this->PortError;
      // if(Debuger) qDebug() << "Error opened serial device " <<
      // devicePort->deviceName();
    }
  } else {
    is_open = false;
    // if(Debuger) qDebug() << "Error create serial device " <<
    // devicePort->deviceName();
    //        statusDevices = this->DeviceError;
  }

  return is_open;
}

bool TG2480_PRINTER::isItYou() {
  int status = 0;
  bool result = isEnabled(status);
  this->closePort();
  return result;
}

bool TG2480_PRINTER::isEnabled(int status) {
  //        int status = 0;
  if (!getStatus(status))
    return false;
  return (status != PrinterState::PrinterNotAvailable);
}

bool TG2480_PRINTER::getStatus(int &aStatus) {
  aStatus = PrinterState::PrinterNotAvailable;
  // засылаем в порт команду самоидентификации
  QByteArray cmd;
  QByteArray answer;
  bool resp_data = false;

  cmd.push_back(CMDTG2480::PrinterStatusCommandFirstByte);
  cmd.push_back(CMDTG2480::PrinterStatusCommandSecondByte);

  if (!this->sendCommand(cmd, true, 200, resp_data, answer, 0)) {
    // if(Debuger) qDebug() << "error in sendPacketInPort()";
    return false;
  }

  if (answer.size() < 1) {
    // if(Debuger) qDebug() << QString("AV268::getStatus(): wrong size of
    // buffer. Buffer is: %1").arg(answer.data());
    return false;
  }

  uchar status = 0;
  // В некоторых случаях присылается больше 1 байта
  // тогда наш байт - второй
  if (answer.size() > 1) {
    status = answer[1];
  } else {
    status = answer[0];
  }

  // Проверим, что это наш статус
  if ((status & 0x10) || (status & 0x80)) {
    // Не наш принтер
    // if(Debuger) qDebug() << QString("AV268::getStatus(): wrong byte returned:
    // %1").arg(status);
    return false;
  }

  // Наш принтер
  aStatus = PrinterState::PrinterOK;
  if (status != CMDTG2480::PrinterNormalState) {
    // Error
    int code = status & CMDTG2480::PrinterTemperatureError;
    if (code > 0) {
      // Temperature error
      aStatus |= PrinterState::TemperatureError;
      // if(Debuger) qDebug() << "AV268::getStatus(): Temperature error";
    }
    code = status & CMDTG2480::PrinterNoPaperError;
    if (code > 0) {
      // No paper
      aStatus |= PrinterState::PaperEnd;
      // if(Debuger) qDebug() << "AV268::getStatus(): No paper";
    }
    code = status & CMDTG2480::PrinterHeadOpenError;
    if (code > 0) {
      // Printing head open
      aStatus |= PrinterState::PrintingHeadError;
      // if(Debuger) qDebug() << "AV268::getStatus(): Printing head open";
    }
    code = status & CMDTG2480::PrinterSystemError;
    if (code > 0) {
      // System error
      aStatus |= PrinterState::PrinterError;
      // if(Debuger) qDebug() << "AV268::getStatus(): System error";
    }
    code = status & CMDTG2480::PrinterDataReceiveError;
    if (code > 0) {
      // Data receive error
      aStatus |= PrinterState::PortError;
      // if(Debuger) qDebug() << "AV268::getStatus(): Data receive error";
    }
  }
  return true;
}

bool TG2480_PRINTER::initialize() {
  // засылаем в порт команду самоидентификации
  QByteArray cmd;
  cmd.push_back(CMDTG2480::PrinterStatusCommandFirstByte);
  cmd.push_back(CMDTG2480::PrinterInitCommandSecondByte);

  bool resp_data = false;
  QByteArray response;

  return sendCommand(cmd, true, 200, resp_data, response, 290);
}

bool TG2480_PRINTER::cut() {
  QByteArray cmd;

  cmd.push_back(CMDTG2480::PrinterStatusCommandFirstByte);
  cmd.push_back(CMDTG2480::PrinterCutCommandSecondByte);

  bool resp_data = false;
  QByteArray response;
  bool res = this->sendCommand(cmd, true, 200, resp_data, response, 50);

  return res;
}

void TG2480_PRINTER::getSpecialCharecters(QByteArray &printText) {
  // Устанавливаем если есть жирный фонт
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::FontTypeBold +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    asciiNull());

  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::CloseTagSymbol +
                            CScharsetParam::FontTypeBold +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    asciiNull());

  // Устанавливаем если есть двойной высоты фонт
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::FontTypeDoubleHeight +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    asciiNull());
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::CloseTagSymbol +
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
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::CloseTagSymbol +
                            CScharsetParam::FontTypeDoubleWidth +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    asciiNull());

  // Устанавливаем если есть курсивный фонт
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::FontTypeItalic +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    asciiNull());
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::CloseTagSymbol +
                            CScharsetParam::FontTypeItalic +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    asciiNull());

  // Устанавливаем если есть подчеркнутый фонт
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::FontTypeUnderLine +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    asciiNull());
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::CloseTagSymbol +
                            CScharsetParam::FontTypeUnderLine +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    asciiNull());

  // Если надо добавить проабел
  QByteArray probel;
  for (int i = 1; i <= leftMargin; i++) {
    probel.append(ASCII::Space);
  }
  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::ProbelCount +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    probel);

  // Добавляем звезды
  int col_z = (chekWidth - 11 - leftMargin) / 2;
  QByteArray star;

  for (int j = 1; j <= col_z; j++) {
    star.append("*");
  }

  printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                            CScharsetParam::StarCount +
                            CScharsetParam::CloseTagDelimiter)
                        .toUtf8(),
                    star);
}

bool TG2480_PRINTER::printCheck(const QString &aCheck) {
  // Меняем кодировку
  QByteArray printText;
  printText = this->encodingString(aCheck, CScodec::c_IBM866);

  // Вставляем если есть Подчеркнутый, Жирный, Курсивный... Шрифт
  this->getSpecialCharecters(printText);

  QByteArray answer;
  bool respData = false;

  return sendCommand(printText, false, 0, respData, answer, 0);
}

void TG2480_PRINTER::print(const QString &aCheck) {
  // установим размер шрифта
  QByteArray cmd;
  cmd.push_back(CMDTG2480::PrinterStatusCommandFirstByte);
  cmd.push_back(0x4D);
  cmd.push_back(0x01);

  bool resp_data = false;
  QByteArray response;

  this->sendCommand(cmd, false, 200, resp_data, response, 290);

  // Печатаем текст
  this->printCheck(aCheck);

  feed(3);

  // Обрезка
  this->cut();
}

bool TG2480_PRINTER::feed(int aCount) {
  QByteArray cmd;
  bool respData = false;
  QByteArray answer;
  cmd.push_back(0x0A);
  cmd.push_back(0x0D);

  for (int i = 0; i < aCount; ++i) {
    if (!this->sendCommand(cmd, true, 50, respData, answer, 0)) {
      return false;
    }
  }

  return true;
}
