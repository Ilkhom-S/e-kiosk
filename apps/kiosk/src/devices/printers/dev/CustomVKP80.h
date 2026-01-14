#pragma once

// System
#include "../AbstractPrinter.h"

class BasePrinterDevices;

namespace CMDCustomVKP80 {

    namespace Constants {
        namespace Status {
            const char Printer = 1;
            const char Offline = 2;
            const char Errors = 3;
            const char Paper = 4;
            const char Printing = 17;
        } // namespace Status

        namespace ControlInfo {
            const char ModelID = 49;
            const char Features = 50;
            const char ROM = 51;
        } // namespace ControlInfo

        /// Идентификаторы моделей принтеров
        namespace ModelIDs {
            const char PrinterID200Dpi = '\x5D'; /// VKP80 200 dpi.
            const char PrinterID300Dpi = '\x5E'; /// VKP80 300 dpi.
        } // namespace ModelIDs
    } // namespace Constants

    namespace AnswersLength {
        // на запрос статуса возвращается 1 байт
        const int Status = 1;

        // на запрос статуса возвращается 1 байт
        namespace ControlInfo {
            const int ModelID = 1;
            const int Features = 1;
            const int ROM = 4;
        } // namespace ControlInfo
    } // namespace AnswersLength

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

            namespace Failures {
                const int Cutter = 3;
                const int Unrecoverable = 5;
                const int AutoRecovery = 6;
            } // namespace Failures

            namespace Paper {
                const int NearEnd = 2;
                const int End = 5;
            } // namespace Paper

            namespace Printing {
                const int MotorOff = 2;
                const int PaperOff = 5;
            } // namespace Printing

            namespace GetFeaturesInfo {
                const int UnicodeSupported = 0;
                const int AutoCutterSupplied = 1;
            } // namespace GetFeaturesInfo
        } // namespace Answer
    } // namespace Positions

    namespace Commands {
        const char Status = '\x04';
        const char GetControlInfo = '\x49';
    } // namespace Commands

    //  статусы неисправностей
    struct SFailures {
        bool Cutter;
        bool Unrecoverable;
        bool AutoRecovery;

        SFailures() : Cutter(false), Unrecoverable(false), AutoRecovery(false) {
        }
    };

    struct SPrinting {
        bool MotorOff;
        bool PaperOff;

        SPrinting() : MotorOff(false), PaperOff(false) {
        }
    };

    // статус бумаги
    struct SPaper {
        bool NearEnd;
        bool End;

        SPaper() : NearEnd(false), End(false) {
        }
    };

    // структура для парсинга статусов
    struct SStatus {
        bool NotAvailabled;

        // статус принтера
        bool Offline;

        // статус бумаги
        SPaper Paper;

        // статус печати
        SPrinting Printing;

        // неисправности
        SFailures Failures;

        // оффлайн-статус
        bool CoverOpen;
        bool PaperOut;
        bool Error;

        SStatus() : NotAvailabled(false), Offline(false), CoverOpen(false), PaperOut(false), Error(false) {
        }
    };

    // структура для парсинга информации
    struct SControlInfo {
        bool noAnswer;
        bool wrongAnswer;
        int Resolution;
        bool isUnicodeSupported;
        bool isAutoCutterSupplied;
        QString ROMVersion;

        SControlInfo()
            : noAnswer(false), wrongAnswer(false), Resolution(0), isUnicodeSupported(false),
              isAutoCutterSupplied(false), ROMVersion("") {
        }
    };

    const int TimeOutAfterWriting = 1;          /// Таймаут порта после запроса.
    const int MaxBufferSize = 1024;             /// Размер буфера.
    const int charTimeOut = 50;                 /// Time out
    const uchar PrinterCommandFirstByte = 0x1B; /// Первая часть команды.

    const uchar PrinterCommandStatusFirstByte = 0x1B;  /// Первая часть команды Printer Status.
    const uchar PrinterCommandStatusSecondByte = 0x76; /// Вторая часть команды Printer Status.

    const uchar FullStatusFirstByte = 0x10;  /// Первая часть команды Printer Status.
    const uchar FullStatusSecondByte = 0x04; /// Вторая часть команды Printer Status.
    const uchar FullStatusThirdByte = 20;    /// Третья часть команды Printer Status.

    const uchar PrinterCommandFeedFirstByte = 0x0D;         /// Возврат каретки.
    const uchar PrinterCommandFeedSecondByte = 0x0A;        /// Команда Feed.
    const uchar PrinterCommandAnotherFeedSecondByte = 0x4A; /// Вторая часть команды Feed.

    const uchar PrinterCommandCutSecondByte = 0x69; /// Вторая часть команды Cut.

    const uchar PrinterCommandInitSecondByte = 0x40; /// Вторая часть команды Initialization.

    const uchar PrinterCommandSetCodePageSecondByte = 0x74; /// Вторая часть команды выставления русской CodePage.
    const uchar PrinterCommandSetCodePageThirdByte = 0x07;  /// Третья часть команды выставления русской CodePage.

    const uchar PrinterCommandCharacterSetSecondByte = 0x21; /// Вторая часть команды international character set.
    const uchar PrinterCommandCharacterSetThirdByte = 0x01;  /// Третья часть команды international character set.

    const uchar PrinterCommandGetIDFirstByte = 0x1D;  /// Первая часть команды получения ID принтера.
    const uchar PrinterCommandGetIDSecondByte = 0x49; /// Вторая часть команды получения ID принтера.
    const uchar PrinterCommandModelParam = 49;        /// Третья часть команды получения ID принтера.

    const uchar PrinterCommandClrDispenserSecondByte = 0x65; /// Второй байт команды clear dispenser
    const uchar PrinterCommandClrDispenserThirdByte = 0x05;  /// Третий байт команды clear dispenser

    const uchar PrinterCommandDispenseThirdByte = 0x03; /// Третий байт команды dispense
    const uchar PrinterCommandDispenseForthByte = 0x1E; /// Четвертый байт команды dispense

    const uchar PrinterCommandPaperSizeSecondByte = 0x33; /// Второй байт параметра размера чека

    const uchar PrinterCommandPaperSizeThirdByteSmall = 0x28; /// Третий байт параметра размера укароченного чека
    const uchar PrinterCommandPaperSizeThirdByteNorm = 0x32;  /// Третий байт параметра размера нормального чека

    const uchar PrinterCommandLogoRegSecondByte = 0x2A; /// Второй байт команды регистрации логотипа

    const uchar PrinterCommandLogoPrintSecondByte = 0xFA; /// Второй байт команды печати логотипа
    const uchar PrinterCommandLogoPrintThirdByte = 0x64;  /// Третий байт команды печати логотипа
    const uchar PrinterCommandLogoPrintFothByte = 0x6E;   /// Третий байт команды печати логотипа

    const uchar SetStandartMode = 0x53; /// Установка стандартного режима печати
    const uchar SetPageMode = 0x4C;     /// Установка страничного  режима печати

    /// Ошибки принтера
    const uchar PrinterIsNotAvailable = 0xFF; /// Принтер недоступен
    const uchar PaperEnd = 0x0C;              /// Бумага закончилась
    const uchar PaperNearEnd = 0x03;          /// Бумага почти закончилась
    const uchar UnknownCommand = 0x20;        /// Неизвестная команда
    const uchar PaperJamError = 0x40;         /// Бумага зажевалась
    const uchar PrinterIsOK = 0x00;           /// Нет ошибок

    /// Константы для установки шрифта принтера
    const uchar PrinterFontCommandSecondByte = 0x21; /// Второй байт команды установки шрифта принтера

    const uchar PrinterFontBoldSecondByte = 0x45;    /// Жирный
    const uchar PrinterFontDoubleWidthHeight = 0x21; /// Двойная высота
    const uchar PrinterFontItalic = 0x34;            /// Курсив
    const uchar PrinterFontUnderline = 0x2D;         /// Подчеркнутый

    const uchar PrinterCommandSetBarCodeHeightSecondByte = 0x68; /// Высота штрих-кода
    const uchar PrinterCommandPrintBarCodeSecondByte = 0x6B;     /// Печать штрих-кода
    const uchar PrinterCommandSetBarCodeSystemThirdByte = 0x49;  /// Система штрих-кода CODE128
    const uchar PrinterCommandSetBarCodeHRIPositionSecondByte = 0x48;
    const uchar PrinterCommandSetBarCodeFontSizeSecondByte = 0x66;
    const uchar PrinterCommandSetBarCodeWidthSecondByte = 0x77;

    const QString DeviceName = "CustomVKP80";

    namespace Control {
        const char StatusMask1 = '\x12';
        const char StatusMask0 = '\x7E';
    } // namespace Control
} // namespace CMDCustomVKP80

class CustomVKP80_PRINTER : public BasePrinterDevices {

  public:
    CustomVKP80_PRINTER(QObject *parent = 0);
    QString printer_name;

    bool OpenPrinterPort();
    bool isEnabled(CMDCustomVKP80::SStatus &s_status, int &state);
    bool getControlInfo(char aInfoType, CMDCustomVKP80::SControlInfo &aControlInfo);
    bool isItYou();
    void print(const QString &aCheck);
    bool registerLogo(const QString &aPixelString, uchar aWidth);
    bool printImageI(const QString &aPixelString, uchar aWidth, bool aNeedRegisterLogo);
    //        QString getImage(QString fileName);
    QByteArray getImage(QString fileName);
    QString comment;

  protected:
    bool openPort();
    bool getStatus(int &aStatus, CMDCustomVKP80::SStatus &s_status);
    bool printCheck(const QString &aCheck);
    void sendFiscalData();
    bool initialize();
    bool cut();
    bool feed(int aCount);
    void dispense();
    bool printImage();
    void getSpecialCharecters(QByteArray &printText);

  private:
    bool getState(char aStatusType, CMDCustomVKP80::SStatus &aStatus);
};

