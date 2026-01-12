#include "CitizenPPU700.h"

#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QIODevice>

CitizenPPU700_PRINTER::CitizenPPU700_PRINTER(QObject *parent) : BasePrinterDevices(parent)
{
	//    printer_name = "Custom-VKP80";
}

bool CitizenPPU700_PRINTER::OpenPrinterPort()
{
	return openPort();
}

bool CitizenPPU700_PRINTER::openPort()
{
	if (devicesCreated)
	{
		is_open = false;

		// Даем девайсу название порта
		serialPort->setPortName(comName);

		if (serialPort->open(QIODevice::ReadWrite))
		{
			// Если Девайсу удалось открыть порт

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
		}
		else
		{
			is_open = false;
		}
	}
	else
	{
		is_open = false;
	}

	return is_open;
}

bool CitizenPPU700_PRINTER::isEnabled(CMDCitizenPPU700::SStatus &s_status, int &status)
{
	//    int status = 0;
	if (!this->getStatus(status, s_status))
	{
		return false;
	}

	return (status != PrinterState::PrinterNotAvailable);
}

bool CitizenPPU700_PRINTER::getStatus(int &aStatus, CMDCitizenPPU700::SStatus &s_status)
{
	// смотрим оффлайн
	if (!getState(CMDCitizenPPU700::Constants::Status::Printer, s_status))
	{
		// if(Debuger) qDebug() <<  "Unable to get status, type Printer!";
		return false;
	}

	// если в оффлайне - смотрим причину
	if (s_status.Offline)
	{
		if (!getState(CMDCitizenPPU700::Constants::Status::Offline, s_status))
		{
			// if(Debuger) qDebug() << "Unable to get status, type Offline!";
			return false;
		}
	}

	// статус ошибки смотрим всегда
	if (!getState(CMDCitizenPPU700::Constants::Status::Errors, s_status))
	{
		// if(Debuger) qDebug() << "Unable to get status, type Errors!";
		return false;
	}

	if (!getState(CMDCitizenPPU700::Constants::Status::ErrorDetails1, s_status))
	{
		// if(Debuger) qDebug() << "Unable to get status, type ErrorDetails1!";
		return false;
	}

	if (!getState(CMDCitizenPPU700::Constants::Status::ErrorDetails2, s_status))
	{
		// if(Debuger) qDebug() << "Unable to get status, type ErrorDetails2!";
		return false;
	}

	// статус бумаги смотрим только если не в оффлайне из-за конца бумаги
	if (!s_status.PaperOut)
	{
		if (!getState(CMDCitizenPPU700::Constants::Status::Paper, s_status))
		{
			// if(Debuger) qDebug() << "Unable to get status, type paper!";
			return false;
		}
	}

	// разбираем статусы
	aStatus = PrinterState::PrinterOK;

	// ошибки
	if (s_status.NotAvailabled)
	{
		aStatus |= PrinterState::PrinterNotAvailable;
	}

	if (s_status.Failures.HighVoltage || s_status.Failures.HighVoltage)
	{
		aStatus |= PrinterState::PowerSupplyError;
	}
	else if (s_status.Failures.Presentor)
	{
		aStatus |= PrinterState::MechanismPositionError;
	}
	else if (s_status.Failures.Memory || s_status.Failures.CPU)
	{
		aStatus |= PrinterState::ElectronicError;
	}
	else if (s_status.Failures.CoverOpen || s_status.CoverOpen)
	{
		aStatus |= PrinterState::CoverIsOpened;
	}
	else if (s_status.Failures.Cutter)
	{
		aStatus |= PrinterState::CutterError;
	}
	else if (s_status.Failures.Unrecoverable || s_status.Failures.CRC || s_status.Failures.DetectionPresenter ||
			 s_status.Error)
	{
		aStatus |= PrinterState::PrinterError;
	}
	else if (s_status.PaperOut || s_status.Paper.End)
	{
		aStatus |= PrinterState::PaperEnd;
	}
	else if (s_status.Paper.NearEndSensor1 || s_status.Paper.NearEndSensor2)
	{
		// Если в админки включен индикатор толщины рулона
		if (this->counterIndicate)
		{
			// Paper near end
			aStatus |= PrinterState::PaperNearEnd;
		}
	}

	return true;
}

bool CitizenPPU700_PRINTER::getState(char aStatusType, CMDCitizenPPU700::SStatus &aStatus)
{
	if (aStatus.NotAvailabled)
	{
		return true;
	}

	QByteArray answerPacket;
	QByteArray commandPacket = QByteArray(1, ASCII::DLE);
	commandPacket.push_back(CMDCitizenPPU700::Commands::Status);
	commandPacket.push_back(aStatusType);
	bool respData = false;
	bool result = true;

	if (this->sendCommand(commandPacket, true, 200, respData, answerPacket, 0))
	{
		if (answerPacket.size() != CMDCitizenPPU700::StatusAnswerLength)
		{
			aStatus.NotAvailabled = true;
			return true;
		}

		aStatus.NotAvailabled = false;
		char answer = answerPacket.right(1)[0];

		// если ответило другое устройство
		if (!positiveMasking(answer, CMDCitizenPPU700::Control::StatusMask))
		{
			aStatus.NotAvailabled = true;
			return true;
		}

		switch (aStatusType)
		{
		case CMDCitizenPPU700::Constants::Status::Printer:
		{
			aStatus.Offline = getBit(answer, CMDCitizenPPU700::Positions::Answer::Offline);
			break;
		}
		case CMDCitizenPPU700::Constants::Status::Offline:
		{
			aStatus.CoverOpen = getBit(answer, CMDCitizenPPU700::Positions::Answer::CoverOpen);
			aStatus.PaperOut = getBit(answer, CMDCitizenPPU700::Positions::Answer::PaperOut);
			aStatus.Error = getBit(answer, CMDCitizenPPU700::Positions::Answer::Error);
			break;
		}
		case CMDCitizenPPU700::Constants::Status::Errors:
		{
			aStatus.Failures.DetectionPresenter =
				getBit(answer, CMDCitizenPPU700::Positions::Answer::Errors::DetectionPresenter);
			aStatus.Failures.Cutter = getBit(answer, CMDCitizenPPU700::Positions::Answer::Errors::Cutter);
			aStatus.Failures.Unrecoverable = getBit(answer, CMDCitizenPPU700::Positions::Answer::Errors::Unrecoverable);
			break;
		}
		case CMDCitizenPPU700::Constants::Status::Paper:
		{
			aStatus.Paper.End = getBit(answer, CMDCitizenPPU700::Positions::Answer::Paper::End);
			aStatus.Paper.InPresenter = getBit(answer, CMDCitizenPPU700::Positions::Answer::Paper::InPresenter);
			aStatus.Paper.NearEndSensor1 = getBit(answer, CMDCitizenPPU700::Positions::Answer::Paper::NearEndSensor1);
			aStatus.Paper.NearEndSensor2 = getBit(answer, CMDCitizenPPU700::Positions::Answer::Paper::NearEndSensor2);
			break;
		}
		case CMDCitizenPPU700::Constants::Status::ErrorDetails1:
		{
			aStatus.Failures.CoverOpen = getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails1::CoverOpen);
			aStatus.Failures.HeadOverheat =
				getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails1::HeadOverheat);
			aStatus.Failures.LowVoltage =
				getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails1::LowVoltage);
			aStatus.Failures.HighVoltage =
				getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails1::HighVoltage);
			break;
		}
		case CMDCitizenPPU700::Constants::Status::ErrorDetails2:
		{
			aStatus.Failures.Memory = getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails2::Memory);
			aStatus.Failures.CRC = getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails2::CRC);
			aStatus.Failures.Presentor = getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails2::Presentor);
			aStatus.Failures.CPU = getBit(answer, CMDCitizenPPU700::Positions::Answer::ErrorDetails2::CPU);
			break;
		}
		default:
		{
			result = false;
		}
		}
	}
	else
	{
		result = false;
	}

	return result;
}

bool CitizenPPU700_PRINTER::isItYou()
{

	QByteArray cmd;
	//    QByteArray data;
	//    QByteArray packet;
	QByteArray answer;
	bool resp_data = false;
	// Citizen

	// Сначала проверим, что это наша модель принтера
	cmd.push_back(CMDCitizenPPU700::PrinterCommandGetIDFirstByte);
	cmd.push_back(CMDCitizenPPU700::PrinterCommandGetIDSecondByte);
	cmd.push_back(CMDCitizenPPU700::PrinterCommandModelParam); // Узнаем модель принтера

	if (!this->sendCommand(cmd, true, 100, resp_data, answer, 0))
	{
		// if(Debuger) qDebug() << "CitizenPPU700::isItYou(): error in sendPacketInPort()";
		return false;
	}

	bool result = false;

	if (answer.lastIndexOf(CMDCitizenPPU700::PrinterPPU700.toLatin1()) >= 0)
	{
		if (answer.contains(CMDCitizenPPU700::Response_d::Resp_Mod_Name))
			// if(Debuger) qDebug() << "CitizenPPU700::isItYou(): response - " <<
			// CMDCitizenPPU700::Response_d::Resp_Mod_Name;
			result = true;
	}
	else if (answer.isEmpty())
	{
		//            if (this->isEnabled())
		//            {
		//                    result = true;
		//            }
	}

	if (!result)
	{
	}
	this->closePort();
	return result;
}

void CitizenPPU700_PRINTER::getSpecialCharecters(QByteArray &printText)
{
	QByteArray fontTypeBold_start;
	fontTypeBold_start.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
	fontTypeBold_start.push_back(CMDCitizenPPU700::PrinterFontBold);
	fontTypeBold_start.push_back(1);
	QByteArray fontTypeBold_end;
	fontTypeBold_end.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
	fontTypeBold_end.push_back(CMDCitizenPPU700::PrinterFontBold);
	fontTypeBold_end.push_back(48);

	QByteArray fontTypeUnderLine_start;
	fontTypeUnderLine_start.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
	fontTypeUnderLine_start.push_back(CMDCitizenPPU700::PrinterFontUnderline);
	fontTypeUnderLine_start.push_back(1);
	QByteArray fontTypeUnderLine_end;
	fontTypeUnderLine_end.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
	fontTypeUnderLine_end.push_back(CMDCitizenPPU700::PrinterFontUnderline);
	fontTypeUnderLine_end.push_back(48);

	QByteArray fontTypeDoubleWidth_start;
	if (smallChek)
	{
		fontTypeDoubleWidth_start.push_back(ASCII::NUL);
	}
	else
	{
		fontTypeDoubleWidth_start.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
		fontTypeDoubleWidth_start.push_back(CMDCitizenPPU700::PrinterFontCommandSecondByte);
		fontTypeDoubleWidth_start.push_back(0x20);
	}

	QByteArray fontTypeDoubleWidth_end;
	if (smallChek)
	{
		fontTypeDoubleWidth_end.push_back(ASCII::NUL);
	}
	else
	{
		fontTypeDoubleWidth_end.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
		fontTypeDoubleWidth_end.push_back(CMDCitizenPPU700::PrinterFontCommandSecondByte);
		fontTypeDoubleWidth_end.push_back(ASCII::NUL);
	}

	QByteArray fontTypeDoubleHeight_start;
	if (smallChek)
	{
		fontTypeDoubleHeight_start.push_back(ASCII::NUL);
	}
	else
	{
		fontTypeDoubleHeight_start.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
		fontTypeDoubleHeight_start.push_back(CMDCitizenPPU700::PrinterFontCommandSecondByte);
		fontTypeDoubleHeight_start.push_back(0x10);
	}

	QByteArray fontTypeDoubleHeight_end;
	if (smallChek)
	{
		fontTypeDoubleHeight_end.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
		fontTypeDoubleHeight_end.push_back(CMDCitizenPPU700::PrinterFontCommandSecondByte);
		fontTypeDoubleHeight_end.push_back(ASCII::NUL);
	}
	else
	{
		fontTypeDoubleHeight_end.push_back(ASCII::NUL);
	}

	// Устанавливаем если есть двойной высоты фонт
	printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeDoubleHeight +
							  CScharsetParam::CloseTagDelimiter)
						  .toUtf8(),
					  fontTypeDoubleHeight_start);
	printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
							  CScharsetParam::FontTypeDoubleHeight + CScharsetParam::CloseTagDelimiter)
						  .toUtf8(),
					  fontTypeDoubleHeight_end);

	// Устанавливаем если есть двойной ширины фонт
	printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeDoubleWidth +
							  CScharsetParam::CloseTagDelimiter)
						  .toUtf8(),
					  fontTypeDoubleWidth_start);
	printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
							  CScharsetParam::FontTypeDoubleWidth + CScharsetParam::CloseTagDelimiter)
						  .toUtf8(),
					  fontTypeDoubleWidth_end);

	// Устанавливаем если есть курсивный фонт
	printText.replace(
		QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter)
			.toUtf8(),
		asciiNull());
	printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
							  CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter)
						  .toUtf8(),
					  asciiNull());

	// Устанавливаем если есть жирный фонт
	printText.replace(
		QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter)
			.toUtf8(),
		fontTypeBold_start);

	printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
							  CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter)
						  .toUtf8(),
					  fontTypeBold_end);

	// Устанавливаем если есть подчеркнутый фонт
	printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeUnderLine +
							  CScharsetParam::CloseTagDelimiter)
						  .toUtf8(),
					  fontTypeUnderLine_start);
	printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
							  CScharsetParam::FontTypeUnderLine + CScharsetParam::CloseTagDelimiter)
						  .toUtf8(),
					  fontTypeUnderLine_end);

	// Если надо добавить проабел
	QByteArray probel;
	for (int i = 1; i <= leftMargin; i++)
	{
		probel.append(ASCII::Space);
	}
	printText.replace(
		QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::ProbelCount + CScharsetParam::CloseTagDelimiter)
			.toUtf8(),
		probel);
}

bool CitizenPPU700_PRINTER::printCheck(const QString &aCheck)
{
	// Меняем кодировку
	QByteArray printText;
	printText = this->encodingString(aCheck, CScodec::c_IBM866);

	// Вставляем если есть Подчеркнутый, Жирный, Курсивный... Шрифт
	this->getSpecialCharecters(printText);

	//    QByteArray cmd;
	QByteArray answer;
	bool respData = false;

	//    // Проинициализируем принтер
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandFirstByte);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandInitSecondByte);

	//    if(!this->sendCommand(cmd,false,0,respData,answer,0))
	//            return false;

	//    cmd.clear();
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandFirstByte);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandPaperSizeSecondByte);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandPaperSizeThirdByteSmall);
	//    cmd.push_back(0x0A);

	if (!this->sendCommand(printText, true, 200, respData, answer, 50))
	{
		return false;
	}

	return true;
}

QString CitizenPPU700_PRINTER::getImage(QString fileName)
{

	QImage image(fileName);
	qDebug() << "image.byteCount(); - " << image.sizeInBytes();
	QByteArray ba;

	QBuffer buffer(&ba);
	buffer.open(QIODevice::ReadWrite);
	image.save(&buffer, "BMP"); // writes image into ba in PNG format
	qDebug() << "buffer.bytesAvailable(); - " << buffer.bytesAvailable();
	qDebug() << "ba.size(); - " << ba.size();
	////    QByteArray compressed = qCompress(ba, 1);

	QFile vrm_file("img_cont.txt");
	if (vrm_file.open(QFile::WriteOnly))
	{
		vrm_file.write(ba);
		vrm_file.close();
	}

	QString data = "";
	if (vrm_file.open(QFile::ReadOnly))
	{
		data = vrm_file.readAll();
		vrm_file.close();
	}

	buffer.close();
	return data;
}

bool CitizenPPU700_PRINTER::print(const QString &aCheck)
{
	// Инициализация
	this->initialize();
	// Картинка
	//      if(viewLogoImg) this->printImage();

	//     QString content = getImage("2.bmp");
	//     qDebug() << content;
	//     this->printImageI(content,255,true);
	// Печатаем текст
	this->printCheck(aCheck);

	if (!feed(3))
	{
		return false;
	}

	// Обрезка
	this->cut();

	// Прокрутка
	//      this->dispense();
	return true;
}

void CitizenPPU700_PRINTER::dispense()
{
	QByteArray cmd;
	QByteArray answer;
	bool respData = false;

	// Dispense
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandGetIDFirstByte);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandClrDispenserSecondByte);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandDispenseThirdByte);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandDispenseForthByte);

	if (!this->sendCommand(cmd, false, 0, respData, answer, 50))
		return;
}

bool CitizenPPU700_PRINTER::initialize()
{
	QByteArray cmd;
	QByteArray answer;
	bool respData = false;

	// Проинициализируем принтер

	cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
	cmd.push_back(CMDCitizenPPU700::PrinterCommandInitSecondByte);

	if (!this->sendCommand(cmd, false, 0, respData, answer, 0))
		return false;

	cmd.clear();

	// Установим русскую Code page
	cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
	cmd.push_back(CMDCitizenPPU700::PrinterCommandSetCodePageSecondByte);
	cmd.push_back(CMDCitizenPPU700::PrinterCommandSetCodePageThirdByte);

	bool result = this->sendCommand(cmd, true, 50, respData, answer, 0);

	if (smallChek)
	{
		// Устанавливаем фонт
		cmd.clear();

		cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
		cmd.push_back(CMDCitizenPPU700::SetFontSecondByte);
		cmd.push_back(CMDCitizenPPU700::SetFont_1);

		this->sendCommand(cmd, false, 50, respData, answer, 0);
	}

	if (SmallBeetwenString)
	{
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

bool CitizenPPU700_PRINTER::cut()
{
	QByteArray cmd;
	QByteArray answer;
	bool respData = false;

	//    cmd.push_back(CMDCitizenPPU700::PrinterCommandFirstByte);
	//    cmd.push_back(CMDCitizenPPU700::PrinterCommandCutSecondByte);

	cmd.push_back(0x1D);
	cmd.push_back(0x56);
	cmd.push_back(0x01);

	if (!this->sendCommand(cmd, true, 50, respData, answer, 0))
	{
		return false;
	}

	return true;
}

bool CitizenPPU700_PRINTER::feed(int aCount)
{

	QByteArray cmd;
	bool respData = false;
	QByteArray answer;
	cmd.push_back(CMDCitizenPPU700::PrinterCommandFeedByte);
	cmd.push_back(0x0D);

	for (int i = 0; i < aCount; ++i)
	{
		if (!this->sendCommand(cmd, true, 50, respData, answer, 0))
		{
			return false;
		}
	}

	return true;
}

bool CitizenPPU700_PRINTER::printImage()
{
	QByteArray cmd;
	QByteArray answer;
	bool respData = false;

	//    cmd.push_back(CMDCustomVKP80::PrinterCommandFirstByte);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandLogoPrintSecondByte);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandCharacterSetThirdByte);
	//    cmd.push_back(ASCII::NUL);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandLogoPrintThirdByte);
	//    cmd.push_back(ASCII::NUL);
	//    cmd.push_back(CMDCustomVKP80::PrinterCommandLogoPrintFothByte);

	if (!this->sendCommand(cmd, false, 0, respData, answer, 50))
	{
		return false;
	}

	return true;
}

bool CitizenPPU700_PRINTER::printImageI(const QString &aPixelString, uchar aWidth, bool aNeedRegisterLogo)
{
	QByteArray cmd;
	QByteArray resp_data;
	bool resp_ok = false;

	if (aNeedRegisterLogo)
	{
		if (!registerLogo(aPixelString, aWidth))
		{
			return false;
		}
	}
	//        cmd.push_back(CMDCustomVKP80::PrinterCommandGetIDFirstByte);
	//        cmd.push_back(0x2F);
	//        cmd.push_back(48);
	//        uchar nil = 0;
	//        cmd.append(CMDCustomVKP80::PrinterCommandFirstByte);
	//        cmd.append(CMDCustomVKP80::PrinterCommandAnotherFeedSecondByte);
	//        cmd.append(nil);

	return this->sendCommand(cmd, true, 100, resp_ok, resp_data, 0);
}

bool CitizenPPU700_PRINTER::registerLogo(const QString &aPixelString, uchar aWidth)
{
	Q_UNUSED(aPixelString)

	if (!aWidth)
	{
		return true;
	}

	QByteArray cmd;
	QByteArray response;
	bool resp_data = false;

	//    uchar height = 0;
	//    if (aWidth){
	//        height = (uchar)(aPixelString.size() / aWidth);
	//    }

	//    uchar verticalSize = height / 8; // Размер по вертикали, который мы передадим принтеру
	//    if (height % 8) {
	//        verticalSize++;
	//    }

	//        cmd.push_back(CMDCustomVKP80::PrinterCommandGetIDFirstByte);
	//        cmd.push_back(CMDCustomVKP80::PrinterCommandLogoRegSecondByte);
	//        cmd.push_back(aWidth);
	//        cmd.push_back(verticalSize);
	////        QString str(aPixelString);
	//        QByteArray imageData = packetImage(aPixelString, aWidth);
	////        QByteArray imageData = aPixelString.toHex();
	////        //if(Debuger) qDebug() << "this->printDataToHex(imageData);";
	//        //if(Debuger) qDebug() << "aPixelString.size()" << aPixelString.size();
	////        this->printDataToHex(imageData);
	//        cmd.append(imageData);

	return sendCommand(cmd, true, 3000, resp_data, response, 3000);
}
