#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtSerialPort/QSerialPort>

namespace CModem_Constants {
const auto regExpBalance =
    "([0-9]{1,5}[.][0-9]{1,2}|[0-9]{1,5}[,][0-9]{1,2}|[-][0-9]{1,5}[.][0-9]{1,"
    "2}|[-][0-9]{1,5}[,][0-9]{1,2})";
const auto regExpNumberSim = "[0-9]{9,12}";
} // namespace CModem_Constants

//--------------------------------------------------------------------------------
/// Константы ATProtocol протокола.
namespace CATProtocolConstants {
/// Тип протокола
const auto ProtocolType = "AT";

/// Пустой байт
const uchar EmptyByte = 0x00;

/// Минимальный размер ответного пакета
const int MinAnswerSize = 4;

/// Число дополнительных чтений для getBalance
const int GetBalanceAddRepeatCount = 10;

/// Число дополнительных чтений для GetAllSMS
const int GetAllSMSRepeatCount = 12;

/// Число дополнительных чтений для sendSMS
const int SendSMSAddRepeatCount = 5;

/// Число дополнительных чтений для getSim_Number
const int GetSim_NumberAddRepeatCount = 4;

/// Пауза между чтениями для getBalance (ms)
const int GetBalancePauseTime = 300;

/// Пауза между чтениями для getSim_Number (ms)
const int GetSim_NumberPauseTime = 300;
}; // namespace CATProtocolConstants

//--------------------------------------------------------------------------------
/// Команды ATProtocol протокола.
namespace CATProtocolCommands {
/// "Получить оператора"
const QString GetOperator = "AT+COPS?";

/// "Сбросить настройки по умолчанию"
const QString ResetSettings = "ATZ";

/// "Отключить эхо-вывод"
const QString OffEcho = "ATE0";

/// "Получить PIN симки"
const QString IsPin = "AT+CPIN?";

/// "Качество сигнала" (Signal quality)
const QString SignalQuality = "AT+CSQ";

/// "Идентификация" (Display product identification information)
const QString Identification = "ATI";

/// Получение баланса
const QString GetBalance = "ATD";

/// Получение комента
const QString Comment = "AT+GMM";

/// Отправка запросов
const QString UssdRequest = "AT+CUSD=1, \"%1\",15";

/// Неявная команда, вызываемая при получении баланса
const QString CUSD = "+CUSD";

/// Команда на перезапуск модема
const QString Restart = "AT+CFUN=0,1";

/// Команда на переход в режим смс
const QString SmsState = "AT+CMGF=0";

/// Команда на отправку смс
const QString SmsSend = "AT+CMGS=%1";

/// Команда на просмотр всех входящих SMS
const QString GetAllInputSms = "AT+CMGL=\"all\"";

// AT+CMGDA=«DEL ALL»
/// Команда на удаление всех SMS
const QString DellAllSms = "AT+CMGD=\"%1\"";

/// Точка с запятой
const QString Semicolon = ";";

/// Окончание строки
const uchar CR = 0x0D;

/// Окончание строки
const uchar LF = 0x0A;
}; // namespace CATProtocolCommands

//--------------------------------------------------------------------------------
/// Возможные ошибки
namespace ATErrors {
namespace Strings {
const QString OK = "OK";
const QString Connect = "CONNECT";
const QString Ring = "RING";
const QString NoCarrier = "NO CARRIER";
const QString Error = "ERROR";
const QString Connect600 = "CONNECT 600";
const QString Connect1200 = "CONNECT 1200";
const QString Connect2400 = "CONNECT 2400";
const QString NoDialtone = "NO DIALTONE";
const QString Busy = "BUSY";
const QString NoAnswer = "NO ANSWER";
}; // namespace Strings

enum Enum {
    /// Модем выполнил команду без ошибок
    OK,

    /// Модем установил связь
    Connect,

    /// Модем обнаружил сигнал звонка на телефонной линии.
    /// Это сообщение модем передает компьютеру каждый раз, когда по телефонной
    /// линии поступает сигнал вызова (звонок)
    Ring,

    /// Модем потерял несущую или не получил ответ от удаленного модема
    NoCarrier,

    /// Ошибка в командной строке, командный буфер переполнен или ошибка в
    /// контрольной сумме (команда I2)
    Error,

    /// Отсутствие сигнала станции при снятии трубки
    NoDialtone,

    /// Модем обнаружил сигнал "занято" после набора номера
    Busy,

    /// Ответ получается в случае использования в командной пятисекундной тишины
    NoAnswer,

    /// Неизвестен
    Unknown
};
}; // namespace ATErrors

/// Команды протокола модемов.
//--------------------------------------------------------------------------------
namespace Modem_ProtocolCommands {
enum Enum {
    /// Получить качество сигнала (в dBm)
    GetSignalQuality,

    /// Получить оператора
    GetOperator,

    /// Отключить эховывод команд
    OffEcho,

    /// Идентификация.
    Identification,

    /// Сброс модема.
    Reset,

    /// Получить PIN симки.
    IsPin,

    /// Получение баланса.
    GetBalance,

    /// Comment
    GetComment,

    /// GetSim_Number
    GetSim_Number,

    /// Restart
    CmdRestart,

    /// State SMS
    CmdStateSMS,

    /// Send SMS
    CmdSendSMS,

    /// Set Number phone SMS
    CmdSetNumberSMS,

    /// Text SMS
    CmdTextIntersetSMS,

    /// Get All SMS
    CmdGetAllSms,

    /// Dell All SMS
    CmdDellAllSms,

    /// Неизвестна
    Uknown
};
}; // namespace Modem_ProtocolCommands

namespace Modem_Cmd {

/// Получить качество сигнала (в dBm)
const QString GetSignalQuality = "GetSignalQuality";

/// Получить оператора
const QString GetOperator = "GetOperator";

/// Отключить эховывод команд
const QString OffEcho = "OffEcho";

/// Идентификация.
const QString Identification = "Identification";

/// Сброс модема.
const QString Reset = "Reset";

/// Получить PIN симки.
const QString IsPin = "IsPin";

/// Получение баланса.
const QString GetBalance = "GetBalance";

/// Получение баланса.
const QString GetComment = "GetComment";

/// Получить номер симки
const QString GetSim_Number = "GetSim_Number";

const QString CmdRestart = "CmdRestart";

/// Отправка SMS
const QString CmdSendSms = "CmdSendSms";

/// Неизвестна
const QString Uknown = "Uknown";
} // namespace Modem_Cmd

/// Коды ошибок абстрактного валидатора
namespace Modem_Errors {
enum Enum {
    /// Всё окей, ошибок нет
    OK,

    /// Нет коннекта, модем не подключен
    NotAvailable,

    /// Ошибка симки
    SIMError,

    /// Неизвестен
    Unknown
};
}; // namespace Modem_Errors

/// Состояния абстрактного валидатора.
namespace Modem_States {
enum Enum {
    /// Модем проиницилизирован, готов к работе.
    Initialize,

    /// Возникла ошибка. Код ошибки Modem_Errors::Enum
    Error,

    /// Неизвестен
    Unknown
};
}; // namespace Modem_States

/// Структура о полном статусе абстрактного модема
struct SModem_StatusInfo {
    /// Состояние модема.
    Modem_States::Enum state;

    /// Ошибка модема (в случае возникновения).
    Modem_Errors::Enum error;

    bool operator==(const SModem_StatusInfo &aModem_StatusInfo) {
        return (state == aModem_StatusInfo.state) && (error == aModem_StatusInfo.error);
    }

    bool operator!=(const SModem_StatusInfo &aModem_StatusInfo) {
        return (state != aModem_StatusInfo.state) || (error != aModem_StatusInfo.error);
    }
};

namespace SmsTextIndex {
enum sms {
    smsErrorValidator = 0,
    smsErrorPrinter = 1,
    smsErrorBalanceAgent = 2,
    smsErrorSim_Balance = 3,
    smsErrorLockTerminal = 4,
    smsErrorConnection = 5,
};

const QString txtErrorValidator = "Error validator";
const QString txtErrorPrinter = "Error printer";
const QString txtErrorBalanceAgent = "Few balance agent";
const QString txtErrorSim_Balance = "Few balance SIM";
const QString txtErrorLockTerminal = "Error lock terminal";
const QString txtErrorConnection = "Error connection";
} // namespace SmsTextIndex

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>

//--------------------------------------------------------------------------------
/// Класс протокола ATProtocol.
class ATProtocol : public QThread {

    //    Q_OBJECT

public:
    ATProtocol(QObject *parent = 0);
    bool createDevicePort();
    bool closePort();
    void setPortName(const QString com_Name);
    bool openPort();
    bool sendSMSParam(QString text);

    QSerialPort *serialPort;

    bool Debugger;
    bool devicesCreated;
    bool is_open;
    QString com_Name;
    QStringList smsTextInit;
    QString balanceRequest;
    QString sim_NumberRequest;

    QString regExpSim_Number;
    QString regExpBalance;
    int position;

    QString nowSim_Number;
    QString nowSim_Balance;
    QString nowModem_Name;
    QString nowModem_Comment;
    QString nowPortName;
    QString nowProviderSim;
    QString nowSignalQuality;
    bool nowSim_Present;
    // protected:
    /// Маска для разбора выражения баланса
    QRegularExpression m_getBalanceRegExp;
    /// Выполнить команду протокола
    ///
    /// Параметры:
    /// - порт;
    /// - команда из перечисления Modem_ProtocolCommands::Enum;
    /// - данные команды;
    /// - данные ответа;
    ///
    /// Возвращает:
    /// - успешность выполнения.
    virtual bool processCommand(Modem_ProtocolCommands::Enum aCommand,
                                const QByteArray &aCommandData,
                                QByteArray &aAnswerData);

    /// Получить текущий статус
    virtual bool getStatusInfo(SModem_StatusInfo &aStatusInfo);

    // signals:
    //         void emit_statusSmsSend(bool sts);
private:
    int GetLengthSMS;
    // QString numberPhoneSms;
    QString textToSendSms;

    QString getCommandString(Modem_ProtocolCommands::Enum aCommand);
    bool readPort(QByteArray &tempAnswer);
    void printDataToHex(const QByteArray &data);
    static void msleep(int ms) { QThread::msleep(ms); }
    /// Получение пакета с сформированной командой и её данными.
    ///
    /// Параметры:
    /// - команда (Modem_ProtocolCommands::Enum);
    /// - данные команды;
    /// - сформированный пакет с конкретными байтами команды
    ///
    /// Возвращает:
    /// - успешность выполнения.
    bool getCommandPacket(Modem_ProtocolCommands::Enum aCommand,
                          const QByteArray &aCommandData,
                          QByteArray &aPacketCommand);

    /// Исполнить команду (отослать команду в порт)
    ///
    /// Параметры:
    /// - порт;
    /// - пакет [команда + данные команды];
    ///
    /// Возвращает:
    /// - успешность выполнения.
    bool execCommand(QByteArray &aPacket);

    bool sendCommand(QByteArray dataRequest,
                     bool getResponse,
                     int timeResponse,
                     QByteArray &dataResponse,
                     int timeSleep);
    bool isOpened();

    /// Получить пакет данных из порта
    ///
    /// Параметры:
    /// - порт;
    /// - буфер для полученных данных
    /// - дополнительное количество чтений;
    /// - время между чтениями;
    ///
    /// Возвращает:
    /// - успешность выполнения.
    bool
    getAnswer(QByteArray &aData, bool &codecError, int aAddRepeatCount = 0, int aPauseTime = 0);

    /// Упаковка команды и данных в пакет.
    ///
    /// Параметры:
    /// - буфер для пакета;
    /// - буфер с пакетом команды;
    void packetData(const QByteArray &aCommandPacket, QByteArray &aPacket);

    /// Распаковка пришедших из порта данных.
    ///
    /// Параметры:
    /// - пакет;
    /// - буфер с данными;
    ///
    /// Возвращает:
    /// - успешность выполнения.
    bool unpacketData(const QByteArray &aPacket, QByteArray &aData);

    /// Анализ на ошибку.
    ///
    /// Параметры:
    /// - данные;
    ///
    /// Возвращает:
    /// - успешность выполнения.
    ATErrors::Enum unpacketError(const QByteArray &aPacket);

    /// Препарировать ответ.
    ///
    /// Параметры:
    /// - команда;
    /// - данные;
    ///
    /// Возвращает:
    /// - успешность выполнения.
    bool prepareAnswer(Modem_ProtocolCommands::Enum aCommand, QByteArray &aAnswer);

    /// Состояние модема
    Modem_States::Enum m_state;

    /// Код ошибки состояния модема
    Modem_Errors::Enum m_error;

    int waittimeforans;

    QString encodeUcs2(QString msg);
    QString decodeUcs2(QString hexString);
    QString encodeGSM7bit(QString msg);
    QString decodeGSM7bit(QString hexString);

    QString octet(QString hexString);
};
