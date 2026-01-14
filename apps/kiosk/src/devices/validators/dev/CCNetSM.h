#pragma once

// System
#include "../AbstractValidator.h"

namespace CCNetConstruct {
    const int poolingTimeout = 220;
    const int MaxRepeatPacket = 3;
    const int MinAnswerSize = 6;
    const int FwPacketSize = 518;

    const char ACK = '\x00';
    const char NAK = '\xFF';

    enum MessHeader { Sync = '\x02' };

    enum PeripheralAddresses {
        PAForbidden = '\x00',
        PABillToBillUnit = '\x01',
        PACoinChanger = '\x02',
        PABillValidator = '\x03',
        PACardReader = '\x04'
        /* next reserved for future */
    };

    /* \enum ControllerCommands
                    Команды запросов
            */
    enum ControllerCommands {
        CCAck = '\x00',
        CCReset = '\x30',
        CCGetStatus = '\x31',
        CCSetSecurity = '\x32',
        CCPoll = '\x33',
        CCEnableBillTypes = '\x34',
        CCStack = '\x35',
        CCReturn = '\x36',
        CCIdentification = '\x37',
        CCHold = '\x38',
        CCSetBarcodeParam = '\x39',
        CCExtractBarcodeData = '\x3A',
        CCGetBillTable = '\x41',
        CCDownload = '\x50', /* команда с подкомандами */
        CCGetCRC32Code = '\x51',
        CCRequestStatistics = '\x60' /* команда с длинным ответом */
    };

    /// Коды cостояний
    namespace States {
        /// The state of the Bill Validator after power up.
        const char PowerUp = '\x10';

        /// After a RESET command from the Controller Bill Validator returns the bill
        /// and continues initializing.
        const char PowerUpInValidator = '\x11';

        /// After the Bill Validator is reset and INITIALIZING is complete, status will
        /// immediately change to STACKED(81H) (Credit Recovery feature).
        const char PowerUpInStacker = '\x12';

        /// Bill Validator executes initialization after the RESET command from
        /// Controller.
        const char Initialize = '\x13';

        /// Bill Validator waits for an inserting of bill into its bill path.
        const char Idling = '\x14';

        /// Bill Validator executes scanning of a bill and determines its denomination.
        const char Accepting = '\x15';

        /// Bill Validator transports a bill from Escrow position to drop cassette and
        /// remains in this state until the bill is stacked or jammed.
        const char Stacking = '\x17';

        /// Bill Validator transports a bill from Escrow position back to customer and
        /// remains in this state until customer removes the bill or the bill is jammed.
        const char Returning = '\x18';

        /// Bill Validator has been disabled by the Controller or just came out of
        /// initialization.
        const char Disabled = '\x19';

        /// The state, in which the bill is held in Escrow position after the HOLD
        /// command of the Controller.
        const char Holding = '\x1A';

        /// Bill Validator cannot answer with a full-length message right now.
        /// On expiration of time YH, peripheral is accessible to polling.
        /// YH is expressed in multiple of 100 milliseconds.
        const char DeviceBusy = '\x1B';

        /// Always followed by rejection reason byte (see below).
        const char Rejecting = '\x1C';

        /// Escrow position, Y = bill type (0 to 23).
        /// If bill type is enabled with escrow the Bill Validator waits command from
        /// Controller to stack or to return bill. If during 10 sec command will not be
        /// sent bill will be returned.
        const char Escrow = '\x80';

        /// Bill stacked
        /// Y = bill type (0 to 23)
        const char Stacked = '\x81';

        /// Bill returned
        /// Y = bill type (0 to 23)
        const char Returned = '\x82';

        /// Bill unloading
        /// B2B is moving the bill(s) from recycling cassette to drop cassette.
        const char Unloading = '\x1E';

        /// Setting type cassette
        /// The unloading of the recycling cassette is carried out, and if it is
        /// necessary, reprogramming EEPROM.
        const char SettingStackerType = '\x21';

        /// Dispensing
        /// B2B moves the bill(s) from recycling cassette to dispenser or
        /// remains in this state until customer take the bill(s) from dispenser.
        const char Dispensing = '\x25';

        /// Dispensed
        /// Dispensing is completed.
        const char Dispensed = '\x25';

        /// Unloaded
        /// Unloading is completed.
        const char Unloaded = '\x26';

        /// Invalid bill number
        /// Required number of bills is incorrect.
        const char InvalidBillNumber = '\x28';

        /// Set type cassette
        /// Setting recycling cassette type is completed.
        const char SetStackerType = '\x29';

        /// Invalid command
        /// Command from the Controller is not valid.
        const char InvalidCommand = '\x30';
    } // namespace States

    /// Коды выбросов
    namespace Rejects {
        /// Rejecting due to Insertion.
        const char Insertion = '\x60';

        /// Rejecting due to Magnetic.
        const char Dielectric = '\x61';

        /// Rejecting due to Remained bill in head.
        const char PreviouslyBillInHead = '\x62';

        /// Rejecting due to Multiplying.
        const char Compensation = '\x63';

        /// Rejecting due to Conveying.
        const char BillTransport = '\x64';

        /// Rejecting due to Identification.
        const char Identification = '\x65';

        /// Rejecting due to Verification.
        const char Verification = '\x66';

        /// Rejecting due to Optic.
        const char Optical = '\x67';

        /// Rejecting due to Inhibit.
        const char Inhibit = '\x68';

        /// Rejecting due to Capacity.
        const char Capacitance = '\x69';

        /// Rejecting due to Operation.
        const char Operation = '\x6A';

        /// Rejecting due to Length.
        const char Length = '\x6C';

        /// Rejecting due to unrecognised.
        /// Bill taken was treated as a barcode but no reliable data can.
        const char Unrecognised = '\x92';

        /// Rejecting due to UV.
        /// Banknote UV properties do not meet the predefined criteria.
        const char UV = '\x6D';

        /// Rejecting due to incorrect number of characters in barcode.
        /// Barcode data was read (at list partially) but is inconsistent.
        const char IncorrectBarcode = '\x93';

        /// Rejecting due to unknown barcode start sequence.
        /// Barcode was not read as no synchronization was established.
        const char UnknownBarcode = '\x94';

        /// Rejecting due to unknown barcode stop sequence.
        /// Barcode was read but trailing data is corrupt..
        const char CorruptedTrailingData = '\x95';
    } // namespace Rejects

    /// Коды ошибок
    namespace Errors {
        /// Drop Cassette full condition.
        const char StackerFull = '\x41';

        /// The Bill Validator has detected the drop cassette to be open or removed.
        const char BadStackerPosition = '\x42';

        /// A bill(s) has jammed in the acceptance path.
        const char ValidatorJammed = '\x43';

        /// A bill has jammed in drop cassette.
        const char StackerJammed = '\x44';

        /// Bill Validator sends this event if the intentions of the user to deceive
        /// the Bill Validator are detected.
        const char Cheated = '\x45';

        /// When the user tries to insert a second bill when the previous bill is in the
        /// Bill Validator but has not been stacked. Thus Bill Validator stops motion of
        /// the second bill until the second bill is removed.
        const char Pause = '\x46';

        /// Generic Failure codes.
        const char Failure = '\x47';

        /// Bill to Bill unit Jammed
        const char BillJammed = '\x48';
    } // namespace Errors

    /// Коды неисправностей
    namespace Failures {
        /// Drop Cassette Motor failure.
        const char StackerMotor = '\x50';

        /// Transport Motor Speed Failure.
        const char TransportMotorSpeed = '\x51';

        /// Transport Motor failure.
        const char TransportMotor = '\x52';

        /// Aligning Motor Failure.
        const char AligningMotor = '\x53';

        /// Initial Cassette Status Failure.
        const char InitialStackerStatus = '\x54';

        /// One of the optic sensors has failed to provide its response.
        const char Optical = '\x55';

        /// Inductive sensor failed to respond.
        const char Inductive = '\x56';

        /// Cassette 1 Motor Failure.
        const char Cassette1MotorF = '\x57';

        /// Cassette 2 Motor Failure.
        const char Cassette2MotorF = '\x58';

        /// Cassette 3 Motor Failure.
        const char Cassette3MotorF = '\x59';

        /// Bill-to-Bill unit Transport Failure.
        const char BillToBillTransport = '\x5A';

        /// Switch Motor 1 Failure.
        const char SwitchMotor1 = '\x5B';

        /// Switch Motor 2 Failure.
        const char SwitchMotor2 = '\x5C';

        /// Dispenser Motor 1 Failure.
        const char DispenserMotor1 = '\x5D';

        /// Dispenser Motor 2 Failure.
        const char DispenserMotor2 = '\x5E';

        /// Capacitance sensor failed to respond.
        const char Capacitance = '\x5F';
    } // namespace Failures

    /// Bill-to-Bill unit Jammed
    namespace BillJammed {
        /// Bill Jammed in Cassette 1.
        const char JCassette1 = '\x70';

        /// Bill Jammed in Cassette 2.
        const char JCassette2 = '\x71';

        /// Bill Jammed in Cassette 3.
        const char JCassette3 = '\x72';

        /// Bill Jammed in Transport Path.
        const char JTransportPath = '\x73';

        /// Bill Jammed in Switch.
        const char JSwitch = '\x74';

        /// Bill Jammed in Dispenser.
        const char JDispenser = '\x75';
    } // namespace BillJammed

    // Валюта
    namespace Nominal_TJ {
        const char nom_1 = '\x00';
        const char nom_3 = '\x01';
        const char nom_5 = '\x02';
        const char nom_10 = '\x03';
        const char nom_20 = '\x04';
        const char nom_50 = '\x05';
        const char nom_100 = '\x06';
        const char nom_200 = '\x07';
        const char nom_500 = '\x08';
    } // namespace Nominal_TJ
} // namespace CCNetConstruct

class BaseValidatorDevices;
class CCNetSm : public BaseValidatorDevices {
    Q_OBJECT

  public:
    CCNetSm(QObject *parent = 0);

    bool OpenPort();
    bool isItYou();
    bool CmdGetStatus();
    bool CmdRestart();
    void CmdStartPoll();
    void CmdStopPoll();
    bool CmdFirmwareUpdate(QString version);

    bool stopPoll;
    bool hasDBError;

    QString PartNumber;
    QString SerialNumber;
    QString AssetNumber;

    bool maxSumReject;
    int maxSum;

  private:
    bool sts_animate_dlg;
    int status;
    QDateTime preDateTime;
    bool execCommand(ValidatorCommands::Enum cmdType, QByteArray &cmdResponse);
    bool openPort();
    QByteArray makeCustomRequest(int adr, int cmd, const QByteArray &data);
    void ParsIdentification(QByteArray respData);
    int readPollInfo(QByteArray byte);
    void setBoolingDlgState(bool sts);
    void setReturnNominalState(bool sts);
    bool validatorLogEnable;

    int getNominal(const uchar nom);

    int nominalSum;
    bool escrowed;

    TResult processCommand(const QByteArray &aCommandData, QByteArray &aAnswerData);
    TResult getAnswer(QByteArray &aAnswerData);

    QLibrary m_lib;

    QByteArray fwCmdRequest(const QByteArray &data, quint8 command = 0);
    QByteArray fwPacketRequest(quint8 command, quint8 address, const QByteArray &data);
    QByteArray fwPacketUpdRequest(quint16 adr, const QByteArray &data);

    QByteArray firmwareGet(QString version);

    bool fwCmdExec(const QByteArray aCommandData);
    bool serviceModeSwitch();
    bool resetValidator();
    bool unlockValidator();
    bool checkBootloader();
    bool firmwareUpdate(const QByteArray fw);

    bool firmwareUpdating;

    bool fwUpdateC100(QString version);
    void fwCancelC100();

  private slots:
    void sendStatusTo(int sts, QString comment);
    void toValidatorLog(int status, QByteArray data, QString text);

  protected:
    bool readAnswers(QList<QByteArray> &aAnswers, int aTimeout);

    bool sendACK();

    ushort calcCRC16(const QByteArray &aData);

    QString check(const QByteArray &aAnswer);

  signals:
    void emitNominal(int nominal);
    void emitNominalDuplicate(int nominal);
    void emitStatus(int sts, QString stsComment);
    void emitAnimateStatus(bool status);
    void emitReturnNominalStatus(bool status);
    void emitLog(int status, QString title, QString text);
    void emitValidatorLog(int status, QByteArray data, QString text);
    void emitFirmwareUpdate(QString state);
};
