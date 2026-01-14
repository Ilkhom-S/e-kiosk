#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QLibrary>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <Common/QtHeadersEnd.h>

// System
#include "../ConstantData.h"

struct NominalPar {
    int index;
    int nominal;
    QString currency;
};

namespace ValidatorConstants {
    //    const int CCNetSm_CharTimeOut           =   50;
    //    const int CCNetSm_AfterRequestTimeOut   =   200;
}

namespace ValidatorCommands {
    Q_NAMESPACE

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
        SetOptionFunction = 14,
        SetSettingsEnable = 15,
        Stack1 = 16,
        Restart = 17,
        FirmwareUpdate = 18
    };
    Q_ENUM_NS(Enum)

    //    namespace NominalType {
    //        const QString nTJ = "TJ";
    //        const QString nRU = "RU";
    //    }
}; // namespace ValidatorCommands

/// Коды ошибок абстрактного валидатора
namespace ValidatorErrors {
    enum Enum {
        /// Всё окей, ошибок нет
        OK = 0,
        /// Валидатор не доступен
        NotAvailabled = 1,
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
} // namespace ValidatorErrors

namespace VStatus {
    namespace Success {
        enum S { Ok = 0 };
    } // namespace Success

    namespace Errors {
        enum E {
            StackerJammed = 1,
            BadStackerPosition = 2,
            StackerFull = 3,
            ValidatorJammed = 4,
            Failure = 5,
            StackerMotor = 6,
            TransportMotorSpeed = 7,
            TransportMotor = 8,
            AligningMotor = 9,
            InitialStackerStatus = 10,
            Optical = 11,
            Cassette1MotorF = 12,
            Cassette2MotorF = 13,
            Cassette3MotorF = 14,
            BillToBillTransport = 15,
            SwitchMotor1 = 16,
            SwitchMotor2 = 17,
            DispenserMotor1 = 18,
            DispenserMotor2 = 19,
            Inductive = 20,
            Capacitance = 21,
            JCassette1 = 22,
            JCassette2 = 23,
            JCassette3 = 24,
            JTransportPath = 25,
            JSwitch = 26,
            JDispenser = 27,
            NotAvailable = 28
        };
    } // namespace Errors

    namespace Warning {
        enum {
            PreviouslyBillInHead = 30,
            Operation = 31,
            Insertion = 32,
            Dielectric = 33,
            Compensation = 34,
            BillTransport = 35,
            Identification = 36,
            Verification = 37,
            Optical = 38,
            Inhibit = 39,
            Capacitance = 40,
            Length = 41,
            Unrecognised = 42,
            UV = 43,
            IncorrectBarcode = 44,
            UnknownBarcode = 45,
            CorruptedTrailingData = 46,
            Cheated = 47,
            Paused = 48,
            Rejected = 49,
            Calibration = 50,
            NoPushMode = 51,
            FlashDownload = 52,
            PreStack = 53,
            InvalidCommand = 54,
            LengthDoubling = 55,
            WidthDoubling = 56,
            EscrowTimeout = 57,
            Width = 58,
            UnableToStack = 59
        };
    } // namespace Warning
} // namespace VStatus

class BaseValidatorDevices : public QThread {

  public:
    BaseValidatorDevices(QObject *parent = 0);
    QSerialPort *serialPort;
    bool devicesCreated;
    bool is_open;
    int statusDevices;
    QString comName;
    QString part_number;

    bool debugger;

    // Создаем устройство для работы с портами
    bool createDevicePort();
    bool closePort();
    void setPortName(const QString com_Name);
    void setPartNumber(const QString partNumber);

    bool isOpened();
    // Печатаем в 16-ом коде
    void printDataToHex(const QByteArray &data);
    // задержка между некоторыми командами
    static void msleep(int ms) {
        QThread::msleep(ms);
    }
    bool sendCommand(QByteArray dataRequest, bool getResponse, int timeResponse, QByteArray &dataResponse,
                     int timeSleep, bool readAll);

    QString cmdName(ValidatorCommands::Enum cmd);
};

