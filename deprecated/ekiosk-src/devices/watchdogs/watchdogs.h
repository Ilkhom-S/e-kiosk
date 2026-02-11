#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtSerialPort/QSerialPort>

namespace COSMP1 {
/// Имя девайса
const QString DeviceName = "OSMP1";

/// При определении железки
const QString DeviceID = "v1";

/// ACK
const uchar ACK = 0x50;

/// NAK
const uchar NAK = 0x80;

/// Пустой байт
const uchar EmptyByte = 0x00;

/// Минимальный размер ответного пакета
const int MinAnswerSize = 4;

/// Максимальный таймаут (секунд)
const int MaxAnswerTimeout = 2;

/// Размер постоянной основы для пакета
const int PacketConstSize = 3;

/// Постоянная основа для пакета
const uchar PacketConst[3] = {0x4F, 0x53, 0x50};

namespace Commands {
/// Ребут компа
const uchar RebootPC = 0xAE;

/// Сброс модема
const uchar ResetModem = 0x02;

/// Запрос активности компа (сигнал об отсутствии зависания)
const uchar PCEnable = 0x05;

/// ИД девайса
const uchar GetID = 0x01;

/// Старт таймера
const uchar StartTimer = 0x03;

/// Стоп таймера
const uchar StopTimer = 0x04;
}; // namespace Commands
}; // namespace COSMP1

namespace WDProtocolCommands {
enum Enum {
    /// Ребут компа
    RebootPC,

    /// Сброс модема
    ResetModem,

    /// Запрос активности компа (сигнал об отсутствии зависания)
    PCEnable,

    /// ИД девайса
    GetID,

    /// Старт таймера
    StartTimer,

    /// Стоп таймера
    StopTimer
};
}; // namespace WDProtocolCommands

class WatchDogs : public QThread {
    Q_OBJECT

public:
    WatchDogs(QObject *parent = 0);
    bool isItYou(QStringList &comList, QString &wdName, QString &comStr, QString &wdComent);
    bool sendCommandToExec(WDProtocolCommands::Enum aCommand);
    bool toCommandExec(bool thread, WDProtocolCommands::Enum aCommand);
    void setPort(const QString comName);

private:
    QSerialPort *serialPort;
    bool devicesCreated;
    bool is_open;
    QString com_Name;

    WDProtocolCommands::Enum nowCommand;
    bool createDevicePort();
    bool closePort();

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

    bool openPort();
    bool isItYou(QString &wdComent);
    bool processCommand(WDProtocolCommands::Enum aCommand,
                        const QByteArray &aCommandData,
                        QByteArray &aAnswerData);

    void run();

signals:
    void commandDone(bool state, int aCommand);
};
