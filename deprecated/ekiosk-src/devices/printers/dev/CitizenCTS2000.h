#pragma once

#include "../AbstractPrinter.h"

class BasePrinterDevices;

namespace CMDCitizenCTS2000 {
const char charTimeOut = 40;
const int TimeOutAfterWriting = 100; /// Таймаут порта после запроса.
const int MaxBufferSize = 1024;      /// Размер буфера.
const QString DeviceName = "CitizenCTS2000";
/// Байты команд
const char PrinterCommandCutFirstByte = '\x1D'; /// Первая часть команды Cut.
const char PrinterCommandFirstByte = '\x1B';    /// Первая часть команды Cut.

const char PrinterCommandStatusFirstByte = '\x10';  /// Первая часть команды Status.
const char PrinterCommandStatusSecondByte = '\x04'; /// Вторая часть команды Status.

const char SetFontSecondByte = '\x4D'; /// параметры фонта
const char SetFont_1 = '\x01';         /// Selection of font A(12?24)
const char SetFont_2 = '\x02';         /// Selection of font B(9?24)
const char SetFont_3 = '\x03';         /// Selection of font C(8?16)
const char SetFontSecondByteLine = '\x33';
const char PrinterCommandFeedByte = '\x0A';        /// Команда Feed.
const char PrinterCommandCutSecondByte = '\x56';   /// Вторая часть команды Cut.
const char PrinterCommandGetIDSecondByte = '\x49'; /// Вторая часть команды identification.
const char PrinterCommandModelParam =
    '\x01'; /// Параметр для команды, чтобы получить ID устройства.
const char PrinterCTS2000 =
    '\x51'; /// Если такой ответ принтера на получение ID, то модель CTS2000.
const char PrinterCommandInitializeFirstByte = '\x1B';  /// Первая часть команды инициализации.
const char PrinterCommandInitializeSecondByte = '\x40'; /// Вторая часть команды инициализации.

const char PrinterCommandCodeTableFirstByte = '\x1B';  /// Первая часть команды установки кодировки.
const char PrinterCommandCodeTableSecondByte = '\x74'; /// Вторая часть команды установки кодировки.
const char PrinterCommandCodeTableRussian = 7;         /// Номер русской Codepage.

const char PrinterCommandImageFirstByte =
    '\x1D'; /// Первый байт команд печати и регистрации логотипа
const char PrinterCommandImageRegSecondByte = '\x2A';   /// Второй байт команды регистрации картинки
const char PrinterCommandImagePrintSecondByte = '\x2F'; /// Второй байт команды печати картинки

/// Ошибки принтера
const char PrinterIsNotAvailable = '\xFF'; /// Принтер недоступен
const char PrinterError = '\x40';          /// Ошибка принтера
const char PaperJam_Error = '\x08';         /// Бумага зажевалась
const char UnrecoverableError = '\x20';    /// Невосстановимая ошибка принтера
const char RecoverableError = '\x40';      /// Восстановимая ошибка принтера
const char PaperEnd = '\x7E';              /// Бумага закончилась
const char PaperNearEnd = '\x1E';          /// Бумага почти закончилась
const char CoverOpen = '\x04';             /// Открыта крышка принтера

const char PrinterIsOK = '\x12'; /// Нет ошибок

/// Константы для установки шрифта принтера
const char PrinterFontCommandSecondByte = '\x21'; /// Второй байт команды установки шрифта принтера

const char PrinterFontBold = '\x45';         /// Жирный
const char PrinterFontDoubleHeight = '\x10'; /// Двойная высота
const char PrinterFontDoubleWidth = '\x20';  /// Двойная ширина
const char PrinterFontUnderline = '\x2D';    /// Подчеркнутый

/// Количество Baud_rate, которое поддерживает принтер
const int Baud_Rate_Count = 4;

/// Количество Parity, которое поддерживает принтер
const int Parity_Count = 2;

const int LogoRegistryTimeOut = 3000;

// на запрос статуса возвращается 1 байт; делаем 3 разных запроса
const int StatusAnswerLength = 3;

// набор эвристик для идентификации
namespace Control {
const char StatusMask = '\x12';
}

// Режимы работы
namespace Modes {
enum Enum { Standart = '\x53', Page = '\x4C' };
}
}; // namespace CMDCitizenCTS2000

class CitizenCTS2000_PRINTER : public BasePrinterDevices {

public:
    CitizenCTS2000_PRINTER(QObject *parent = 0);
    QString printer_name;

    bool OpenPrinterPort();
    bool isEnabled(int &status);
    bool isItYou();
    bool print(const QString &aCheck);
    bool registerLogo(const QString &aPixelString, uchar aWidth);
    bool printImageI(const QString &aPixelString, uchar aWidth, bool aNeedRegisterLogo);
    QString getImage(QString fileName);

protected:
    bool openPort();
    bool getStatus(int &aStatus);
    bool printCheck(const QString &aCheck);

    bool initialize();
    bool cut();
    bool feed(int aCount);
    void dispense();
    bool printImage();
    void getSpecialCharacters(QByteArray &printText);

private:
    QByteArray getState();
};
