#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include "../ConstantData.h"

namespace AcceptorCommands {
enum Enum {
    Reset = 1,
    GetNominalTable = 2,
    SetNominalTable = 3,
    SetEnabled = 4,
    Poll = 5,
    Stack = 6,
    Return = 7,
    StartPolling = 8,
    StopPolling = 9,
    Idintification = 10,
    SetSecurity = 11,
    ACK = 12,
    SetDisabled = 13,
    RequestProductCode = 14,
    SetSettingsEnable = 15,
    RequestSerialNumber = 16,
    Restart = 17,
    Com_Clear = 18,
    Perform_SelfCheck = 19
};
}; // namespace AcceptorCommands

/// Коды ошибок абстрактного валидатора
namespace AcceptorErrors {
enum Enum {
    /// Всё окей, ошибок нет
    OK = 0,

    /// Валидатор не доступен
    NotAvailable = 1,

    /// Валюта не поддерживается (настройки terminal.xml и прошивка валидатора не
    /// совпадают)
    WrongCurrency = 2,

    /// Купюра удерживается в валидаторе (не уложена в стекер).
    Holding = 3,

    /// Валидатор занят, позвоните попозже.
    DeviceBusy = 4,

    /// Отбраковка купюры. Неверно вставлена.
    RejectInsertion = 5,

    /// Отбраковка купюры. Магнетик.
    RejectDielectric = 6,

    /// Отбраковка купюры. Предыдущая купюра застряла в хедере девайса.
    RejectPreviouslyInsertedBillRemainsInHead = 7,

    /// Отбраковка купюры. Засунуто несколько купюр.
    RejectCompensation = 8,

    /// Отбраковка купюры. Транспорт купюры.
    RejectBillTransport = 9,

    /// Отбраковка купюры. Идентификация купюры.
    RejectIdentification = 10,

    /// Отбраковка купюры. Верификация купюры.
    RejectVerification = 11,

    /// Отбраковка купюры. Оптические сенсоры.
    RejectOpticSensor = 12,

    /// Отбраковка купюры. Запрещенная купюра.
    RejectInhibitDenomination = 13,

    /// Отбраковка купюры. Емкостные сенсоры.
    RejectCapacitance = 14,

    /// Отбраковка купюры. Ошибка операции.
    RejectOperation = 15,

    /// Отбраковка купюры. Ошибка длины (купюры).
    RejectLength = 16,

    /// Отбраковка купюры. Недостаточно надежно распознана.
    RejectUnrecognised = 17,

    /// Отбраковка купюры. Ультрафиолет.
    RejectUV = 18,

    /// Отбраковка купюры. Не совпадает штрих-код с номиналом.
    RejectIncorrectNumberOfCharactersInBarcode = 19,

    /// Отбраковка купюры. Неверная начальная последовательность штрих-кода.
    RejectUnknownBarcodeStartSequence = 20,

    /// Отбраковка купюры. Неверная финальная последовательность штрих-кода.
    RejectTrailingDataIsCorrupt = 21,

    /// Отбраковка купюры. Причина неизвестна.
    RejectUnknown = 22,

    /// Стекер полон.
    DropCassetteFull = 23,

    /// Стекер открыт или вытащен.
    DropCassetteOutOfPosition = 24,

    /// Зажевало купюру в валидаторе.
    ValidatorJammed = 25,

    /// Зажевало купюру в стекере.
    DropCassetteJammed = 26,

    /// Попытка мошеничества
    Cheated = 27,

    /// Попытка сунуть вторую купюру, пока первая ещё не проглочена стекером
    Pause = 28,

    /// Ошибка прошивки (boot).
    FailureBOOTError = 29,

    /// Убран хедер валидатора.
    FailureValidatorHeadRemove = 30,

    /// Неисправность. Мотор стекера.
    FailureStackMotor = 31,

    /// Неисправность. Скорость транспортирующего мотора.
    FailureTransportMotorSpeed = 32,

    /// Неисправность. Транспортирующий мотор.
    FailureTransportMotor = 33,

    /// Неисправность. Выравнивающий мотор.
    FailureAligningMotor = 34,

    /// Неисправность. Неверный статус инициализации кассеты.
    FailureInitialCassetteStatus = 35,

    /// Неисправность. Оптический канал.
    FailureOpticCanal = 36,

    /// Неисправность. Магнитный канал.
    FailureMagneticCanal = 37,

    /// Неисправность. Емкостной канал.
    FailureCapacitanceCanal = 38,

    /// Неисправность. Неизвестна.
    FailureUnknown = 39,

    /// Ошибка выполнения команды stackBill
    StackBillError = 40,

    /// Ошибка выполнения команды returnBill
    ReturnBillError = 41,

    /// Неизвестен
    Unknown = 42,

    /// Ошибка исправлена. Технологический статус, только в ICT00х
    ErrorExlusion = 43,

    PortError = 44,

    DevicesError = 45
};
} // namespace AcceptorErrors

namespace CCtalkStatus {
namespace Success {
enum S { Ok = 0 };
} // namespace Success

namespace Errors {
enum E {
    RejectCoin = 1,
    InhibitedCoin = 2,
    MultipleWindow = 3,
    WakeUpTimeout = 4,
    ValidationTimeout = 5,
    CreditSensorTimeout = 6,
    SorterOptoTimeout = 7,
    SecondCloseCoinError = 8,
    AcceptGateNotReady = 9,
    CreditSensorNotReady = 10,
    SorterNotReady = 11,
    RejectCoinNotCleared = 12,
    ValidationSensorNotReady = 13,
    CreditSensorBlocked = 14,
    SorterOptoBlocked = 15,
    CreditSequenceError = 16,
    CoinGoingBackwards = 17,
    CoinTooFast = 18,
    CoinTooSlow = 19,
    CoinOnStringMechanism_Activated = 20,
    DCEOptoTimeout = 21,
    DCEOptoNotSeen = 22,
    CreditSensorReachedTooEarly = 23,
    RejectCoinRepeatedSequentialTrip = 24,
    RejectSlug = 25,
    RejectSensorBlocked = 26,
    GamesOverload = 27,
    MaxCoinMeterPulsesExceeded = 28,
    AcceptGateOpenNotClosed = 29,
    AcceptGateClosedNotOpen = 30,
    DataBlockRequest = 31,
    CoinReturnMechanism_Activated = 32,
    UnspecifiedAlarm_Code = 33,
    NotAvailable = 34
};
} // namespace Errors
} // namespace CCtalkStatus

class BaseAcceptorDevices : public QThread {

public:
    BaseAcceptorDevices(QObject *parent = 0);
    QSerialPort *serialPort;

    int Debugger;
    bool devicesCreated;
    bool is_open;
    int statusDevices;
    QString com_Name;
    QString part_number;

    // Создаем устройство для работы с портами
    bool createDevicePort();
    bool closePort();
    void setPortName(const QString com_Name);
    void setPartNumber(const QString partNumber);

    bool isOpened();

    static void msleep(int ms) { QThread::msleep(ms); }

protected:
    bool sendCommand(QByteArray dataRequest,
                     bool getResponse,
                     int timeResponse,
                     QByteArray &dataResponse,
                     int timeSleep);
};
