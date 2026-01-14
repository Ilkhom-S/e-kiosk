#pragma once

// System
#include "../AbstractPrinter.h"

class BasePrinterDevices;

namespace CMDStarTUP900 {
    const char charTimeOut = 40;
    const int TimeOutAfterWriting = 100; /// Таймаут порта после запроса.
    const int MaxBufferSize = 1024;      /// Размер буфера.

    const uchar PrinterStatusCommand = 0x04;         /// Команда получения статуса
    const uchar PrinterCommandFeedFirstByte = 0x0D;  /// Первая часть команды feed.
    const uchar PrinterCommandFeedSecondByte = 0x0A; /// Вторая часть команды feed.

    const uchar PrinterCommandCutFirstByte = 0x1B;  /// Первая часть команды cut.
    const uchar PrinterCommandCutSecondByte = 0x64; /// Вторая часть команды cut.
    // const uchar PrinterCommandCutData               = 0;    /// ТoDо, что надо
    // передать для команды cut

    const uchar PrinterCommandInitFirstByte = 0x1B;  /// Первая часть команды Init.
    const uchar PrinterCommandInitSecondByte = 0x40; /// Вторая часть команды Init.

    const uchar PresenterAutoPushCmdSecondByte = 0x16; /// Вторая часть команды PresenterAutoPush.
    const uchar PresenterAutoPushCmdThirdByte = 0x31;  /// Третья часть команды PresenterAutoPush.

    const uchar PrinterSetFontCmdSecondByte = 0x69; /// Вторая часть команды SetFont.

    const uchar PrinterCommandSetLeftMarginSecondByte = 0x6C; /// Вторая часть команды установки левой границы.

    const uchar PrinterCommandLogoFirstByte = 0x1B;      /// Первая часть команд, связанных с логотипом
    const uchar PrinterCommandLogoSecondByte = 0x1C;     /// Вторая часть команд, связанных с логотипом
    const uchar PrinterCommandRegLogoThirdByte = 0x71;   /// Третья часть команды регистрации логотипа
    const uchar PrinterCommandPrintLogoThirdByte = 0x70; /// Третья часть команды печати логотипа

    const QString DeviceName = "StarTUP900";

    /// Ошибки принтера
    const uchar PrinterIsNotAvailable = 0xFF; /// Принтер недоступен
    const uchar PaperJamError = 0x40;         /// Бумага зажевалась
    const uchar PaperEnd = 0x08;              /// Бумага закончилась
    const uchar PaperNearEnd = 0x04;          /// Бумага почти закончилась

    const uchar PrinterIsOK = 0x12; /// Нет ошибок

    /// Константы для установки шрифта принтера
    const uchar PrinterCmdSetBoldSecondByte = 0x45;    /// Второй байт команды установки жирного шрифта
    const uchar PrinterCmdCancelBoldSecondByte = 0x46; /// Второй байт команды отмены жирного шрифта

    const uchar PrinterCmdDoubleHeightSecondByte = 0x68; /// Второй байт команды двойной высоты шрифта
    const uchar PrinterCmdDoubleWidthSecondByte = 0x57;  /// Второй байт команды двойной ширины шрифта
    const uchar PrinterCmdUnderlineSecondByte = 0x2D;    /// Второй байт команды подчеркнутого шрифта

    const uchar PrinterCmdSetMode = 1;     /// Установить мод
    const uchar PrinterCmdCancelMode = 48; /// Отменить мод

    /// Количество Baud_Rate, которое поддерживает принтер
    const int Baud_Rate_Count = 4;

    const int LogoRegistryTimeOut = 3000;
}; // namespace CMDStarTUP900

class StarTUP900_PRINTER : public BasePrinterDevices {

  public:
    StarTUP900_PRINTER(QObject *parent = 0);
    QString printer_name;

    bool OpenPrinterPort();
    bool isEnabled();
    bool getControlInfo(char aInfoType);
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
    void getSpecialCharecters(QByteArray &printText);

  private:
    QByteArray getState();
};

