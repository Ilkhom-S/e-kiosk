#ifndef CCTalk_H
#define CCTalk_H

// System
#include "../AbstractAcceptor.h"

namespace CCTalkConstruct {
const char NAK = '\x05';
const char BUSY = '\x06';

/// Константа для вычисления контрольной суммы.
const ushort Polynominal = 0x1021;

/// Последний бит для вычисления CRC.
const ushort LastBit = 0x8000;

/// Минимальный размер ответного пакета.
const int MinAnswerSize = 5;

/// Максимальное количество повторов из-за BUSY устройства.
const int MaxBusyNAKRepeats = 3;

/// Таймауты, [мс].
namespace Timeouts {
/// Повтор после BUSY или NAK-а.
const int NAKBusy = 1000;

/// Дефолтный для ожидания ответа.
const int Reading = 500;
} // namespace Timeouts

enum Address { Host = 0x01, CoinAcceptor = 0x02 };

enum perekl {
  ApZero = 0x00,
  ApOne = 0x01,
  ApTwo = 0x02,
  ApThree = 0x03,
  ApFour = 0x04
};

enum ControllerCommands {
  ApReset = 0x01,
  ApRequestCommStatus = 0x02,
  ApClearCommStatus = 0x03,
  ApRequestCommsRevision = 0x04,
  ApStoreEncyptionCode = 0x88,
  ApSwitchEncryptionCode = 0x89,
  ApRequestBillOperationMode = 0x98,
  ApModifyBillOperationMode = 0x99,
  ApAcceptBill = 0x9A,
  ApRequestBillPosition = 0x9B,
  ApRequestCountryScalingFactor = 0x9C,
  ApRequestBillId = 0x9D,
  ApReadBufferedBillEvents = 0xE5,
  ApRequestAdressMode = 0xA9,
  ApRequestSequritySetting = 0xB4,
  ApModifySequritySetting = 0xB5,
  ApRequestCoinId = 0xB8,
  ApRequestBuildCode = 0xC0,
  ApSendConfigurationToEEPROM = 0xC7,
  ApRequestOptionsFlag = 0xD5,
  ApRequestDataStorageAvailability = 0xD8,
  ApReadMasterInhibitStatus = 0xE3,
  ApModifyMasterInhibitStatus = 0xE4,
  ApRequestInhibitStatus = 0xE6,
  ApModifyInhibitStatus = 0xE7,
  APPerformSelfCheck = 0xE8,
  ApRequestSoftwareRevision = 0xF1,
  ApRequestSerialNumber = 0xF2,
  ApRequestProductCode = 0xF4,
  ApRequestCategoryId = 0xF5,
  ApRequestManufactureId = 0xF6,
  ApRequestPollingPriority = 0xF9,
  ApAddressChangeRandom = 0xFA,
  ApAddressChange = 0xFB,
  ApAddressClash = 0xFC,
  ApAddresPoll = 0xFD,
  ApSimplePoll = 0xFE
};

namespace States {
const char Accepting = 0x00;
const char RejectCoin = 0x01;
const char InhibitedCoin = 0x02;
const char MultipleWindow = 0x03;
const char WakeUpTimeout = 0x04;
const char ValidationTimeout = 0x05;
const char CreditSensorTimeout = 0x06;
const char SorterOptoTimeout = 0x07;
const char SecondCloseCoinError = 0x08;
const char AcceptGateNotReady = 0x09;
const char CreditSensorNotReady = 0x0A;
const char SorterNotReady = 0x0B;
const char RejectCoinNotCleared = 0x0C;
const char ValidationSensorNotReady = 0x0D;
const char CreditSensorBlocked = 0x0E;
const char SorterOptoBlocked = 0x0F;
const char CreditSequenceError = 0x10;
const char CoinGoingBackwards = 0x11;
const char CoinTooFast = 0x12;
const char CoinTooSlow = 0x13;
const char CoinOnStringMechanismActivated = 0x14;
const char DCEOptoTimeout = 0x15;
const char DCEOptoNotSeen = 0x16;
const char CreditSensorReachedTooEarly = 0x17;
const char RejectCoinRepeatedSequentialTrip = 0x18;
const char RejectSlug = 0x19;
const char RejectSensorBlocked = 0x1A;
const char GamesOverload = 0x1B;
const char MaxCoinMeterPulsesExceeded = 0x1C;
const char AcceptGateOpenNotClosed = 0x1D;
const char AcceptGateClosedNotOpen = 0x1E;
const char DataBlockRequest = 0xFD;
const char CoinReturnMechanismActivated = 0xFE;
const char UnspecifiedAlarmCode = 0xFF;
} // namespace States
} // namespace CCTalkConstruct

class BaseAcceptorDevices;
class CCTalk : public BaseAcceptorDevices {
  Q_OBJECT

public:
  CCTalk();
  bool OpenPort();
  bool isItYou();
  bool CmdGetStatus();
  bool CmdRestart();
  void CmdStartPoll();
  void CmdStopPoll();
  void CmdInit();

  bool stopPoll;

  QString PartNumber;
  QString SerialNumber;
  QString AssetNumber;

private:
  bool debugger;
  void toDebug(QString data);

  int coin_ev_counter;
  int events_in_queue;

  int coin_status;

  bool sts_animate_dlg;
  int status;
  double preDate;
  int preNominal;
  bool execCommand(int cmdType, QByteArray &cmdResponse,
                   QByteArray data = QByteArray());
  bool openPort();

  QByteArray makeCustomRequest(int cmd, const QByteArray &cmdResponse);

  int readPollInfo(QByteArray byte);
  void setBoolingDlgState(bool sts);

  bool checkCoin;
  bool coinAcceptorLogEnable;

  QMap<int, QString> billTable;
  QMap<QString, int> billValue;

  void setBillConstData();

  QString billTableResp;

  TResult processCommand(const QByteArray &aCommandData,
                         QByteArray &aAnswerData);

protected:
  ushort calcCRC8(const QByteArray &aData);
  bool check(QByteArray &aAnswer);
  bool getAnswer(QByteArray &aAnswer, const QByteArray &aCommandData);

private slots:
  void sendStatusTo(int sts, QString comment);
  void toCoinAcceptorLog(int status, QByteArray data, QString text);
  void clearCoin();

signals:
  void emitNominal(int nominal);
  void emitNominalDuplicate(int nominal);
  void emitStatus(int sts, QString stsComment);
  void emitAnimateStatus(bool status);
  void emitLoging(int status, QString title, QString text);
  void emitBillTable(QString bill_table);
  void emitCoinAcceptorLog(int status, QByteArray data, QString text);
};

#endif // CCTalk_H
