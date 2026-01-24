#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtSerialPort/QSerialPortInfo>
#include <QtWebSockets/QWebSocket>
#include <QtWidgets/QMainWindow>
#include <Common/QtHeadersEnd.h>

// System
#include "connection/Connect.h"
#include <db/sqlconnection.h>
#include "devices/ClassDevice.h"
#include "modules/AuthRequest.h"
#include "modules/CheckOnline.h"
#include "modules/CollectDaemons.h"
#include "modules/CommandConfirm.h"
#include "modules/ConfirmOtp.h"
#include "modules/GetBalanceAgent.h"
#include "modules/GetServices.h"
#include "modules/JsonRequest.h"
#include "modules/PayDaemons.h"
#include "modules/SendLogInfo.h"
#include "modules/SendOtp.h"
#include "modules/SendReceipt.h"
#include "modules/StatusDaemons.h"
#include "modules/UserDaemons.h"
#include "other/logClean.h"
#include "other/logger.h"
#include "other/loggerValidator.h"
#include "other/receipt.h"
#include "other/systemInfo.h"
#include "other/utils.h"
#include "ui/admindialog.h"
#include "ui/avtorizationtoadminin.h"
#include "ui/changepassword.h"
#include "ui/incasaciyaform.h"
#include "ui/loadinggprsform.h"
#include "ui/mainpageloader.h"
#include "ui/registrationform.h"
#include "ui/searchdevicesform.h"
#include "updater/update.h"

// Project
#include "main.h"

struct TerminalParams {
    QString login;
    QString token;
    QString secretLogin;
    QString secretPass;
};

// Параметры принтера
struct PrinterParams {
    QString name;
    QString port;
    QString comment;

    bool smallChek = false;
    int chekWidth = 80;
    bool smallBeetwenString;
    int leftMargin = 2;
    int state = 0;
    QString allState;
    int printCheckUntil = 0;
};

struct ValidatorParams {
    QString name;
    QString port;
    QString comment;
    int state;
    bool present = false;
    QString serialNumber;
    QString partNumber;
};

struct CoinAcceptorParams {
    QString name;
    QString port;
    QString comment;
    int state = CCtalkStatus::Errors::NotAvailable;
    bool present = false;
    QString serialNumber;
    QString partNumber;
};

struct ModemParams {
    bool found = false;
    QString name;
    QString port;
    QString comment;
    bool present = false;
    bool simPresent = false;
    QString provider;
    QString rate;
    QString balance;
    QString trafic;
    QString number;
};

struct WDParams {
    bool found = false;
    QString name;
    QString port;
    QString comment;
    bool present = false;
};

struct Config {
    // Пораметры терминала
    TerminalParams terminalData;
    // Пораметры модема
    ModemParams modemData;
    // Параметры купюроприёмника
    ValidatorParams validatorData;
    // Параметры монетоприемника
    CoinAcceptorParams coinAcceptorData;
    // Параметры принтера
    PrinterParams printerData;
    // Параметры Сторожевика
    WDParams WDData;
    // Наименование сервера
    QString serverAddress;
    // Шаблон
    QString tpl = "tjk";
    // Тестовая среда
    bool test = false;
    // Инспект браузера
    bool inspect = false;
    // Ключ для кодирования
    qint32 coddingKey;
    // Проверять баланс сим карты
    bool checkGetBalanceSim = true;
    // Проверять номер сим карты
    bool checkGetNumberSim = true;
    // Индекс для проверки баланса сим карты
    int indexCheckBalance = 0;
    // Запрос на проверку баланса сим
    QString simBalanceRequest = "*100#";
    // Запрос на проверку Номера сим
    QString simNumberRequest = "*99#";
    // Наименование точки дозвона
    QString vpnName;
    // Показать диалог печати чека
    bool showPrintDialog = true;
    // Таймер интервала перезагрузки RasError
    int timerRasReb = 20;
    // Язык по умолчанию
    QString langDefault = Lang::RU;
    // Информация о системе
    QVariantMap systemInfo;
    // Поиск валидатора
    bool searchValidator = true;
    // Поиск монетоприемника
    bool searchCoinAcceptor = true;
    // Поиск принтера
    bool searchPrinter = false;
    // Поиск модема
    bool searchModem = true;
    // Поиск сторожевика
    bool searchWD = false;
    // Ширина Win чека
    int winPrtChekWidth = 90;
    // Высота Win чека
    int winPrtChekHeight = 150;
    // Размер шрифта
    int winPrtChekFontSize = 8;
    // Левый отступ Win чека
    int winPrtChekLeftMargin = 2;
    // Правый отступ Win чека
    int winPrtChekRightMargin = 1;
    // Верхний отступ Win чека
    int winPrtChekTopMargin = 1;
    // Нижний отступ Win чека
    int winPrtChekBottomMargin = 1;
    // Счетчик чека
    bool existCounterChek = true;
    // Включение отключения датчика рулона на принтере
    bool existCounterPrinterChek = false;
    // Длина рулона Счетчик чека
    int counterWidthRulon = 0;
    // Длина чека Счетчик чека
    double counterWidthChek = 0;
    // Оповещать при достижении
    int counterCountCheck = 0;
    // Номер для отправки смс
    QString smsSendNumber;
    // Отправка смс при ошибке купюроприёмника
    bool smsErrValidator = false;
    // Отправка смс при ошибке принтера
    bool smsErrPrinter = false;
    // Отправка смс при малом балансе агента
    bool smsErrBalanceAgent = false;
    // Значение баланса агента при котором надо отправлять смс
    double smsValueBalanceAgent = false;
    // Отправка смс при малом балансе на сим карте
    bool smsErrSimBalance = false;
    // Отправка смс при блокировке терминала
    bool smsErrLockTerminal = false;
    // Отправка смс при ошибке соединения с сервером
    bool smsErrConnection = false;
    // Отправка статуса замятия купюры в боксе
    bool statusValidatorJamInBox = false;
    // Счетчик подсчета замятия купюры в боксе
    int statusValidatorJamInBoxValueCounter = 0;
    // Блокировть ли терминал при замятии в боксе
    bool statusValidatorJamInBoxLockers = false;
    // Блокировка при дубликат купюры
    bool lockDuplicateNominal = false;
    // Включение/отключение автоматического обновления
    bool autoUpdateStatus = true;
};

enum Page { LoadingDevices = 0, LoadingGprs = 1, LoadingMain = 2 };

namespace Action {
    enum Ac {
        /// Снят валидатор купюр
        aOpenValidatorBox = 0,
        /// АСО загружает обновления
        aPoUpdateNow = 1,
        /// АСО запускается
        aPoStartNow = 2,
        /// Обновляется конфигурация
        aPoGetServices = 3
    };
} // namespace Action

namespace CommandInit {
    enum Cmd {
        /// Перезагрузить терминал
        cRebootTerminal = 0,
        /// Выключить терминал
        cTurnOffTerminal = 1,
        /// Обновить ПО
        cUpdatePO = 2,
        /// Включить Автообновление
        cTurnOnAutoApdate = 3,
        /// Выключить Автообновление
        cTurnOffAutoApdate = 4,
        /// Загрузить Лог
        cGetLogTerminal = 5,
        /// Перезагрузить купюроприемник
        cRestartValidator = 6,
        /// Отправить лог на сервер
        cSendLogInfo = 7,
        /// Удаленная инкассация
        cGetIncashment = 8,
        /// Отправить лог валидатора
        cSendLogValidator = 9,
        /// Обновление прошивки купюроприемника
        cBVFirmwareUpdate = 10
    };
} // namespace CommandInit

namespace SmsSend {
    enum smsState { NotSend = 0, OK = 1 };

    enum lockState { Unlock = 0, Lock = 1 };
} // namespace SmsSend

struct LockItem {
    bool lock;
    QString comment;
};

namespace Lock {
    enum Data {
        // Терминал работает
        Ok = 0,
        // Терминал заблокирован из за недостатка средств
        NonMoney = 1,
        // Терминал заблокирован из за ошибки валидатора
        ErrorValidator = 2,
        // Терминал заблокирован из за проблемы с интерфейсом
        ErrorInterface = 3,
        // Терминал заблокирован по сигналу с сервера
        LockFromServer = 4,
        // Терминал заблокирован по ошибке определения устройств
        ErrorDevice = 5,
        // Терминал заблокирован по параметру is_active
        IsActiveLock = 6,
        // Терминал заблокирован по статусу Терминал не активен(11)
        Status_11 = 7,
        // Терминал заблокирован по статусу Доступ запрещен (Агент не активен (12))
        Status_12 = 8,
        // Терминал заблокирован по статусу Ошибка авторизации(Не верный номер
        // терминала, логин, пароль (150))
        ErrorAvtorizat = 9,
        // Терминал заблокирован по статусу Не верный тип терминала(245)
        ErrorTypeTrm = 10,
        // Терминал заблокирован по статусу Нет прав на прим платежей(133)
        ErrorNoRoulPay = 11,
        // Терминал заблокирован по причине в системе более 5-и платежей
        MorePayIn = 12,
        // Пользователь(Персона) заблокирована(14)
        ErrorTypeUser = 13,
        // Терминал заблокирован по причине открыта админка
        ErrorOpenAdminP = 14,
        ErrorDublicateNominal = 15,
        ErrorDatabase = 16
    };
} // namespace Lock

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool testMode;
    bool hasProcess(const QString &process);

    void init();

  private:
    Ui::MainWindow *ui;

    Logger *logger;
    LoggerValidator *loggerValidator;
    LogClean *logClean;
    SystemInfo *systemInfo;
    ConnectionPart *connObject;

    // Devices
    ClassPrinter *clsPrinter;
    ClassValidator *clsValidator;
    ClassAcceptor *clsCoinAcceptor;
    ClassModem *clsModem;
    WatchDogs *watchDogs;
    SearchDevices *searchDevices;

    // UI
    RegistrationForm *registrationForm;
    AdminDialog *adminDialog;
    SearchDevicesForm *sDevicesForm;
    LoadingGprsForm *loadingGprs;
    MainPageLoader *mainPage;
    IncasaciyaForm *incasaciyaForm;
    AvtorizationToAdminIn *adminAuthDialog;
    ChangePassword *changePassword;

    // Modules
    AuthRequest *authRequest;
    GetServices *getServices;
    PayDaemons *payDaemons;
    StatusDaemons *statusDaemons;
    CollectDaemons *collectDaemons;
    UserDaemons *userDaemons;
    CheckOnline *checkOnline;
    SendReceipt *sendReceipt;
    SendOtp *sendOtp;
    ConfirmOtp *confirmOtp;
    CommandConfirm *commandConfirm;
    SendLogInfo *sendLogInfo;
    GetBalanceAgent *getBalanceAgent;
    JsonRequest *jsonRequest;

    // Timers
    QTimer *statusValidatorTimer;
    QTimer *statusCoinAcceptorTimer;
    QTimer *lockerTimer;
    QTimer cmdTimer;
    QTimer cmdExecTimer;
    QTimer wsReconnectTimer;
    QTimer wsStateTimer;
    QTimer servicesTimer;

    // WS
    QWebSocket webSocket;

    // Updater
    DownloadManager *downManager;

  private:
    QString lang;
    QString senderName;
    QString versionFull();

    // Database
    QSqlDatabase db;
    QSqlDatabase dbUpdater;

    // Config / settings
    Config config;
    QString settingsPath();
    void settingsSave();
    void settingsGet();
    void settingsSet(const QString key, const QVariant value);

    // Browser settings
    void loadWebSettings();

    bool getPrinterState;

    QString textSms;

    QMap<Action::Ac, bool> actionList;
    QStringList lstCommandInfo;
    QStringList printerStatus;
    QStringList billValidatorList;
    QStringList coinAcceptorList;
    QStringList printerList;

    QStringList portList();

    QMap<Lock::Data, LockItem> lockList;
    void setLockList();
    Lock::Data getLock();

    QMap<QString, QString> serverAddress;

    QString interfaceText(QString key);

    CommandInit::Cmd cmdExec;
    QVariantMap cmdMeta;
    int cmdTryCount = 0;

    QVariantList cmdList;

    QVariantMap terminalInfo;
    QVariantMap nominalData();

    QString printText;

    bool oneGetServices = true;
    bool oneSendStatus = true;
    bool oneLoadingGprs = true;
    bool systemInfoGot = false;

    bool whenRasReboot;
    int countGS = 0;
    int countOtherErrRas = 0;
    int countRepEntires = 0;
    int countCmd = 0;

    bool afterRestartRas = false;
    int rebootCount();

    int terminalSmsCount();
    void insertSmsContentInf();

    bool clearDataBase();
    bool deleteTerminalData();
    bool getDeviceFromDB();

    QString getWinprinterFromDB();

    bool commandExist(QString trn);
    bool saveCommand(const QString trn, const int cmd, const QString account, const QString comment);
    bool getActiveCommand(QString &trn, int &cmd, QString &account, QString &comment, QString &status);
    bool commandStatusUpdate(QString trn, QString status);

    bool saveBillValidatorEvent(const QString event, const QString dateTime);
    bool getBillValidatorEvent(QString &dateTime, QString event, QString status);
    bool updateBillValidatorEvent(QString status);

    void applyAuthToModules();

    QString printerStatusList();

    void deleteSearchParam();

    QString systemHash;
    QString systemHashGet();

    QStringList getWinPrinterNames();

    bool isModemConnectionUp(QString &connectionName);

  protected:
    void closeEvent(QCloseEvent *event);

  private slots:
    void loadStyleSheet();
    qint32 generateEncodeKey();

    // Connection
    void connectionError();
    void connectionError(QString errNum, QString errComment);
    void connectionState(QString res, QString comment);
    void connectionResult(bool result);
    void connectionUpState();
    void connectionCheck();
    void startToConnection();
    void startToConnect(QString connection);

    // Auth request
    void sendAuthRequest(QString login, QString otp);
    void authResponse(QString resultCode, QString login, QString token, QString message);

    // Get services
    void getServicesRequest();
    void getServicesReturn(bool result);
    void getServicesError();

    void getTerminalInfo(QVariantMap map);
    void setTerminalInfo(QString data);

    // Проверяем пользователя в базе
    bool checkUserInBase();
    void checkConfigData(bool skipSearchDevice = false);
    void getRegistrationData(QVariantMap data);
    void createDialUpConnection(QVariantMap data);

    void lockUnlockCenter(Lock::Data state, bool lock);
    void openAdminDialog();
    void getCommandFromAdmin(AdminCommand::AdminCmd cmd);
    void unlockAdminOpenSts();
    void checkLockTerminal();
    bool saveTerminalAuthData(bool update, QString login, QString token);
    bool saveAdminAuthData(bool update, QString secretLogin, QString secretPassword);
    void editAdminAuthData(QString login, QString password);
    void openAdminAuthEditDialog();
    void openAdminAuthDialog();

    bool saveLockDuplicateNominal(bool lock);

    void tplSelected(QString tpl, bool test);

    void getBalanceUser(double balance, double overdraft, double threshold);
    void getBalanceAgentData(QString balance, QString overdraft);
    void isActiveLock(bool active);
    void avtorizationLockUnlock(bool lock, int sts);

    void getBanners(QVariantList banners);

    void nonSendPaymentLock(bool lock);
    void errorDBLock();

    void rebootCountSet(int val = 0);
    void rebootCountClear();

    void rebootModemEntries();
    void rebootEntries(int errInit);

    void getSystemInfo(QVariantMap data);

    // SMS
    void createSmsSendTable();
    void getSmsSendStatus(bool state, QStringList lstId);

    // Devices
    void deviceSearchStart(QVariantMap data);
    void deviceSearchResult(int device, int result, QString dev_name, QString dev_comment, QString dev_port);
    void deviceSearchFinished();
    void deviceTest(int device, QString name, QString port, QString comment);
    void saveDevice(int deviceId, QString deviceName, QString port, QString comment, int state);
    void devicesInitialization();

    // Printer
    void toPrintText(QString text);
    void statusPrinter(int status);

    // Bill acceptor
    void validatorStatusGet();
    void validatorStatusGetOnMain();
    void nominalGet(int nominal);
    void nominalDuplicateGet(int nominal);
    void incameStatusFromValidator(int sts, QString comment);
    void validatorInit(bool action);
    void openValidatorBox();
    void getCommandFromIncash(int cmd);
    void validatorFirmwareResult(QString state);

    // Coin acceptor
    void coinAcceptorStatusGet();
    void coinAcceptorStatusGetOnMain();
    void coinGet(int coin);
    void coinDuplicateGet(int coin);
    void incameStatusFromCoinAcceptor(int sts, QString comment);
    void coinAcceptorInit(bool action);

    // Watchdog
    void cmdWatchdogDone(bool state, int aCommand);

    // Cmd
    void getCommandFromServer(QVariantList cmdList);
    void commandConfirmed(QString trn);
    void commandExecution(CommandInit::Cmd cmd, QVariantMap meta = QVariantMap());
    void restartTerminalInit();
    void commandCheck();
    void commandExecute();

    // Bill validator events
    void bValidatorEventCheck();

    // Hash
    void checkHash(QString hash);
    void checkUpdateHash(QString hash, QString path);

    // Websocket
    void wsConnectionOpen();
    void wsConnected();
    void wsQuery(const QString query, const QVariantMap data = QVariantMap());
    void wsStateChanged(QAbstractSocket::SocketState state);
    void wsStateQuery();

    // Json request
    void jsonResponseError(QString error, QString requestName);
    void jsonResponseSuccess(QVariantMap response, QString requestName);

    void gotoPage(Page page);
    void getDataToSendStatus();
    void toSendMonitoringStatus();
    void killSheller();

    void filesUpdated(QVariantMap files);
    void updaterLock(bool lock);

    QVariantList getServicesFromDB();
    QVariantList getCategoriesFromDB();
    QVariantList getServicesInputsFromDB();
    QVariantList getServicesFieldsFromDB();

    bool updateHashConfig(QString hash);

    void toLog(int state, QString title, QString text);
    void toValidatorLog(int state, QByteArray data, QString text);
    void closeApp();

  signals:
    void emitStatusAnimateSum(bool status);
    void emitStatusReturnNominal(bool status);
};
