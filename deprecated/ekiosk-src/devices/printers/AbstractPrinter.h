#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtGui/QImage>
#include <QtSerialPort/QSerialPort>

namespace CScodec {
const QByteArray c_IBM866 = "IBM 866";
const QByteArray WindowsCp1251 = "Windows-1251";
} // namespace CScodec

namespace PrinterState {
/// Вообще не статус
const int NoStatus = 0xFFFFFFFF;

//--------------------------------------------

/// Нет ошибок
const int PrinterOK = 0x00;

//--------------------------------------------

/// Бумага закончилась
const int PaperEnd = 0x01;

/// Бумага заканчивается
const int PaperNearEnd = 0x02;

/// Бумага зажевана
const int PaperJam = 0x04;

/// Неизвестная команда
const int UnknownCommand = 0x08;

/// Ошибка питания
const int PowerSupplyError = 0x10;

/// Ошибка печатающей головки
const int PrintingHeadError = 0x20;

/// Ошибка порта
const int PortError = 0x40;

/// Ошибка принтера
const int PrinterError = 0x80;

/// Температурная ошибка
const int TemperatureError = 0x100;

/// Неизвестная ошибка
const int UnknownError = 0x200;

/// Крышка открыта
const int CoverIsOpened = 0x400;

/// Ошибка фискальной памяти
const int FiscalMemoryError = 0x800;

/// Ошибка ЭКЛЗ
const int EKLZError = 0x1000;

/// ЭКЛЗ близка к заполнению
const int EKLZNearEnd = 0x2000;

/// Ошибка контрольной ленты (если ФР имеет контр. ленту и
/// есть связанная с ней неисправность - ФР печатать не будет)
const int ControlPaperEnd = 0x4000;

/// Внутренний механизм не приведен в рабочее положение - рычаги не
/// опущены/подняты, др.
const int Mechanism_PositionError = 0x8000;

/// Ошибка отрезчика
const int CutterError = 0x10000;

/// Ошибка электроники
const int ElectronicError = 0x20000;

/// Ошибка электроники
const int FiscalMemoryNearEnd = 0x40000;

//--------------------------------------------

/// Принтер недоступен
const int PrinterNotAvailable = 0x80000000;

//--------------------------------------------
// тип запроса состояния принтера
namespace Type {
enum Enum { isFiscalOK, isPrinterOK, isStatusOK };
} // namespace Type

namespace Param {
const QString PrinterOK = "0";
const QString PaperEnd = "1";
const QString PaperNearEnd = "2";
const QString PaperJam = "3";
const QString UnknownCommand = "4";
const QString PowerSupplyError = "5";
const QString PrintingHeadError = "6";
const QString PortError = "7";
const QString PrinterError = "8";
const QString TemperatureError = "9";
const QString CoverIsOpened = "10";
const QString FiscalMemoryError = "11";
const QString EKLZError = "12";
const QString EKLZNearEnd = "13";
const QString ControlPaperEnd = "14";
const QString Mechanism_PositionError = "15";
const QString CutterError = "16";
const QString ElectronicError = "17";
const QString FiscalMemoryNearEnd = "18";
const QString PrinterNotAvailable = "19";
const QString NoStatus = "20";
const QString UnknownError = "21";
} // namespace Param
} // namespace PrinterState

namespace ASCII {
const char NUL = '\x00';
const char ETX = '\x02';
const char STX = '\x03';
const char EOT = '\x04';
const char ENQ = '\x05';
const char ACK = '\x06';
const char DLE = '\x10';
const char NAK = '\x15';
const char ESC = '\x1B';
const char GS = '\x1D';

const char CR = '\x0D';
const char LF = '\x0A';

const char Zero = '\x30';
const char Space = '\x20';
const char Dot = '\x2E';
} // namespace ASCII

namespace CScharsetParam {
/// Тэги для разных шрифтов
const QString FontTypeBold = "b";
const QString FontTypeItalic = "i";
const QString FontTypeUnderLine = "u";
const QString FontTypeDoubleWidth = "dw";
const QString FontTypeDoubleHeight = "dh";
/// Тэг для штрих-кода
const QString BarCodeTag = "bc";
const QString SpaceCount = "p";
const QString StarCount = "s";
const QString MinusCount = "m";
const QString CenterCount = "c";

const QString OpenTagDelimiter = "[";
const QString CloseTagDelimiter = "]";
const QString CloseTagSymbol = "/";
} // namespace CScharsetParam

class BasePrinterDevices : public QThread {

public:
    BasePrinterDevices(QObject *parent = 0);

    QSerialPort *serialPort{};

    bool Debugger;
    bool devicesCreated;
    bool is_open{};

    QString com_Name;
    QString company_name;
    int statusDevices{};
    bool smallCheck;
    int checkWidth{};
    bool viewLogoImg;
    int leftMargin{};
    bool SmallBetweenString{};
    bool counterIndicate;

    // Методы которые не надо перегружать

    // Создаем устройство для работы с портами
    bool createDevicePort();
    bool closePort();
    void setPortName(const QString comName);

    bool isOpened();
    // Печатаем в 16-ом коде
    void printDataToHex(const QByteArray &data);
    // задержка между некоторыми командами
    static void msleep(int ms) { QThread::msleep(ms); }
    bool sendCommand(QByteArray dataRequest,
                     bool getResponse,
                     int timeResponse,
                     bool &respOk,
                     QByteArray &dataResponse,
                     int timeSleep);
    QByteArray encodingString(const QString &text, const QByteArray charCode);
    void setFirm_Pattern(const QString firmName);
    void setCheckWidth(int width);
    void setCheckSmall(bool smallChek);
    void setLeftMargin(int leftMargin);
    QByteArray packetImage(const QString &aPixelString, uchar aWidth);
    void setSmallBetweenString(bool beetwen);

    QByteArray asciiNull();
};

// возвращает значением бит (1/0)

bool getBit(char aValue, int aShift);

//--------------------------------------------------------------------------------

/// применяет векторизованное наложение маски
/// - каждый положительный бит маски сравнивается с соответствующим битом
/// аргумента Возвращается true, если все 1-биты совпали
bool positiveMasking(char aValue, char aMask);

/// применяет векторизованное наложение маски
/// - каждый 0-й бит маски сравнивается с соответствующим битом аргумента
/// Возвращается true, если все 0-биты совпали
bool negativeMasking(char aValue, char aMask);
