#pragma once

// System
#include "../AbstractPrinter.h"

namespace CMDAV268 {
    const int charTimeOut = 15;        /// Time out
    const int TimeOutAfterWriting = 0; /// Таймаут порта после запроса.
    const int MaxBufferSize = 1024;    /// Размер буфера.

    const uchar PrinterStatusCommandFirstByte = 0x1B;  /// Статус принтера первый байт
    const uchar PrinterStatusCommandSecondByte = 0x76; /// Статус принтера второй байт
    const uchar PrinterInitCommandSecondByte = 0x40;   /// Инициализация принтера второй байт
    const uchar PrinterCutCommandSecondByte = 0x69;    /// Отрез бумаги второй байт
    const uchar PrinterFeedCommandByte = 0x0A;         /// Прокрутка бумаги
    const uchar PrinterSetFontSecondByte = 0x21;       /// Второй байт команды комплексной установки шрифта

    const uchar PrinterHighFontSecondByte = 0x10;      /// Второй байт установки высокого шрифта
    const uchar PrinterBaldFontSecondByte = 0x45;      /// Второй байт установки жирного шрифта
    const uchar PrinterUnderlineFontSecondByte = 0x2D; /// Второй байт установки подчеркнутого шрифта
    const uchar PrinterDWFontSecondByte = 0x21;        /// Второй байт установки шрифта с двойной шириной
    const uchar PrinterDWFontThirdByte = 0x20;         /// Третий байт установки шрифта с двойной шириной

    const uchar PrinterNormalState = 0x00; /// Нормальный статус принтера
    /// Биты статуса принтера
    const uchar PrinterTemperatureError = 0x02; /// Температурный бит. 0- норма
    const uchar PrinterNoPaperError = 0x04;     /// Наличие бумаги бит. 0- норма
    const uchar PrinterHeadOpenError = 0x08;    /// Дверца термоголовки(открыта). 0- норма
    const uchar PrinterSystemError = 0x20;      /// Системная ошибка. 0- норма
    const uchar PrinterDataReceiveError = 0x40; /// Ошибка приема данных. 0- норма

    /// Количество Baud_Rate, которое поддерживает принтер
    const int Baud_Rate_Count = 1;
    /// Количество Parity, которое поддерживает принтер
    const int Parity_Count = 1;
    const auto DeviceName = "AV268";
}; // namespace CMDAV268

class BasePrinterDevices;

class AV268_PRINTER : public BasePrinterDevices {
    Q_OBJECT

  public:
    AV268_PRINTER(QObject *parent = 0);

    bool OpenPrinterPort();
    bool isItYou();
    bool isEnabled(int status);
    void print(const QString &aCheck);

  protected:
    bool openPort();
    bool getStatus(int &aStatus);
    bool initialize();
    bool cut();
    bool printCheck(const QString &aCheck);
    void getSpecialCharecters(QByteArray &printText);
};
