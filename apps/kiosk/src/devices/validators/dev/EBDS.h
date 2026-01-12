#ifndef EBDS_H
#define EBDS_H

#include "../AbstractValidator.h"
//#include "include/modules/SameFunction.h"


namespace EBDSConstruct
{
    const char Prefix  = '\x02';

    /// Конечный байт (не считая CRC).
    const char Postfix = '\x03';

    /// Маска для ACK.
    const char ACKMask = '\x0F';

    /// Таймаут для чтения ответа по умолчанию, [мс].
    const int AnswerTimeout = 500;

    /// Таймаут после команды Reset, [мс].
    const int ResetTimeout = 8000;

    /// Тег для идентификации Advanced-модели.
    const char AdvancedModelTag[] = "SCN";

    /// Направление приема - любое (2-3 биты).
    const char Orientation = 0x0C;

    /// Escrow - разрешено.
    const char Escrow = 0x10;

    /// 1-й байт по дефолту - складывается из направления приема и escrow.
    const char Byte1 = Orientation | Escrow;

    /// 2-й байт по дефолту - включен расширенный режим, "push" режим.
    const char Byte2 = 0x10;

    /// Биты команд.
    const char Stack  = 0x20;
    const char Return = 0x40;

    /// Размер используемой части блока данных 1 номинала в ответе на запрос номиналов.
    const int NominalSize = 16;

    /// Количество номиналов.
    const int NominalCount = 50;

    namespace Commands {
        const char Host2Validator   = 0x10;   /// Standard Host to Acceptor messages.
        const char Validator2Host   = 0x20;   /// Standard Acceptor to Host messages.
        const char BookmarkSelected = 0x30;   /// Bookmark selected.
        const char CalibrateMode    = 0x40;   /// Calibrate Mode.
        const char FlashDownload    = 0x50;   /// Flash Download.
        const char Control          = 0x60;   /// Request CRC, Get Cash in Box, Soft Reset.
        const char Extended         = 0x70;   /// Extended message set.

        /// Для типа Control.
        const QByteArray Reset              = QByteArray::fromRawData("\x60\x7F\x7F\x7F", 4);    /// Сброс.
        const QByteArray GetType            = QByteArray::fromRawData("\x60\x00\x00\x04", 4);    /// Тип купюроприемника (что это такое - только в MEI знают).
        const QByteArray GetSerialNumber    = QByteArray::fromRawData("\x60\x00\x00\x05", 4);    /// Серийник.
        const QByteArray GetBootSoftVersion = QByteArray::fromRawData("\x60\x00\x00\x06", 4);    /// Софт загрузчика интерфейсной платы.
        const QByteArray GetAppSoftVersion  = QByteArray::fromRawData("\x60\x00\x00\x07", 4);    /// Софт приложения головы.
        const QByteArray GetVariantName     = QByteArray::fromRawData("\x60\x00\x00\x08", 4);    /// Название билл-сета (вариант, по-канадски).
        const QByteArray GetVariantVersion  = QByteArray::fromRawData("\x60\x00\x00\x09", 4);    /// Версия билл-сета.

        /// Для типа Extended.
        const char GetPar[]      = "\x70\x02";    /// Получить номинал по индексу.
        const char SetInhibits[] = "\x70\x03";    /// Установить запрещения номиналов.
    }

    //Byte 0
    namespace State_0
    {
        const int Idling    = 0;
        const int Accepting = 1;
        const int Escrowed  = 2;
        const int Stacking  = 3;
        const int Stacked   = 4;
        const int Returning = 5;
        const int Returned  = 6;
    }

    namespace State_1
    {
        const char Cheated      = 0;
        const char Rejected     = 1;
        const char Jammed       = 2;
        const char CassetteFull = 3;
        const char LRCPresent   = 4;
        const char Paused       = 5;
        const char Calibration  = 6;
    }

    namespace State_2
    {
        const char PowerUp        = 0;
        const char InvalidCommand = 1;
        const char Failure        = 2;
    }

    namespace State_3
    {
        const char NoPushMode    = 0;
        const char FlashDownload = 1;
        const char PreStack      = 2;
    }

    /// Коды ошибок
    namespace Errors
    {
        /// Drop Cassette full condition.
        const uchar StackerFull        = 0x41;

        /// The Bill Validator has detected the drop cassette to be open or removed.
        const uchar BadStackerPosition = 0x42;

        /// A bill(s) has jammed in the acceptance path.
        const uchar ValidatorJammed    = 0x43;

        /// A bill has jammed in drop cassette.
        const uchar StackerJammed      = 0x44;

        /// Bill Validator sends this event if the intentions of the user to deceive
        /// the Bill Validator are detected.
        const uchar Cheated            = 0x45;

        /// When the user tries to insert a second bill when the previous bill is in the Bill Validator
        /// but has not been stacked. Thus Bill Validator stops motion of the second bill until the second bill
        /// is removed.
        const uchar Pause              = 0x46;

        /// Generic Failure codes.
        const uchar Failure            = 0x47;

        /// Bill to Bill unit Jammed
        const uchar BillJammed         = 0x48;
    }

    /// Коды неисправностей
    namespace Failures
    {
        /// Drop Cassette Motor failure.
        const uchar StackerMotor         = 0x50;

        /// Transport Motor Speed Failure.
        const uchar TransportMotorSpeed  = 0x51;

        /// Transport Motor failure.
        const uchar TransportMotor       = 0x52;

        /// Aligning Motor Failure.
        const uchar AligningMotor        = 0x53;

        /// Initial Cassette Status Failure.
        const uchar InitialStackerStatus = 0x54;

        /// One of the optic sensors has failed to provide its response.
        const uchar Optical              = 0x55;

        /// Inductive sensor failed to respond.
        const uchar Inductive            = 0x56;

        /// Cassette 1 Motor Failure.
        const uchar Cassette1MotorF      = 0x57;

        /// Cassette 2 Motor Failure.
        const uchar Cassette2MotorF      = 0x58;

        /// Cassette 3 Motor Failure.
        const uchar Cassette3MotorF      = 0x59;

        /// Bill-to-Bill unit Transport Failure.
        const uchar BillToBillTransport  = 0x5A;

        /// Switch Motor 1 Failure.
        const uchar SwitchMotor1         = 0x5B;

        /// Switch Motor 2 Failure.
        const uchar SwitchMotor2         = 0x5C;

        /// Dispenser Motor 1 Failure.
        const uchar DispenserMotor1      = 0x5D;

        /// Dispenser Motor 2 Failure.
        const uchar DispenserMotor2      = 0x5E;

        /// Capacitance sensor failed to respond.
        const uchar Capacitance          = 0x5F;
    }

    /// Bill-to-Bill unit Jammed
    namespace BillJammed
    {
        /// Bill Jammed in Cassette 1.
        const uchar JCassette1           = 0x70;

        /// Bill Jammed in Cassette 2.
        const uchar JCassette2           = 0x71;

        /// Bill Jammed in Cassette 3.
        const uchar JCassette3           = 0x72;

        /// Bill Jammed in Transport Path.
        const uchar JTransportPath       = 0x73;

        /// Bill Jammed in Switch.
        const uchar JSwitch              = 0x74;

        /// Bill Jammed in Dispenser.
        const uchar JDispenser           = 0x75;
    }
}

class BaseValidatorDevices;

class EBDS: public BaseValidatorDevices
{
    Q_OBJECT

public:
    EBDS(QObject *parent = 0);

    bool OpenPort();
    bool isItYou();
    bool CmdGetStatus();
    bool CmdRestart();
    void CmdStartPoll();
    void CmdStopPoll();

    bool maxSumReject;
    int  maxSum;

    bool stopPoll;
    bool hasDBError;

    QString partNumber;
    QString serialNumber;

private slots:
    void sendStatusTo(int sts, QString comment);
    void toLogingValidator(int status, QByteArray data, QString text);

private:
    bool sts_animate_dlg;
    bool mEnabled;
    bool mACK;

    int status;
    QDateTime preDateTime;

    bool execCommand(int cmdType, QByteArray &cmdResponse);
    TResult processCommand(QByteArray& aCommandData, QByteArray& aAnswerData, bool aNeedAnswer);

    bool getAnswer(QByteArray & aAnswer);
    bool check(const QByteArray & cmdRequest, const QByteArray & cmdResponse);

    bool openPort();

    QByteArray makeCustomRequest(const QByteArray &commandData);
    QByteArray pollRequest(const char aAction = 0);

    void parseIdentification(QByteArray respData);
    int getNominal(QByteArray respData);

    int  readPollInfo(QByteArray byte);
    void setBoolingDlgState(bool sts);
    void setReturnNominalState(bool sts);
    bool validatorLogEnable;
    bool escrowed;
    int nominalSum;

    void returnBill();
    void stackBill();

    uchar calcCRC(const QByteArray & aData);
    bool checkBit(QByteRef bytes, int bit);

signals:
    void emitNominal(int nominal);
    void emitNominalDuplicate(int nominal);
    void emitStatus(int sts, QString stsComment);
    void emitAnimateStatus(bool status);
    void emitReturnNominalStatus(bool status);
    void emitLog(int status, QString title, QString text);
    void emitValidatorLog(int status, QByteArray data, QString text);
};

#endif // EBDS_H
