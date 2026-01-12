#ifndef CITIZENPPU700_H
#define CITIZENPPU700_H

// System
#include "../AbstractPrinter.h"

class BasePrinterDevices;

/// Константы принтера, используются по дефолту в случае пустого конфига.

namespace CMDCitizenPPU700 {
namespace Constants {
namespace Status {
const char Printer = '\x01';
const char Offline = '\x02';
const char Errors = '\x03';
const char Paper = '\x04';
const char ErrorDetails1 = '\x05';
const char ErrorDetails2 = '\x06';
} // namespace Status
} // namespace Constants

/// Позиции в пакетах
namespace Positions {
/// Байты в пакете команды
namespace Command {
const int Prefix = 0;
const int Command = 1;
const int Data = 2;
} // namespace Command

/// Биты в пакете ответа
namespace Answer {
const int CoverOpen = 2;
const int Offline = 3;
const int PaperOut = 5;
const int Error = 6;

namespace Errors {
const int DetectionPresenter = 2;
const int Cutter = 3;
const int Unrecoverable = 5;
const int AutoRecovery = 6;
} // namespace Errors

namespace Paper {
const int NearEndSensor1 = 2;
const int NearEndSensor2 = 3;
const int End = 5;
const int InPresenter = 6;
} // namespace Paper

namespace ErrorDetails1 {
const int CoverOpen = 2;
const int HeadOverheat = 3;
const int LowVoltage = 5;
const int HighVoltage = 6;
} // namespace ErrorDetails1

namespace ErrorDetails2 {
const int Memory = 2;
const int CRC = 3;
const int Presentor = 5;
const int CPU = 6;
} // namespace ErrorDetails2
} // namespace Answer
} // namespace Positions

namespace Commands {
const char Status = '\x04';
}

// статусы неисправностей
struct SFailures {
  bool DetectionPresenter;
  bool Cutter;
  bool Unrecoverable;
  bool AutoRecovery;

  bool CoverOpen;
  bool HeadOverheat;
  bool LowVoltage;
  bool HighVoltage;

  bool Memory;
  bool CRC;
  bool Presentor;
  bool CPU;

  SFailures()
      : DetectionPresenter(false), Cutter(false), Unrecoverable(false),
        AutoRecovery(false),

        CoverOpen(false), HeadOverheat(false), LowVoltage(false),
        HighVoltage(false),

        Memory(false), CRC(false), Presentor(false), CPU(false) {}
};

// статус бумаги
struct SPaper {
  bool NearEndSensor1;
  bool NearEndSensor2;
  bool InPresenter;
  bool End;

  SPaper()
      : NearEndSensor1(false), NearEndSensor2(false), InPresenter(false),
        End(false) {}
};

// структуры для парсинга статусов
struct SStatus {
  bool NotAvailabled;

  // статус принтера
  bool Offline;

  // статус бумаги
  SPaper Paper;

  // оффлайн-статус
  bool CoverOpen;
  bool PaperOut;
  bool Error;

  // неисправности
  SFailures Failures;

  SStatus()
      : NotAvailabled(false), Offline(false), CoverOpen(false), PaperOut(false),
        Error(false) {}
};

const int charTimeOut = 40;

const int TimeOutAfterWriting = 1; /// Таймаут порта после запроса.
const int MaxBufferSize = 1024;    /// Размер буфера.

const uchar PrinterCommandFirstByte = 0x1B; /// Первая часть команды.

const uchar PrinterCommandPaperStatusSecondByte =
    0x76; /// Вторая часть команды paper sensor Status.

const uchar PrinterCommandStatusFirstByte =
    0x10; /// Первая часть команды Printer Status.
const uchar PrinterCommandStatusSecondByte =
    0x04; /// Вторая часть команды Printer Status.
const uchar PrinterCommandStatusThirdByte =
    5; /// Третья часть команды Printer Status.
const uchar PrinterCommandStatusThirdByte1 =
    3; /// Третья часть команды Printer Status(paper jam).

const uchar PrinterCommandFeedByte = 0x0A; /// Команда Feed.

const uchar PrinterCommandCutSecondByte = 0x69; /// Вторая часть команды Cut.

const uchar PrinterCommandInitSecondByte =
    0x40; /// Вторая часть команды Initialization.

const uchar PrinterCommandSetCodePageSecondByte =
    0x74; /// Вторая часть команды выставления русской CodePage.
const uchar PrinterCommandSetCodePageThirdByte =
    0x07; /// Третья часть команды выставления русской CodePage.

const uchar PrinterCommandGetIDFirstByte =
    0x1D; /// Первая часть команды получения ID принтера.
const uchar PrinterCommandGetIDSecondByte =
    0x49; /// Вторая часть команды получения ID принтера.
const uchar PrinterCommandModelParam =
    67; /// Параметр для команды получения ID принтера.
const QString PrinterPPU700 =
    "_PPU-700"; /// Идентификатор для принтера PPU 700.

const uchar PrinterCommandImageSecondByte =
    0x2F; /// Второй байт команды печать картинки
const uchar PrinterCommandRegImageSecondByte =
    0x2A; /// Второй байт команды регистрации картинки

/// Ошибки принтера
const uchar PrinterIsNotAvailable = 0xFF; /// Принтер недоступен
const uchar PaperEnd = 0x04;              /// Бумага закончилась
const uchar PaperNearEnd = 0x01;          /// Бумага почти закончилась
const uchar PrintingHeadTemperatureError =
    0x08; /// Температурная ошибка печатающей головки принтера
const uchar PowerSupplyError = 0x20; /// Ошибка питания принтера
const uchar PaperJamError = 0x08;    /// Бумага зажевалась
const uchar PrinterIsOK = 0x00;      /// Нет ошибок

// Параметры шрифта
const uchar SetFontSecondByte = 0x4D;     /// параметры фонта
const uchar SetFontSecondByteLine = 0x33; /// параметры межстрочного интервала
const uchar SetFont_1 = 0x01;             /// Selection of font A(12?24)
const uchar SetFont_2 = 0x02;             /// Selection of font B(9?24)
const uchar SetFont_3 = 0x03;             /// Selection of font C(8?16)

/// Константы для установки шрифта принтера
const uchar PrinterFontCommandSecondByte =
    0x21; /// Второй байт команды установки шрифта принтера

const uchar PrinterFontBold = 0x45;         /// Жирный
const uchar PrinterFontDoubleHeight = 0x10; /// Двойная высота
const uchar PrinterFontDoubleWidth = 0x20;  /// Двойная ширина
const uchar PrinterFontUnderline = 0x2D;    /// Подчеркнутый

const uchar PrinterCommandSetBarCodeHeightSecondByte =
    0x68;                                                /// Высота штрих-кода
const uchar PrinterCommandPrintBarCodeSecondByte = 0x6B; /// Печать штрих-кода
const uchar PrinterCommandSetBarCodeSystemThirdByte =
    0x49; /// Система штрих-кода CODE128
const uchar PrinterCommandSetBarCodeHRIPositionSecondByte = 0x48;
const uchar PrinterCommandSetBarCodeFontSizeSecondByte = 0x66;
const uchar PrinterCommandSetBarCodeWidthSecondByte = 0x77;

/// Константы для установки параметров штрих-кода
// Ширина линии
// 0x02 - 0.25 mm
// 0x03 - 0.375 mm
// 0x04 - 0.5 mm
// 0x05 - 0.625 mm
// 0x06 - 0.75 mm
const uchar PrinterBarCodeWidth = 0x02;

// Высота штрих-кода
// 0x00..FF
// 0x0A - 20.25 mm
const uchar PrinterBarCodeHeight = 0xA0;

// Позиционирование символов штрих-кода
// 0x01 - выше
// 0x02 - ниже
// 0x03 - выше и ниже
const uchar PrinterBarCodeHRIPosition = 0x02;

// Размер шрифта штрих-кода
const uchar PrinterBarCodeFontSize = 0x01;

const int Baud_Rate_Count = 4;
const int Parity_Count = 2;
const QString DeviceName = "CitizenPPU700";
const int LogoRegistryTimeOut = 3000;

// на запрос статуса возвращается 1 байт
const int StatusAnswerLength = 1;

// набор эвристик для идентификации
namespace Control {
const char StatusMask = '\x12';
}

// Режимы работы
namespace Modes {
enum Enum { Standart = 0x53, Page = 0x4C };
}

namespace Response_d {
const char StatusMask = '\x75';
const QByteArray Resp_Mod_Name = "PPU-700";
} // namespace Response_d
} // namespace CMDCitizenPPU700

class CitizenPPU700_PRINTER : public BasePrinterDevices {

public:
  CitizenPPU700_PRINTER(QObject *parent = 0);
  QString printer_name;

  bool OpenPrinterPort();
  bool isEnabled(CMDCitizenPPU700::SStatus &s_status, int &status);
  //        bool getControlInfo(char aInfoType, CMDCitizenPPU700::SControlInfo &
  //        aControlInfo);
  bool isItYou();
  bool print(const QString &aCheck);
  bool registerLogo(const QString &aPixelString, uchar aWidth);
  bool printImageI(const QString &aPixelString, uchar aWidth,
                   bool aNeedRegisterLogo);
  QString getImage(QString fileName);

protected:
  bool openPort();
  bool getStatus(int &aStatus, CMDCitizenPPU700::SStatus &s_status);
  bool printCheck(const QString &aCheck);

  bool initialize();
  bool cut();
  bool feed(int aCount);
  void dispense();
  bool printImage();
  void getSpecialCharecters(QByteArray &printText);

private:
  bool getState(char aStatusType, CMDCitizenPPU700::SStatus &aStatus);
};

#endif // CITIZENPPU700_H
