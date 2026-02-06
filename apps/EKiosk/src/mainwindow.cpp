#include "mainwindow.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    lang = Lang::RU;
    senderName = "MAIN";

    // Логирование данных
    logger = new Logger();
    loggerValidator = new LoggerValidator();

    billValidatorList << ValidatorModel::CashCodeCCNET << ValidatorModel::MeiEBDS;
    coinAcceptorList << AcceptorModel::CCTalk;
    printerList << PrinterModel::Custom_VKP80 << PrinterModel::CitizenCBM1000
                << PrinterModel::Citizen_CTS2000 << PrinterModel::Custom_TG2480
                << PrinterModel::Citizen_PPU700 << PrinterModel::AV_268
                << PrinterModel::Phoenix_model << PrinterModel::KM1X
                << PrinterModel::Windows_Printer;

    // Соединение с базой данных
    createConnection(db, QString("%1%2").arg(ConstData::Path::Config, ConstData::FileName::DBName));

    createConnectionFile(dbUpdater,
                         QString("%1%2").arg(ConstData::Path::Config, ConstData::FileName::DBFile));

    auto logData = QString("EKiosk VERSION - %1\n").arg(ConstData::version);

    toLog(LoggerLevel::Info, senderName, logData);

    setWindowTitle(versionFull());

    // Ключ для кодирования декодирования данных
    config.coddingKey = generateEncodeKey();

    // Серверы
    serverAddress["tjk_prod"] = "https://aso.humo.tj/aso/v3/";
    serverAddress["tjk_test"] = "https://aso-test.humo.tj/aso/v3/";

    serverAddress["uzb_prod"] = "https://api.aso.orzu.tech/";
    serverAddress["uzb_test"] = "http://192.168.145.28/aso/api/";

    // Загружаем стиль интерфейса
    loadStyleSheet();

    // Настройки браузера
    loadWebSettings();

    registrationForm = nullptr;
    loadingGprs = nullptr;
}

MainWindow::~MainWindow() {
    delete ui;
}

QString MainWindow::settingsPath() {
    return qApp->applicationDirPath() + "/" + ConstData::Path::Config +
           ConstData::FileName::Settings;
}

void MainWindow::init() {

    adminDialog = new AdminDialog(this);
    connect(adminDialog,
            SIGNAL(emit_execToMain(AdminCommand::AdminCmd)),
            this,
            SLOT(getCommandFromAdmin(AdminCommand::AdminCmd)));
    connect(adminDialog, SIGNAL(emit_unlockOpenAdminSts()), SLOT(unlockAdminOpenSts()));

    authRequest = new AuthRequest();
    connect(authRequest,
            SIGNAL(emitResult(QString, QString, QString, QString)),
            SLOT(authResponse(QString, QString, QString, QString)));

    systemInfo = new SystemInfo();
    connect(
        systemInfo, SIGNAL(emitSystemInfo(QVariantMap)), this, SLOT(getSystemInfo(QVariantMap)));

    // Отключаем проводник
    if (!testMode) {
        getCommandFromAdmin(AdminCommand::aCmdHideExplorer);
    }

    connObject = new ConnectionPart(this);

    connect(connObject, SIGNAL(emit_ConnectionError()), this, SLOT(connectionError()));
    connect(connObject, SIGNAL(emit_Ping(bool)), this, SLOT(connectionResult(bool)));
    connect(connObject,
            SIGNAL(emit_errorState(QString, QString)),
            this,
            SLOT(connectionError(QString, QString)));
    connect(connObject,
            SIGNAL(emit_connState(QString, QString)),
            this,
            SLOT(connectionState(QString, QString)));
    connect(connObject, SIGNAL(emit_ConnectionUp()), this, SLOT(connectionUpState()));
    connect(connObject, SIGNAL(emit_checkConState()), this, SLOT(connectionCheck()));
    connect(connObject,
            SIGNAL(emit_toLoging(int, QString, QString)),
            SLOT(toLog(int, QString, QString)));

    // Демон для работы с инкасацией
    collectDaemons = new CollectDaemons(this);
    collectDaemons->setDbConnect(db);
    connect(collectDaemons,
            SIGNAL(emit_Loging(int, QString, QString)),
            this,
            SLOT(toLog(int, QString, QString)));

    // Объект поиска устпойств
    searchDevices = new SearchDevices(this);
    connect(searchDevices,
            SIGNAL(emitDeviceSearch(int, int, QString, QString, QString)),
            SLOT(deviceSearchResult(int, int, QString, QString, QString)));
    connect(searchDevices, SIGNAL(emitDeviceSearchFinished()), SLOT(deviceSearchFinished()));

    // Присваиваем базу
    searchDevices->setDbName(db);

    searchDevices->setComListInfo(portList());
    searchDevices->takeBalanceSim = config.checkGetBalanceSim;
    searchDevices->takeSimNumber = config.checkGetNumberSim;

    searchDevices->s_ussdRequestBalanseSim = config.simBalanceRequest;
    searchDevices->s_ussdRequestNumberSim = config.simNumberRequest;
    searchDevices->s_indexBalanceParse = config.indexCheckBalance;

    // Тут надо проверять пользователя в базе
    if (checkUserInBase()) {
        checkConfigData();
    } else {
        // Показываем регистрацию
        registrationForm = new RegistrationForm();
        connect(registrationForm,
                SIGNAL(emitStartSearchDevices(QVariantMap)),
                SLOT(deviceSearchStart(QVariantMap)));
        connect(registrationForm,
                SIGNAL(emitDeviceTest(int, QString, QString, QString)),
                SLOT(deviceTest(int, QString, QString, QString)));
        connect(registrationForm,
                SIGNAL(emitCreateNewConnection(QVariantMap)),
                SLOT(createDialUpConnection(QVariantMap)));
        connect(registrationForm, SIGNAL(emitTpl(QString, bool)), SLOT(tplSelected(QString, bool)));
        connect(registrationForm,
                SIGNAL(emitSaveDevice(int, QString, QString, QString, int)),
                SLOT(saveDevice(int, QString, QString, QString, int)));
        connect(
            registrationForm, SIGNAL(emitStartToConnect(QString)), SLOT(startToConnect(QString)));
        connect(registrationForm,
                SIGNAL(emitSendAuthRequest(QString, QString)),
                SLOT(sendAuthRequest(QString, QString)));
        connect(registrationForm,
                SIGNAL(emitRegistrationData(QVariantMap)),
                this,
                SLOT(getRegistrationData(QVariantMap)));
        connect(registrationForm,
                SIGNAL(emitToLog(int, QString, QString)),
                SLOT(toLog(int, QString, QString)));

        collectDaemons->firstCollection = true;

        // Вставляем список соединений
        QStringList connectionList;
        connectionList << QString("Local connection");

        auto rasConnectionList = connObject->getRasConnectionList();
        if (rasConnectionList.count() > 0) {
            connectionList.append(rasConnectionList);
        }

        //        QString connectionName;

        registrationForm->setDB(db);

        QVariantMap data;
        //        data["check_balance_sim"] = config.checkGetBalanceSim;
        //        data["check_number_sim"] = config.checkGetNumberSim;
        //        data["ussd_balance_sim"] = config.simBalanceRequest;
        //        data["ussd_number_sim"] = config.simNumberRequest;
        //        data["index_check_balance"] = config.indexCheckBalance;
        //        data["modem_connection_up"] = isModemConnectionUp(connectionName);

        data["search_validator"] = config.searchValidator;
        data["search_coin_acceptor"] = config.searchCoinAcceptor;
        data["search_printer"] = config.searchPrinter;
        data["search_modem"] = config.searchModem;
        data["search_watchdog"] = config.searchWD;

        registrationForm->setSearchDeviceParams(data);

        registrationForm->validatorList = billValidatorList;
        registrationForm->coinAcceptorList = coinAcceptorList;
        registrationForm->printerList = printerList;
        registrationForm->winprinterList = getWinPrinterNames();
        registrationForm->comPortList = portList();

        registrationForm->setDataListConnection(connectionList);

        // Вставляем устройства
        QStringList lstDevDialup;

        bool dialDev = connObject->hasInstalledModems(lstDevDialup);

        if (!dialDev) {
            toLog(LoggerLevel::Error, "CONNECTION", "На терминале нет установленных модемов...");
        }

        registrationForm->dialupDevice = lstDevDialup;

        registrationForm->move(x() + (width() - registrationForm->width()) / 2,
                               y() + (height() - registrationForm->height()) / 2);
        registrationForm->showMe();
    }

    // Берем информацию о системе
    toLog(LoggerLevel::Info, senderName, "GET SYSINFO START");

    systemInfo->start();
}

void MainWindow::checkConfigData(bool skipSearchDevice) {

    getPrinterState = false;

    // Комманды на выполнения действий
    lstCommandInfo << "Перезагрузить терминал"
                   << "Выключить терминал"
                   << "Обновить ПО"
                   << "Включить Автообновление"
                   << "Выключить Автообновление"
                   << "Загрузить Лог"
                   << "Перезагрузить купюроприемник"
                   << "Загрузить Лог на сервер"
                   << "Отложенная инкасация";

    //*************************************************
    // Снят валидатор купюр
    actionList[Action::aOpenValidatorBox] = false;
    // АСО загружает обновления
    actionList[Action::aPoUpdateNow] = false;
    // АСО запускается
    actionList[Action::aPoStartNow] = true;
    // Обновляется конфигурация
    actionList[Action::aPoGetServices] = false;
    //*************************************************

    // Статус принтера
    printerStatus << "Активен."
                  << "Кончилась бумага."
                  << "Заканчивается бумага."
                  << "Замятие бумаги."
                  << "Не известная команда."
                  << "Ошибка питания на принтер."
                  << "Ошибка печатающей головки."
                  << "Ошибка порта."
                  << "Ошибка принтера."
                  << "Ошибка температуры."
                  << "Открыта крышка принтера."
                  << "Ошибка фискальной памяти."
                  << "Ошибка позиции механима."
                  << "Ошибка отрезчика."
                  << "Сбой в электроника."
                  << "Заканчивается фискальная память."
                  << "Принтер не доступен."
                  << "Нет статуса."
                  << "Не известная ошибка.";

    // Присваиваем сеттингс структуре
    settingsGet();

    auto key = QString("%1_%2").arg(config.tpl, config.test ? "test" : "prod");

    config.serverAddress = serverAddress[key];
    authRequest->setUrl(config.serverAddress + "auth");

    QStringList langList;

    langList << "local" << "ru" << "en";

    adminDialog->setLangList(langList);

    QString logoPath = config.tpl == "tjk" ? "assets/images/logo.png" : "";

    // Принтер
    clsPrinter = new ClassPrinter(this);
    connect(clsPrinter, SIGNAL(emit_status(int)), SLOT(statusPrinter(int)));

    // Валидатор
    clsValidator = new ClassValidator(this);
    connect(clsValidator, SIGNAL(eNominal(int)), SLOT(nominalGet(int)));
    connect(clsValidator, SIGNAL(eNominalDuplicate(int)), SLOT(nominalDuplicateGet(int)));
    connect(clsValidator, SIGNAL(showHideDialogAnimate(bool)), SIGNAL(emitStatusAnimateSum(bool)));
    connect(clsValidator,
            SIGNAL(showHideDialogReturnNominal(bool)),
            SIGNAL(emitStatusReturnNominal(bool)));
    connect(clsValidator,
            SIGNAL(emitStatusValidator(int, QString)),
            SLOT(incameStatusFromValidator(int, QString)));
    connect(
        clsValidator, SIGNAL(emitLog(int, QString, QString)), SLOT(toLog(int, QString, QString)));
    connect(clsValidator,
            SIGNAL(emitValidatorLog(int, QByteArray, QString)),
            SLOT(toValidatorLog(int, QByteArray, QString)));
    connect(
        clsValidator, SIGNAL(emitFirmwareUpdate(QString)), SLOT(validatorFirmwareResult(QString)));

    // Монетоприемник
    clsCoinAcceptor = new ClassAcceptor(this);
    connect(clsCoinAcceptor, SIGNAL(eNominal(int)), SLOT(coinGet(int)));
    connect(clsCoinAcceptor, SIGNAL(eNominalDuplicate(int)), SLOT(coinDuplicateGet(int)));
    connect(
        clsCoinAcceptor, SIGNAL(showHideDialogAnimate(bool)), SIGNAL(emitStatusAnimateSum(bool)));
    connect(clsCoinAcceptor,
            SIGNAL(emitStatusCoinAcceptor(int, QString)),
            SLOT(incameStatusFromCoinAcceptor(int, QString)));
    connect(clsCoinAcceptor,
            SIGNAL(emitLoging(int, QString, QString)),
            SLOT(toLog(int, QString, QString)));

    // Модем
    clsModem = new ClassModem(this);
    connect(clsModem,
            SIGNAL(emit_statusSmsSend(bool, QStringList)),
            SLOT(getSmsSendStatus(bool, QStringList)));
    textSms = "";

    watchDogs = new WatchDogs(this);
    connect(watchDogs, SIGNAL(commandDone(bool, int)), SLOT(cmdWatchdogDone(bool, int)));

    // Интерфейс Поиска устройств
    sDevicesForm = new SearchDevicesForm(this);

    ui->searchDevicesLayout->addStretch();
    ui->searchDevicesLayout->addWidget(sDevicesForm);
    ui->searchDevicesLayout->addStretch();

    sDevicesForm->setLogo(logoPath);

    // Интерфейс подключения к сети
    loadingGprs = new LoadingGprsForm(this);
    loadingGprs->setLogo(logoPath);

    ui->loadingGprsLayout->addStretch();
    ui->loadingGprsLayout->addWidget(loadingGprs);
    ui->loadingGprsLayout->addStretch();

    QString info = QString("v %1").arg(ConstData::version);

    loadingGprs->setCopirightText(QString("%1").arg(info), config.terminalData.login);

    // В случае если соединения нет то подымаем соединение
    connObject->setEndpoint(5, config.serverAddress + "ping");
    connObject->setConnectionConfig(config.vpnName);

    //    config.checkGetBalanceSim = true;
    //    config.checkGetNumberSim  = true;

    sDevicesForm->setCopirightText(QString("%1").arg(info), "№ " + config.terminalData.login);
    //    sDevicesForm->setValidatorSearchText(0,
    //    interfaceText("validator_info_searching"));

    // Объект для получения конфигурации
    getServices = new GetServices(this);
    getServices->setDbConnect(db);

    connect(
        getServices, SIGNAL(emit_infoData(QVariantMap)), this, SLOT(getTerminalInfo(QVariantMap)));
    connect(getServices, SIGNAL(emit_getServices(bool)), this, SLOT(getServicesReturn(bool)));
    connect(getServices,
            SIGNAL(emit_responseBalance(double, double, double)),
            this,
            SLOT(getBalanceUser(double, double, double)));
    connect(getServices, SIGNAL(emit_responseIsActive(bool)), this, SLOT(isActiveLock(bool)));
    connect(getServices,
            SIGNAL(lockUnlockAvtorization(bool, int)),
            this,
            SLOT(avtorizationLockUnlock(bool, int)));
    connect(getServices, SIGNAL(emit_ErrResponse()), this, SLOT(getServicesError()));
    connect(
        getServices, SIGNAL(emit_timeServer(QString)), connObject, SLOT(setDateTimeIn(QString)));
    connect(getServices, SIGNAL(emit_banners(QVariantList)), this, SLOT(getBanners(QVariantList)));
    connect(getServices,
            SIGNAL(emit_Loging(int, QString, QString)),
            this,
            SLOT(toLog(int, QString, QString)));

    getServices->setUrl(config.serverAddress);

    // Параметры блокировки
    setLockList();

    mainPage = new MainPageLoader(ui->p_mainContent);

    mainPage->setDbName(db);
    mainPage->langDefaultSet(config.langDefault);
    ui->p_mainContentLayout->addWidget(mainPage);

    connect(mainPage, SIGNAL(emit_sendStatusValidator()), this, SLOT(validatorStatusGet()));
    connect(mainPage, SIGNAL(validator_activate(bool)), this, SLOT(validatorInit(bool)));
    connect(mainPage, SIGNAL(validator_activate(bool)), this, SLOT(coinAcceptorInit(bool)));
    connect(
        mainPage, SIGNAL(emit_toLoging(int, QString, QString)), SLOT(toLog(int, QString, QString)));
    connect(mainPage, SIGNAL(emit_openAvtorizationDialog()), SLOT(openAdminAuthDialog()));
    connect(mainPage, SIGNAL(emit_updaterLock(bool)), SLOT(updaterLock(bool)));
    connect(
        this, SIGNAL(emitStatusReturnNominal(bool)), mainPage, SLOT(showHideReturnNominal(bool)));

    // Показать диалог печати
    mainPage->printDialogShow = config.showPrintDialog;

    // Шаблон
    mainPage->setTemplate(config.tpl);

    // Инспект браузера
    if (config.inspect) {
        mainPage->inspectEnable();
        qApp->setOverrideCursor(Qt::ArrowCursor);
    }

    // Статус принтера
    mainPage->printerStatus = true;

    // Платежный демон
    payDaemons = new PayDaemons(this);
    payDaemons->setDbConnect(db);
    payDaemons->setUrl(config.serverAddress);
    payDaemons->tpl = config.tpl;
    connect(payDaemons,
            SIGNAL(emit_Loging(int, QString, QString)),
            this,
            SLOT(toLog(int, QString, QString)));
    connect(payDaemons, SIGNAL(emit_to_print(QString)), SLOT(toPrintText(QString)));
    connect(payDaemons,
            SIGNAL(emit_responseBalance(double, double, double)),
            this,
            SLOT(getBalanceUser(double, double, double)));
    connect(payDaemons, SIGNAL(lockUnlockNonSend(bool)), this, SLOT(nonSendPaymentLock(bool)));
    connect(payDaemons, SIGNAL(emit_RestartNet()), SLOT(startToConnection()));
    connect(payDaemons, SIGNAL(emit_RestartTerminal()), SLOT(restartTerminalInit()));
    connect(payDaemons, SIGNAL(emit_errorDB()), SLOT(errorDBLock()));

    connect(
        mainPage, SIGNAL(emit_pay_new(QVariantMap)), payDaemons, SLOT(get_new_pay(QVariantMap)));
    connect(mainPage,
            SIGNAL(emit_update_pay(QVariantMap)),
            payDaemons,
            SLOT(get_update_pay(QVariantMap)));
    connect(mainPage,
            SIGNAL(emit_confirm_pay(QString, bool)),
            payDaemons,
            SLOT(get_confirm_pay(QString, bool)));
    connect(mainPage, SIGNAL(emit_print_pay(QString)), payDaemons, SLOT(get_print_id(QString)));
    connect(mainPage, SIGNAL(emit_checkStatus54()), payDaemons, SLOT(checkPayStatus54()));

    // Статус демон
    statusDaemons = new StatusDaemons(this);
    statusDaemons->setDbConnect(db);

    statusDaemons->setUrl(config.serverAddress);
    connect(statusDaemons, SIGNAL(getRequestParam()), this, SLOT(getDataToSendStatus()));
    connect(statusDaemons,
            SIGNAL(emit_Loging(int, QString, QString)),
            this,
            SLOT(toLog(int, QString, QString)));
    connect(statusDaemons,
            SIGNAL(emit_responseBalance(double, double, double)),
            this,
            SLOT(getBalanceUser(double, double, double)));
    connect(statusDaemons, SIGNAL(emit_responseIsActive(bool)), this, SLOT(isActiveLock(bool)));
    connect(statusDaemons,
            SIGNAL(lockUnlockAvtorization(bool, int)),
            this,
            SLOT(avtorizationLockUnlock(bool, int)));
    connect(statusDaemons,
            SIGNAL(emit_cmdToMain(QVariantList)),
            this,
            SLOT(getCommandFromServer(QVariantList)));
    connect(statusDaemons, SIGNAL(emit_hashToCheck(QString)), this, SLOT(checkHash(QString)));
    connect(statusDaemons,
            SIGNAL(emit_hashUpdateToCheck(QString, QString)),
            this,
            SLOT(checkUpdateHash(QString, QString)));

    collectDaemons->setUrl(config.serverAddress);
    collectDaemons->setNominalData(nominalData());

    // Демон для работы с данными пользователя
    userDaemons = new UserDaemons(this);
    //    userDaemons->setParent(this);
    userDaemons->setUrl(config.serverAddress);
    connect(userDaemons, SIGNAL(emit_UserData(QString)), this, SLOT(setTerminalInfo(QString)));
    connect(userDaemons,
            SIGNAL(emit_Loging(int, QString, QString)),
            SLOT(toLog(int, QString, QString)));

    connect(mainPage,
            SIGNAL(emit_getUserInfo(QString, QString)),
            userDaemons,
            SLOT(sendUserDataRequest(QString, QString)));

    // Демон для онлайн проверки аккаунта банка
    checkOnline = new CheckOnline(this);
    checkOnline->setUrl(config.serverAddress);
    connect(checkOnline,
            SIGNAL(emit_CheckOnlineResult(QString, QString, QString, QVariantList)),
            mainPage,
            SLOT(setCheckOnlineResult(QString, QString, QString, QVariantList)));
    connect(checkOnline,
            SIGNAL(emit_Loging(int, QString, QString)),
            SLOT(toLog(int, QString, QString)));

    connect(mainPage,
            SIGNAL(emit_checkOnline(QString, QString, QString, double, QVariantMap)),
            checkOnline,
            SLOT(sendCheckOnlineRequest(QString, QString, QString, double, QVariantMap)));

    // Демон для отправки эл.чека
    sendReceipt = new SendReceipt(this);
    sendReceipt->setUrl(config.serverAddress);
    connect(sendReceipt,
            SIGNAL(emitSendReceiptResult(QString, QString, QString)),
            mainPage,
            SLOT(sendReceiptResult(QString, QString, QString)));
    connect(sendReceipt,
            SIGNAL(emit_Loging(int, QString, QString)),
            SLOT(toLog(int, QString, QString)));

    connect(mainPage,
            SIGNAL(emit_sendReceipt(QString, QString)),
            sendReceipt,
            SLOT(sendReceiptRequest(QString, QString)));

    // Демон для отправки смс кода (otp)
    sendOtp = new SendOtp(this);
    sendOtp->setUrl(config.serverAddress);
    connect(sendOtp,
            SIGNAL(emit_SendOtpResult(QString, QString)),
            mainPage,
            SLOT(sendOtpResult(QString, QString)));
    connect(mainPage, SIGNAL(emit_otpSend(QString)), sendOtp, SLOT(sendOtpRequest(QString)));

    // Демон для подтверждения смс кода (otp)
    confirmOtp = new ConfirmOtp(this);
    confirmOtp->setUrl(config.serverAddress);
    connect(confirmOtp,
            SIGNAL(emit_ConfirmOtpResult(QString)),
            mainPage,
            SLOT(confirmOtpResult(QString)));
    connect(mainPage,
            SIGNAL(emit_otpConfirm(QString, QString)),
            confirmOtp,
            SLOT(confirmOtpRequest(QString, QString)));

    // Демон для отправки команды
    commandConfirm = new CommandConfirm(this);
    commandConfirm->setUrl(config.serverAddress);
    connect(
        commandConfirm, SIGNAL(emit_cmdConfirmed(QString)), this, SLOT(commandConfirmed(QString)));

    // Демон для отправки лога
    sendLogInfo = new SendLogInfo(this);
    sendLogInfo->setUrl(config.serverAddress);
    connect(sendLogInfo,
            SIGNAL(emit_Loging(int, QString, QString)),
            this,
            SLOT(toLog(int, QString, QString)));

    getBalanceAgent = new GetBalanceAgent(this);
    getBalanceAgent->setUrl(config.serverAddress);
    connect(getBalanceAgent,
            SIGNAL(emit_BalanceAgent(QString, QString)),
            this,
            SLOT(getBalanceAgentData(QString, QString)));

    jsonRequest = new JsonRequest(this);
    jsonRequest->setBaseUrl(config.serverAddress);
    connect(jsonRequest,
            SIGNAL(emitResponseSuccess(QVariantMap, QString)),
            this,
            SLOT(jsonResponseSuccess(QVariantMap, QString)));
    connect(jsonRequest,
            SIGNAL(emitResponseError(QString, QString)),
            this,
            SLOT(jsonResponseError(QString, QString)));

    connect(mainPage,
            SIGNAL(emit_sendJsonRequest(QJsonObject, QString, QString, int, int, QVariantMap)),
            jsonRequest,
            SLOT(sendRequest(QJsonObject, QString, QString, int, int, QVariantMap)));

    logClean = new LogClean();

    // Websocket
    wsReconnectTimer.setSingleShot(true);
    wsReconnectTimer.setInterval(10000);
    connect(&wsReconnectTimer, &QTimer::timeout, this, &MainWindow::wsConnectionOpen);

    wsStateTimer.setInterval(wsStateInterval * 1000);
    connect(&wsStateTimer, &QTimer::timeout, this, &MainWindow::wsStateQuery);

    connect(&webSocket, &QWebSocket::connected, this, &MainWindow::wsConnected);
    connect(&webSocket, &QWebSocket::stateChanged, this, &MainWindow::wsStateChanged);

    // Программа впереди всех програм
    if (!testMode) {
        setWindowFlags(Qt::WindowStaysOnTopHint);
    }

    // Показываем на весь экран
    showFullScreen();

    statusValidatorTimer = new QTimer(this);
    connect(statusValidatorTimer, SIGNAL(timeout()), this, SLOT(validatorStatusGetOnMain()));

    statusCoinAcceptorTimer = new QTimer(this);
    connect(statusCoinAcceptorTimer, SIGNAL(timeout()), this, SLOT(coinAcceptorStatusGetOnMain()));

    incasaciyaForm = new IncasaciyaForm(this);
    connect(incasaciyaForm, SIGNAL(execCommand(int)), SLOT(getCommandFromIncash(int)));
    connect(incasaciyaForm, SIGNAL(openDialog()), SLOT(openAdminAuthEditDialog()));

    // Таймер для проверок блокировок
    lockerTimer = new QTimer(this);
    connect(lockerTimer, SIGNAL(timeout()), SLOT(checkLockTerminal()));

    connect(&cmdTimer, &QTimer::timeout, this, &MainWindow::commandCheck);
    cmdTimer.setInterval(5000);

    connect(&cmdExecTimer, &QTimer::timeout, this, &MainWindow::commandExecute);
    cmdExecTimer.setInterval(10000);

    rebootCountSet();
    QTimer::singleShot(config.timerRasReb * 60000, this, SLOT(rebootCountClear()));

    QString IpServerName = config.serverAddress + "update";

    // updater
    downManager = new DownloadManager();
    connect(downManager,
            SIGNAL(emit_Loging(int, QString, QString)),
            SLOT(toLog(int, QString, QString)));
    connect(
        downManager, SIGNAL(emit_FilesUpdated(QVariantMap)), this, SLOT(filesUpdated(QVariantMap)));
    connect(downManager,
            SIGNAL(emit_replaceApp(QString, QVariantMap)),
            this,
            SLOT(wsQuery(QString, QVariantMap)));
    connect(downManager, SIGNAL(emit_killSheller()), SLOT(killSheller()));

    downManager->setDbName(dbUpdater);
    downManager->setUrlServerIp(IpServerName);
    downManager->settingsPath = settingsPath();

    toLog(LoggerLevel::Info, senderName, "Поиск устройств...");

    QString connectionName;

    // Начинаем искать устройства
    searchDevices->modemConUp = isModemConnectionUp(connectionName);
    searchDevices->searchValidator = config.searchValidator;
    searchDevices->searchCoinAcceptor = config.searchCoinAcceptor;
    searchDevices->searchPrinter = config.searchPrinter;
    searchDevices->searchModem = config.searchModem;
    searchDevices->searchWD = config.searchWD;
    searchDevices->testMode = testMode;

    // Создаем таблицу если нету таковой
    createSmsSendTable();

    // Проверяем есть ли записи в таблице смс
    if (terminalSmsCount() == 0) {
        insertSmsContentInf();
    }

    adminAuthDialog = new AvtorizationToAdminIn(this);
    connect(adminAuthDialog, SIGNAL(emit_openAdminDialog()), SLOT(openAdminDialog()));

    changePassword = new ChangePassword(this);
    connect(changePassword,
            SIGNAL(emit_changepass(QString, QString)),
            this,
            SLOT(editAdminAuthData(QString, QString)));

    // Удаляем старые логи (больше 2 месяцов)
    logClean->start();

    // Очищаем базу данных
    clearDataBase();

    // Открываем ws соединение
    wsConnectionOpen();

    if (skipSearchDevice) {
        // Вытаскиваем данные об устройствах из базы
        getDeviceFromDB();

        // Тут надо сделать инициализацию устройств
        devicesInitialization();

        // В начале надо проверить соединение с сервером
        connectionCheck();
    } else {
        searchDevices->prtWinName = getWinprinterFromDB();
        searchDevices->start();
    }
}

void MainWindow::deviceSearchStart(QVariantMap data) {
    config.searchValidator = data.value("search_validator").toBool();
    config.searchPrinter = data.value("search_printer").toBool();
    config.searchCoinAcceptor = data.value("search_coin_acceptor").toBool();
    config.searchWD = data.value("search_watchdog").toBool();
    config.searchModem = data.value("search_modem").toBool();

    QString connectionName;

    searchDevices->modemConUp = isModemConnectionUp(connectionName);
    searchDevices->searchValidator = config.searchValidator;
    searchDevices->searchCoinAcceptor = config.searchCoinAcceptor;
    searchDevices->searchPrinter = config.searchPrinter;
    searchDevices->searchModem = config.searchModem;
    searchDevices->searchWD = config.searchWD;
    searchDevices->testMode = testMode;

    searchDevices->start();
}

void MainWindow::connectionResult(bool result) {
    // Соединение есть
    if (result) {
        if (registrationForm) {
            registrationForm->setConnectionState(true);
            return;
        }

        // Проверим исчерпаны ли попытки отправки инкасации
        if (collectDaemons->countAllRep >= collectDaemons->RealRepeet) {
            // Запустим таймер отправки инкасаций
            if (!collectDaemons->demonTimer->isActive()) {
                collectDaemons->demonTimer->start(55000);
            }

            // Обнуляем количество отправок
            collectDaemons->countAllRep = 0;
        }

        toLog(LoggerLevel::Info, "CONNECTION", "Соединение с сервером активно");

        // Только при первом запуске
        if (oneGetServices) {
            connect(&servicesTimer, &QTimer::timeout, this, [&] {
                if (systemInfoGot) {
                    servicesTimer.stop();
                    getServicesRequest();
                }
            });

            servicesTimer.start(1000);
        }

        if (adminDialog->isVisible()) {
            auto data = QVariantMap({{"message", "Соединение с сервером активно"}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
        }

        mainPage->connectionIsUp = true;

        // Раз связь с сервером активна выходим из процедуры
        return;
    }

    // Соединения нет
    toLog(LoggerLevel::Error, "CONNECTION", "Нет соединение с сервером.");

    if (registrationForm) {
        registrationForm->setConnectionState(false, "Нет соединение с сервером");
        //        registrationForm->setConnectionState(true);
        return;
    }

    if (oneGetServices) {
        // Только при первой загрузки
        gotoPage(Page::LoadingGprs);

        // Удаление поиска устройств
        deleteSearchParam();

        loadingGprs->setSimInfo(
            QString("Оператор: \"%3\"               Сигнал %1: %2%               "
                    "Номер: \"%4\"               Баланс: %5")
                .arg(config.modemData.comment,
                     config.modemData.rate,
                     config.modemData.provider,
                     config.modemData.number,
                     config.modemData.balance));

        // Начинаем активизировать таймер проверки соединения
        connObject->startCheckConnection();
    }

    if (adminDialog->isVisible()) {
        auto data = QVariantMap({{"message", "Нет соединение с сервером."}});
        adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
    }

    mainPage->connectionIsUp = false;

    // Тут по идеи демон который проверяет соединение должен его поднять
    startToConnection();
}

void MainWindow::startToConnection() {
    QString text = "";

    // Чтобы не было конфликтов проверяем не находится ли оно в состоянии поднятия
    // соединения
    if (connObject->conState != Connection::conStateUpping &&
        connObject->conState != Connection::GetSimData &&
        connObject->conState != Connection::SendingSMS) {
        // Запускаем соединение
        connObject->connectNet();
    } else {

        switch (connObject->conState) {
        case Connection::conStateUpping: {
            text = "Соединение занято... идет поднятия соединения";
        } break;
        case Connection::GetSimData: {
            text = "Модем занят... идет опрос данных SIM карты";
        } break;
        case Connection::SendingSMS: {
            text = "Модем занят... идет отправка SMS сообщения";
        } break;
        }

        toLog(LoggerLevel::Warning, "CONNECTION", text);
    }

    QStringList vrmLst;
    vrmLst << text;
    auto data = QVariantMap({{"message", text}});

    if (adminDialog->isVisible()) {
        adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
    }
}

void MainWindow::getServicesRequest() {
    getServices->setAuthData(config.terminalData.token, systemHashGet(), ConstData::version);
    getServices->sendGetServicesQuery();
}

void MainWindow::getServicesReturn(bool result) {
    if (result) {
        countGS = 0;

        actionList[Action::aPoGetServices] = true;

        // Конфигурация получина успешно
        // Берём данные об услугах
        mainPage->services = getServicesFromDB();
        mainPage->categories = getCategoriesFromDB();
        mainPage->favoriteServicesInit();
        mainPage->interfaceCacheClear();

        if (adminDialog->isVisible()) {
            auto data = QVariantMap({{"message", "Конфигурация получена успешно..."}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, data);
        }
    } else {
        // Тут ошибка авторизации
        if (adminDialog->isVisible()) {
            auto data = QVariantMap({{"message", "Ошибка авторизации..."}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, data);
            adminDialog->authButtonSet(true);
        }

        QTimer::singleShot(60000, this, SLOT(getServicesRequest()));
    }

    // Надо посмотреть первый раз или нет
    if (oneGetServices) {
        gotoPage(Page::LoadingMain);

        oneGetServices = false;

        if (result) {
            QTimer::singleShot(2000, payDaemons, SLOT(sendPayRequest()));

            // Проверяем статус принтера
            getPrinterState = true;
            QTimer::singleShot(3000, clsPrinter, SLOT(CMD_GetStatus()));
        }

        // Начинаем активизировать таймер проверки соединения
        connObject->startCheckConnection();

        // Надо проверить есть ли непроведенная инкасация
        if (!collectDaemons->demonTimer->isActive()) {
            collectDaemons->demonTimer->start(60000);
        }

        // Запускаем невыполненные команды
        cmdTimer.start();

        // Проверяем открытие бокса
        bValidatorEventCheck();
    } else {

        if (mainPage->getStepByStepPage() == PageIn::Main) {
            mainPage->loadHtmlPage(PageIn::Main);
        }
    }
}

void MainWindow::getServicesError() {

    if (adminDialog->isVisible()) {
        auto data = QVariantMap(
            {{"message", "Истёк таймаут ожидания запроса\nна получение конфигурации..."}});
        adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, data);
        adminDialog->authButtonSet(true);
    }

    // Увеличивать на один количество повторов
    countGS++;

    // Проверяем если количество больше 3
    if (countGS > 3) {
        countGS = 0;

        // Тут перегружаем соединение
        int page = ui->mainStacker->currentIndex();
        if (page == Page::LoadingGprs) {
            connObject->connectNet();
        } else {
            QTimer::singleShot(10000, this, SLOT(getServicesRequest()));
        }
    } else {
        // Отправляем еще раз get services
        toLog(LoggerLevel::Info,
              senderName,
              QString("Попытка N-%1 на получение конфигурацию.").arg(countGS));

        QTimer::singleShot(10000, this, SLOT(getServicesRequest()));
    }
}

void MainWindow::deleteSearchParam() {
    if (sDevicesForm) {
        ui->searchDevicesLayout->removeWidget(sDevicesForm);
        delete sDevicesForm;
        sDevicesForm = 0;
    }

    if (searchDevices) {
        searchDevices->terminate();
        searchDevices->wait();
    }
}

bool MainWindow::isModemConnectionUp(QString &connectionName) {
    QStringList lstCon;
    bool isConnectionUp = connObject->getNowConnectionState(lstCon);

    connectionName = lstCon.count() > 1 ? lstCon.at(0) : "";

    return isConnectionUp;
}

void MainWindow::connectionError() {
    toLog(LoggerLevel::Error, "CONNECTION", "Ошибка поднятия соединения с " + config.vpnName);
}

void MainWindow::connectionError(QString errNum, QString errComment) {
    connObject->conState = Connection::conStateError;

    if (loadingGprs) {
        loadingGprs->setGprsInfo(QString("ERROR - %1").arg(errNum));
        loadingGprs->setGprsComment(errComment);
    }

    //    if (registrationForm) {
    //        registrationForm->setStatusText(2, QString("ERROR - %1
    //        (%2)").arg(errNum, errComment));
    //    }

    if (adminDialog->isVisible()) {
        QVariantMap data;
        data["message"] = QString("%1 -(%2)").arg(errNum, errComment);
        adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
    }

    toLog(LoggerLevel::Error, "CONNECTION", errNum + " (" + errComment + ")");
    int errInit = errNum.toInt();

    if (registrationForm) {
        registrationForm->setConnectionState(
            false,
            QString("ERROR %1").arg(errNum) +
                (errComment.isEmpty() ? "" : QString("\n%2").arg(errComment)));
        connObject->stopReconnect();
        return;
    }

    switch (errInit) {
    case 756: {
        // При ошибке 756 делаем перезапуск АСО

        // Перезагружаем модем
        // rebootModemEntries();

        /// Проверяем счетчик перезагрузок
        int count_reboot = rebootCount();
        toLog(LoggerLevel::Info,
              "MAIN",
              QString("Количество перезагрузок по ошибке 756 - %1").arg(count_reboot));

        if (count_reboot < 3 && count_reboot != 99) {
            toLog(LoggerLevel::Info, "MAIN", QString("Даем каманду на перезагрузку по ошибке 756"));
            whenRasReboot = true;

            cmdExec = CommandInit::cRebootTerminal;
            cmdExecTimer.start(10000);
        } else {
            // Если все же перезагрузка не помогла тогда перезагрузим через 15-30 мин
            afterRestartRas = true;

            // Перезагружаем модем
            rebootModemEntries();

            countRepEntires = 0;
            countOtherErrRas = 0;
        }
    } break;
    case 692: {
        // Перезагружаем модем
        rebootModemEntries();
    } break;
    default: {
        if (countRepEntires >= 2) {

            // Перезагружаем модем
            rebootModemEntries();

            countRepEntires = 0;

            // При остальных ошибках если число превышает 3
            rebootEntries(errInit);
        } else {

            countRepEntires = countRepEntires + 1;

            toLog(LoggerLevel::Info,
                  "CONNECTION",
                  QString("Попытка N - %1 на перезагрузку соединения завершена.")
                      .arg(countRepEntires));
        }
    } break;
    }
}

void MainWindow::rebootModemEntries() {
    // Тут будем перезагружать модем
    if (config.WDData.port.contains("COM")) {
        // Присваем порт
        watchDogs->setPort(config.WDData.port);
        WDProtocolCommands::Enum protocolCommand = WDProtocolCommands::ResetModem;
        toLog(LoggerLevel::Info, "WatchDog", QString("Даем каманду на перезагрузку модема..."));
        watchDogs->toCommandExec(true, protocolCommand);
    }
}

void MainWindow::rebootEntries(int errInit) {
    countOtherErrRas++;

    if (countOtherErrRas >= 3) {
        int countReboot = rebootCount();
        if (countReboot < 3 && countReboot != 99) {
            toLog(LoggerLevel::Info,
                  "MAIN",
                  QString("Даем каманду на перезагрузку по ошибке %1").arg(errInit));
            whenRasReboot = true;

            cmdExec = CommandInit::cRebootTerminal;
            cmdExecTimer.start(10000);

            countOtherErrRas = 0;
            countRepEntires = 0;
        }
    }
}

void MainWindow::getSystemInfo(QVariantMap data) {
    config.systemInfo = data;
    systemInfoGot = true;

    toLog(LoggerLevel::Info, senderName, "GET SYSINFO FINISHED");
    QJsonDocument json = QJsonDocument::fromVariant(config.systemInfo);
    toLog(LoggerLevel::Info, senderName, QString(json.toJson(QJsonDocument::Compact)));
}

void MainWindow::connectionState(QString res, QString comment) {
    if (res != "ERROR") {

        if (loadingGprs) {
            loadingGprs->setGprsInfo(res);
            loadingGprs->setGprsComment(comment);
        }

        if (registrationForm) {
            registrationForm->setStatusText(
                0, res + (comment.isEmpty() ? "" : QString("\n%2").arg(comment)));
        }

        if (adminDialog->isVisible()) {
            QVariantMap data;
            data["message"] = res;
            adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
        }

        toLog(0, "CONNECTION", res + " (" + comment + ")");
    }
}

void MainWindow::connectionUpState() {
    countOtherErrRas = 0;
    countRepEntires = 0;
    afterRestartRas = false;
    // toDebuging("\n<<======= Connection UP ========\n");

    connObject->conState = Connection::conStateUp;

    toLog(LoggerLevel::Info, "CONNECTION", "Соединение с " + config.vpnName + " установлено.");

    if (adminDialog->isVisible()) {
        QVariantMap data;
        data["message"] = "Соединение с " + config.vpnName + " установлено.";
        adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
    }

    if (registrationForm) {
        registrationForm->setStatusText(1, "Соединение поднято успешно");
    }

    // Только при первом запуске
    if (oneGetServices) {
        // Начинаем опять проверять соединение
        // И все идет по кругу
        connectionCheck();
    }
}

void MainWindow::connectionCheck() {
    // Делаем проверку соединения
    // В начале надо проверить соединение с сервером
    toLog(LoggerLevel::Info, "CONNECTION", "Начинаем проверять соединение с сервером");
    connObject->checkConnection(TypePing::Request);
}

bool MainWindow::checkUserInBase() {
    QSqlQuery userSql(db);

    QString userQuery = "SELECT * FROM terminal_data;";

    if (!userSql.exec(userQuery)) {
        return false;
    }

    if (!userSql.isSelect()) {
        return false;
    }

    QSqlRecord record = userSql.record();

    QString vrmLogin = "";
    QString vrmToken = "";
    QString vrmSecretLogin = "";
    QString vrmSecretPass = "";

    if (userSql.next()) {
        vrmLogin = userSql.value(record.indexOf("login")).toString();
        vrmToken = userSql.value(record.indexOf("token")).toString();
        vrmSecretLogin = userSql.value(record.indexOf("secret_login")).toString();
        vrmSecretPass = userSql.value(record.indexOf("secret_pass")).toString();
    }

    if (vrmLogin.trimmed() == "" || vrmToken.trimmed() == "") {
        // Некорректные данные о пользователе
        return false;
    }

    // Расшифровка данных и запись их в переменные
    config.terminalData.login = decodeStr(vrmLogin, config.coddingKey);
    config.terminalData.token = decodeStr(vrmToken, config.coddingKey);
    config.terminalData.secretLogin = decodeStr(vrmSecretLogin, config.coddingKey);
    config.terminalData.secretPass = decodeStr(vrmSecretPass, config.coddingKey);

    return true;
}

void MainWindow::getRegistrationData(QVariantMap data) {
    auto login = data.value("login").toString();
    auto token = data.value("token").toString();
    auto adminLogin = data.value("admin_login").toString();
    auto adminPassword = data.value("admin_password").toString();

    // Записываем данные
    saveTerminalAuthData(false, login, token);
    saveAdminAuthData(true, adminLogin, adminPassword);

    // Выставляем шаблон
    config.tpl = data.value("tpl", "tjk").toString();
    config.test = data.value("test", false).toBool();

    if (config.tpl == "uzb") {
        config.langDefault = "local";
    }

    // Соединение
    config.vpnName = data.value("connection").toString();

    // Создаем нулевую инкассацию если не существует
    if (collectDaemons->getCollectionCount() == 0) {
        collectDaemons->firstCollection = true;
        auto collectionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        collectDaemons->createNewCollection(collectionId);
    }

    // Закрываем окно регистрации
    if (registrationForm->isVisible()) {
        registrationForm->close();
        registrationForm = nullptr;
    }

    // Делаем проверку пользователя
    if (checkUserInBase()) {
        settingsSave();

        checkConfigData(true);
    }
}

bool MainWindow::saveTerminalAuthData(bool update, QString login, QString token) {

    QSqlQuery userSql(db);
    QString userQuery;

    login = encodeStr(login, config.coddingKey);
    token = encodeStr(token, config.coddingKey);

    if (update) {
        userQuery = QString("UPDATE terminal_data SET login = '%1', token = '%2' WHERE id = 1;");
    } else {
        userQuery = QString("INSERT INTO terminal_data(id, login, token) VALUES(1, '%1', '%2');");
    }

    userQuery = userQuery.arg(login, token);

    if (!userSql.exec(userQuery)) {
        // toDebuging("Error insert or update User data");
        return false;
    }

    return true;
}

bool MainWindow::saveAdminAuthData(bool update, QString secretLogin, QString secretPassword) {

    QSqlQuery userSql(db);
    QString userQuery;

    // Необходимо зашифровать данные
    secretLogin = encodeStr(secretLogin, config.coddingKey);
    secretPassword = encodeStr(secretPassword, config.coddingKey);

    if (update) {
        userQuery = QString("UPDATE terminal_data SET secret_login = '%1', "
                            "secret_pass = '%2' WHERE id = 1;");
    } else {
        userQuery = QString("INSERT INTO terminal_data(id, secret_login, "
                            "secret_pass) VALUES(1, '%1', '%2');");
    }

    userQuery = userQuery.arg(secretLogin, secretPassword);

    if (!userSql.exec(userQuery)) {
        qDebug() << "Error insert or update User data";
        return false;
    }

    return true;
}

void MainWindow::openAdminAuthDialog() {

    adminAuthDialog->setAuthParam(config.terminalData.secretLogin, config.terminalData.secretPass);
    //    adminAuthDialogIn->setAuthParam("humopay20", "Humo#2023#P@y");
    adminAuthDialog->show();
}

void MainWindow::openAdminAuthEditDialog() {
    changePassword->show();
}

void MainWindow::editAdminAuthData(QString login, QString password) {
    saveAdminAuthData(true, login, password);
    checkUserInBase();
}

void MainWindow::createDialUpConnection(QVariantMap data) {

    QString conName, devName, phone, login, pass;

    devName = data.value("device").toString();
    conName = data.value("connection").toString();
    phone = data.value("phone").toString();
    login = data.value("login").toString();
    pass = data.value("password").toString();

    // В начале надо проверить соединение с сервером
    toLog(LoggerLevel::Info,
          "CONNECTION",
          "Начинаем создавать Dialup соединение с параметрами:\n" +
              QString("Наименование устройства: %1\n"
                      "Наименование соединения: %2\n"
                      "Номер дозвона:           %3\n"
                      "Имя пользователя:        %4\n"
                      "Пароль пользователя:     %5\n")
                  .arg(devName, conName, phone, login, pass));

    // Создаем соединение
    int status = connObject->createNewDialupConnection(conName, devName, phone, login, pass);

    QString title = "Создание соединения";
    QString text = "";
    bool success = false;

#ifdef Q_OS_WIN
    switch (status) {
    case ErrorDialup::rErrorCreateDialupCon:
        text = "Ошибка при создании соединения";
        break;
    case ErrorDialup::rErrorSetDialupParam:
        text = "Ошибка при присвоении параметров дозвона";
        break;
    default:
        text = "Соединение успешно создана";
        success = true;
        break;
    }
#else
    text = "Соединение успешно создана";
    success = true;
#endif

    if (adminDialog->isVisible()) {
        adminDialog->showMsgDialog(title, text);

        // Вставляем список
        getCommandFromAdmin(AdminCommand::aCmdRasConnlist);
    }

    if (registrationForm && registrationForm->isVisible()) {
        registrationForm->setStatusText(success ? 1 : 2, text);

        // Вставляем список соединений
        if (success) {
            QStringList connectionList;
            connectionList << QString("Local connection");

            auto rasConnectionList = connObject->getRasConnectionList();
            if (rasConnectionList.count() > 0) {
                connectionList.append(rasConnectionList);
            }

            registrationForm->setDataListConnection(connectionList);
        }
    }
}

void MainWindow::startToConnect(QString connection) {
    //    fromReg = true;

    config.vpnName = connection;

    if (connection.toUpper() == "LOCAL CONNECTION") {
        connectionCheck();
    } else {
        connObject->setConnectionConfig(connection);

        QString connectionName;

        if (isModemConnectionUp(connectionName)) {
            // Тут надо сообщить регистрации что соединение поднято
            registrationForm->setStatusText(1, tr("Соединение с %1 поднято").arg(connectionName));

            connectionCheck();
        } else {
            // Тут надо подымать соединение
            startToConnection();
        }
    }
}

void MainWindow::sendAuthRequest(QString login, QString otp) {
    auto hash = systemHashGet();

    authRequest->version = ConstData::version;

    toLog(LoggerLevel::Info,
          "AUTH",
          QString("Отправляется запрос на авторизацию (логин: %1)").arg(login));

    auto cid = collectDaemons->firstCollection ? collectDaemons->getCollectionId() : "";

    authRequest->sendAuthRequest(login, otp, hash, cid);
}

void MainWindow::authResponse(QString resultCode, QString login, QString token, QString message) {
    if (registrationForm) {
        registrationForm->authResponse(resultCode, token, message);
        return;
    }

    QVariantMap msg;

    if (resultCode != "" && resultCode.toInt() == 0 && token != "") {
        // Сохраняем данные
        saveTerminalAuthData(false, login, token);
        saveAdminAuthData(true, config.terminalData.secretLogin, config.terminalData.secretPass);

        checkUserInBase();

        // Получаем конфигурацию
        if (adminDialog->isVisible()) {
            msg = QVariantMap({{"message", "Получение конфигурации ..."}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, msg);
        }

        getServicesRequest();
        return;
    }

    if (resultCode == "") {
        if (adminDialog->isVisible()) {
            msg = QVariantMap({{"message", "Сервер недоступен или нет связи"}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, msg);
            adminDialog->authButtonSet(true);
        }

        toLog(LoggerLevel::Warning, "AUTH", "Сервер недоступен или нет связи");
    } else {
        message = message.isEmpty() ? "Ошибка авторизации" : message;
        if (adminDialog->isVisible()) {
            msg = QVariantMap({{"message", message}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, msg);
            adminDialog->authButtonSet(true);
        }

        toLog(LoggerLevel::Error, "AUTH", message);

        config.terminalData.login = "";
        config.terminalData.token = "000000";

        applyAuthToModules();
    }
}

QString MainWindow::systemHashGet() {
    auto disk = config.systemInfo.value("disk").toMap();
    auto mboard = config.systemInfo.value("mboard").toMap();

    QString diskModel = disk.value("model").toString();
    QString diskSerial = disk.value("serialnumber").toString();
    QString mboardProduct = mboard.value("product").toString();
    QString mboardSerialnumber = mboard.value("serialnumber").toString();

    QString uuid =
        diskModel + diskSerial + mboardProduct + mboardSerialnumber + QDir::currentPath();
    QString hash = QCryptographicHash::hash(uuid.toUtf8(), QCryptographicHash::Sha256).toHex();

    return hash;
}

void MainWindow::openAdminDialog() {

    // Если открыта инкасация закрываем её
    if (incasaciyaForm->isVisible()) {
        incasaciyaForm->close();
    }

    // Присваиваем тайтл
    adminDialog->setWindowTitle("Admin ( " + versionFull() + " )");

    QVariantMap data;

    data["message"] = lockList[getLock()].comment;

    // Ставим инфу о состоянии терминала
    adminDialog->setDataToAdmin(AdminCommand::aCmdShowAsoStatus, data);

    adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, QVariantMap({{"message", ""}}));

    lockUnlockCenter(Lock::ErrorOpenAdminP, true);

    // Баланс агента
    auto balance = terminalInfo.value("balance").toString();
    auto overdraft = terminalInfo.value("overdraft").toString();
    getBalanceAgentData(balance, overdraft);

    // Новые платежи
    int i_count = 0;
    payDaemons->getCountPayment(i_count);

    data.clear();
    data["payment_count"] = i_count;
    adminDialog->setDataToAdmin(AdminCommand::aCmdGetNewOperation, data);

    // Список инкасаций
    QStringList lstIncash;
    lstIncash << adminDialog->titleDataIncashment;
    int count_inc = 0;
    collectDaemons->getDataCollectList(lstIncash, count_inc);
    data["encash_list"] = lstIncash;
    adminDialog->setDataToAdmin(AdminCommand::aCmdListAllIncash, data);

    // Показываем html инкасации
    // Вставляем информацию о состоянии бокса
    QString nonCollectPay = "0";
    int moneyOutCount = 0;
    double moneyOutSum = 0;
    QString cId = "";
    QString cTrn = "";
    QString trnFrom = "";
    QString trnTo = "";
    QString htmlCenter = collectDaemons->getHtmlInfoBox(
        nonCollectPay, moneyOutCount, moneyOutSum, "", cId, cTrn, trnFrom, trnTo);

    // Делаем небольшой html
    QString header =
        QString("<ul>"
                "<li>Не инкасированных платежей - " +
                nonCollectPay +
                "</li>"
                "<li>Количество купюр мимо      - " +
                QString::number(moneyOutCount) + " на сумму - " + QString::number(moneyOutSum) +
                "</li>"
                "</ul>");

    QString allHtml = QString(header + htmlCenter);

    data.clear();
    data["html"] = allHtml;
    data["c_id"] = cId;
    data["c_trn"] = cTrn;
    data["trn_from"] = trnFrom;
    data["trn_to"] = trnTo;

    adminDialog->setDataToAdmin(AdminCommand::aCmdHtmlIncash, data);

    /// Страница устройств

    QStringList pList = portList();

    // Вставляем список купюрников
    QString vrmValidatorInfo =
        QString("firmware: %1\n"
                "serial: %2")
            .arg(config.validatorData.partNumber, config.validatorData.serialNumber);

    data["validator_info"] = vrmValidatorInfo;          // Info
    data["validator_list"] = billValidatorList;         // ListValidator
    data["validator_name"] = config.validatorData.name; // Name
    data["validator_port_list"] = pList;                // List Com Portov
    data["validator_port"] = config.validatorData.port; // Active Port

    adminDialog->setDataToAdmin(AdminCommand::aCmdValidatorInform, data);

    // Вставляем список монетоприемников
    QString vrmCoinAcceptorInfo =
        QString("model:  %1\n"
                "serial:   %2")
            .arg(config.coinAcceptorData.partNumber, config.coinAcceptorData.serialNumber);

    data["coin_acceptor_info"] = vrmCoinAcceptorInfo;          // Info
    data["coin_acceptor_list"] = coinAcceptorList;             // ListValidator
    data["coin_acceptor_name"] = config.coinAcceptorData.name; // Name
    data["coin_acceptor_port_list"] = pList;                   // List Com Portov
    data["coin_acceptor_port"] = config.coinAcceptorData.port; // Active Port

    adminDialog->setDataToAdmin(AdminCommand::aCmdCoinAcceptorInf, data);

    // Вставляем список принтеров
    data["printer_comment"] = config.printerData.comment; // Info
    data["printer_list"] = printerList;                   // ListPrinter
    data["winprinter_list"] = getWinPrinterNames();       // ListWinprinter
    data["printer_name"] = config.printerData.name;       // Name
    data["printer_port_list"] = pList;                    // List Com Portov
    data["printer_port"] = config.printerData.port;       // Active Port

    adminDialog->setDataToAdmin(AdminCommand::aCmdPrinterInform, data);

    // Вставляем список модемов
    data["modem_comment"] = config.modemData.comment; // Info
    data["modem_name"] = config.modemData.name;       // Name
    data["modem_port_list"] = pList;                  // List Com Portov
    data["modem_port"] = config.modemData.port;       // Active Port

    adminDialog->setDataToAdmin(AdminCommand::aCmdModemInform, data);

    // Вставляем список в информацию о сторожевике
    data["watchdog_comment"] = config.WDData.comment; // Info
    data["watchdog_name"] = config.WDData.name;       // Name
    data["watchdog_port_list"] = pList;               // List Com Portov
    data["watchdog_port"] = config.WDData.port;       // Active Port

    adminDialog->setDataToAdmin(AdminCommand::aCmdWDInform, data);

    // Вставляем данные по поиску устройств
    data["search_validator"] = config.searchValidator;
    data["search_coin_acceptor"] = config.searchCoinAcceptor;
    data["search_printer"] = config.searchPrinter;
    data["search_modem"] = config.searchModem;
    data["search_watchdog"] = config.searchWD;

    adminDialog->setDataToAdmin(AdminCommand::aCmdSearchParamRef, data);

    // Вставляем информацию о сим карте
    data["modem_sim_provider"] = config.modemData.provider;
    data["modem_sim_number"] = config.modemData.number;
    data["modem_sim_rate"] = config.modemData.rate + "%";
    data["modem_sim_balance"] = config.modemData.balance;

    adminDialog->setDataToAdmin(AdminCommand::aCmdSimInfoData, data);

    // Вставляем список соединений
    getCommandFromAdmin(AdminCommand::aCmdRasConnlist);

    // Интервал перезагрузки ПО при RAS ошибках
    data["ras_error_interval_reboot"] = config.timerRasReb;
    adminDialog->setDataToAdmin(AdminCommand::aCmdErrorRasReb, data);

    // Список устройств для создания Dialup подключения
    QStringList lstDevDialup;
    bool dialDev = connObject->hasInstalledModems(lstDevDialup);
    if (!dialDev)
        toLog(LoggerLevel::Error, "CONNECTION", "На терминале нет установленных модемов...");
    adminDialog->dialupDevice = lstDevDialup;

    // Параметры модема
    data["check_balance_sim"] = config.checkGetBalanceSim;
    data["check_number_sim"] = config.checkGetNumberSim;
    data["ussd_balance_sim"] = config.simBalanceRequest;
    data["index_check_balance"] = config.indexCheckBalance;
    data["ussd_number_sim"] = config.simNumberRequest;

    adminDialog->setDataToAdmin(AdminCommand::aCmdModemInfData, data);

    // Общие параметры чека
    data["show_print_dialog"] = config.showPrintDialog;
    data["chek_small_beetwen_string"] = config.printerData.smallBeetwenString;
    data["chek_small_text"] = config.printerData.smallChek;
    data["chek_width"] = config.printerData.checkWidth;
    data["chek_left_size"] = config.printerData.leftMargin;
    data["printing_chek"] = config.printerData.printCheckUntil;

    adminDialog->setDataToAdmin(AdminCommand::aCmdPrinterInfData, data);

    // Параметры SMS оповещений
    data["sms_err_validator"] = config.smsErrValidator;
    data["sms_err_printer"] = config.smsErrPrinter;
    data["sms_err_balance_agent"] = config.smsErrBalanceAgent;
    data["sms_value_balance_agent"] = config.smsValueBalanceAgent;
    data["sms_err_sim_balance"] = config.smsErrSimBalance;
    data["sms_err_lock_terminal"] = config.smsErrLockTerminal;
    data["sms_err_connection"] = config.smsErrConnection;

    adminDialog->setDataToAdmin(AdminCommand::aCmdSmsSending, data);

    // Параметры win printera
    data["prt_win_width"] = config.winPrtChekWidth;
    data["prt_win_height"] = config.winPrtChekHeight;
    data["prt_win_font_size"] = config.winPrtChekFontSize;
    data["prt_win_left_margin"] = config.winPrtChekLeftMargin;
    data["prt_win_right_margin"] = config.winPrtChekRightMargin;
    data["prt_win_top_margin"] = config.winPrtChekTopMargin;
    data["prt_win_bottom_margin"] = config.winPrtChekBottomMargin;

    adminDialog->setDataToAdmin(AdminCommand::aCmdWinPrinterParam, data);

    // Параметры счетчика чека
    data["exist_counter_chek"] = config.existCounterChek;
    data["counter_len_rulon"] = config.counterWidthRulon;
    data["counter_len_chek"] = config.counterWidthChek;
    data["counter_ring_value"] = config.counterCountCheck;

    adminDialog->setDataToAdmin(AdminCommand::aCmdCounterCheckInf, data);

    // Сколько накрутило
    data["counter_info"] = "всего: 0 шт.\nнапечатано: 0 шт.\nосталось: 0 шт.";
    adminDialog->setDataToAdmin(AdminCommand::aCmdCounterCheckVal, data);

    // Данные авторизации терминала
    data.clear();
    data["login"] = config.terminalData.login;
    data["secret_login"] = config.terminalData.secretLogin;

    adminDialog->setDataToAdmin(AdminCommand::aCmdAvtorizationTrmP, data);

    // Остальные настройки
    data["status_validator_jam_in_box"] = config.statusValidatorJamInBox;
    data["status_validator_jam_in_box_value_counter"] = config.statusValidatorJamInBoxValueCounter;
    data["status_validator_jam_in_box_lockers"] = config.statusValidatorJamInBoxLockers;
    data["default_lang"] = config.langDefault;
    data["auto_update_status"] = config.autoUpdateStatus;
    data["exist_counter_printer_chek"] = config.existCounterPrinterChek;
    data["lock_duplicate_nominal"] = config.lockDuplicateNominal;

    adminDialog->setDataToAdmin(AdminCommand::aCmdOtherSettings, data);

    // Открываем админку
    adminDialog->openThis();
}

void MainWindow::getCommandFromAdmin(AdminCommand::AdminCmd cmd) {
    QProcess proc;

    switch (cmd) {
    case AdminCommand::aCmdGetBalance: {
        getBalanceAgent->sendDataRequest();
    } break;
    case AdminCommand::aCmdHtmlIncash: {
        QString dateCollect = "";
        if (adminDialog->dateCollectParam != adminDialog->titleDataIncashment) {
            dateCollect = adminDialog->dateCollectParam;
        }

        // Вставляем информацию о состоянии бокса
        QString nonCollectPay = "0";
        int moneyOutCount = 0;
        double moneyOutSum = 0;
        QString cId = "";
        QString cTrn = "";
        QString trnFrom = "";
        QString trnTo = "";
        QString htmlCenter = collectDaemons->getHtmlInfoBox(
            nonCollectPay, moneyOutCount, moneyOutSum, dateCollect, cId, cTrn, trnFrom, trnTo);

        // Делаем небольшой html
        QString header =
            QString("<ul>"
                    "<li>Не инкасированных платежей - " +
                    nonCollectPay +
                    "</li>"
                    "<li>Количество купюр мимо      - " +
                    QString::number(moneyOutCount) + " на сумму - " + QString::number(moneyOutSum) +
                    "</li>"
                    "</ul>");

        QString allHtml = QString(header + htmlCenter);

        QVariantMap data;
        data["html"] = allHtml;
        data["c_id"] = cId;
        data["c_trn"] = cTrn;
        data["trn_from"] = trnFrom;
        data["trn_to"] = trnTo;

        adminDialog->setDataToAdmin(AdminCommand::aCmdHtmlIncash, data);
    } break;
    case AdminCommand::aCmdExecIncashmant: {
        int page = ui->mainStacker->currentIndex();

        if (page == Page::LoadingMain) {
            // Делаем инкасацию
            QString text = "";
            if (collectDaemons->getCheckText(text, false, "")) {
                if (text != "") {
                    clsPrinter->CMD_Print(text);
                }
            }
        } else {
            QMessageBox msgBox;
            msgBox.setWindowTitle(windowTitle());
            msgBox.setText("Инкассация возможна после авторизации");
            msgBox.exec();
        }
    } break;
    case AdminCommand::aCmdExecDateIncash: {
        auto dateCollect = adminDialog->dateCollectParam;
        // Делаем инкассацию
        QString text = "";
        if (dateCollect != "") {
            if (collectDaemons->getCheckText(text, true, dateCollect)) {
                if (text != "") {
                    if (config.printerData.state == PrinterState::PrinterNotAvailable) {
                        adminDialog->showMsgDialog("Принтер ",
                                                   "Принтер недоступен для печати чека инкассации");
                        return;
                    }

                    clsPrinter->CMD_Print(text);
                }
            }
        }
    } break;
    case AdminCommand::aCmdShowKeyPud: {
        proc.start("taskkill", QStringList() << "/f" << "/IM" << "osk.exe");
        proc.waitForFinished(2000);

        proc.startDetached("osk.exe", QStringList());
    } break;
    case AdminCommand::aCmdShowExplorer: {
        proc.startDetached("explorer.exe", QStringList());
        proc.waitForFinished(2000);

        proc.startDetached("taskmgr.exe", QStringList());
        proc.waitForFinished(2000);

        wsQuery("state_stop");

        close();
    } break;
    case AdminCommand::aCmdHideExplorer: {
        proc.startDetached("taskkill", QStringList() << "/f" << "/IM" << "explorer.exe");
    } break;
    case AdminCommand::aCmdPrintTestCheck: {
        // если это km1x
        if (adminDialog->printerName == PrinterModel::KM1X) {
            config.printerData.state = PrinterState::PrinterOK;

            if (config.printerData.name != PrinterModel::KM1X) {
                config.printerData.name = adminDialog->printerName;
                // Тут обявляем какую модель принтера
                clsPrinter->setPrinterModel(config.printerData.name);
            }

            config.printerData.port = adminDialog->printerPort;

            clsPrinter->portSpeed = adminDialog->printerPortSpeed;
            clsPrinter->setPortName(config.printerData.port);

            // Открытие порта
            clsPrinter->printerOpen();
        }

        QStringList vrmLst;
        int prtStatus = printerStatusList().toInt();

        if (prtStatus) {
            auto data = QVariantMap({{"message", printerStatus.at(prtStatus)}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdInfrmationPanel, data);
            toLog(LoggerLevel::Error, "PRINTER", QString("Статус (%1)").arg(vrmLst.at(0)));
            return;
        }

        toLog(LoggerLevel::Info, "PRINTER", "Принтер активен...");

        // Делаем инкасацию
        QString text = payDaemons->getReceiptInfo("");

        if (text != "") {
            clsPrinter->winPrinterName = adminDialog->printerComment;
            clsPrinter->CMD_Print(text);
            auto data = QVariantMap({{"message", "Идет печать пробного чека..."}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdInfrmationPanel, data);
        }
    } break;
    case AdminCommand::aCmdRestartValidator: {
        // Команда на перезагрузку купюроприемника
        clsValidator->execCommand(ValidatorCommands::Restart);
    } break;
    case AdminCommand::aCmdRestartModem: {
        // Тут будем перезагружать модем
        rebootModemEntries();
    } break;
    case AdminCommand::aCmdRestartApp: {
        QVariantMap data;
        data["app"] = qApp->applicationName();

        wsQuery("restart", data);
    } break;
    case AdminCommand::aCmdRestartASO: {
        if (!connObject->restartWindows(true)) {
#ifdef Q_OS_WIN32
            proc.startDetached("c:/windows/system32/cmd.exe",
                               QStringList() << "/c" << "shutdown -r -t 0");
#endif // Q_OS_WIN32
        }
    } break;
    case AdminCommand::aCmdShutDounASO: {
        if (!connObject->restartWindows(false)) {
#ifdef Q_OS_WIN32
            proc.startDetached("c:/windows/system32/cmd.exe",
                               QStringList() << "/c" << "shutdown -s -t 0");
#endif // Q_OS_WIN32
        }
    } break;
    case AdminCommand::aCmdSaveDeviceParam: {
        auto data = adminDialog->settings;

        for (auto &key : data.keys()) {
            settingsSet(key, data.value(key));
        }

        settingsGet();

        auto msg = QVariantMap({{"message", "Изменения успешно сохранены..."}});
        adminDialog->setDataToAdmin(AdminCommand::aCmdInfrmationPanel, msg);
    } break;
    case AdminCommand::aCmdSaveDeviceParamR: {
        auto data = adminDialog->settings;

        for (auto &key : data.keys()) {
            settingsSet(key, data.value(key));
        }

        settingsGet();

        auto validatorName = data.value("validator_name").toString();
        auto validatorPort = data.value("validator_port").toString();
        auto coinAcceptorName = data.value("coin_acceptor_name").toString();
        auto coinAcceptorPort = data.value("coin_acceptor_port").toString();
        auto printerName = data.value("printer_name").toString();
        auto printerPort = data.value("printer_port").toString();
        auto printerComment = data.value("printer_comment").toString();
        auto modemPort = data.value("modem_port").toString();
        auto watchdogPort = data.value("watchdog_port").toString();

        // если выбран принтер KM1X, то в коммент добавим скорость порта
        if (validatorName == PrinterModel::KM1X) {
            printerComment = data.value("printer_port_speed").toString();
        }

        // Тут надо в базу записать устройства
        saveDevice(1, validatorName, validatorPort, config.validatorData.comment, 1);
        saveDevice(2, printerName, printerPort, printerComment, 1);
        saveDevice(3, config.modemData.name, modemPort, config.modemData.comment, 1);
        saveDevice(4, config.WDData.name, watchdogPort, config.WDData.comment, 1);
        saveDevice(5, coinAcceptorName, coinAcceptorPort, config.coinAcceptorData.comment, 1);

        toLog(LoggerLevel::Info, "MAIN", "Устройства успешно сохранены.");

        auto msg = QVariantMap({{"message", "Изменения успешно сохранены..."}});

        adminDialog->setDataToAdmin(AdminCommand::aCmdInfrmationPanel, msg);

        // Тут делаем перезагрузку программы
        QVariantMap d;
        data["app"] = qApp->applicationName();

        wsQuery("restart", d);
    } break;
    case AdminCommand::aCmdCheckConnect: {
        // В начале надо проверить соединение с сервером
        connectionCheck();
    } break;
    case AdminCommand::aCmdRasConnCreate: {
        createDialUpConnection(adminDialog->data);
    } break;
    case AdminCommand::aCmdRasConnlist: {
        // Вставляем список соединений
        QVariantMap data;

        QStringList connectionList;
        connectionList << QString("Local connection");

        auto rasConnectionList = connObject->getRasConnectionList();
        if (rasConnectionList.count() > 0) {
            connectionList.append(rasConnectionList);
        }

        data["connection_list"] = connectionList;
        data["vpn_point"] = config.vpnName;

        adminDialog->setDataToAdmin(AdminCommand::aCmdRasConnlist, data);
    } break;
    case AdminCommand::aCmdGetActiveDialup: {
        // Вытаскиваем активное рас соединение
        QString connectionName;
        QString text;

        if (isModemConnectionUp(connectionName)) {
            text = QString("1- ( %1 ) активно...\t").arg(connectionName);
        } else {
            text = "Нет активных соединений...";
        }

        QVariantMap data;
        data["message"] = text;
        adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
    } break;
    case AdminCommand::aCmdRestartDialupCon: {
        /// Тут по идеи демон который проверяет соединение должен его поднять
        startToConnection();
    } break;
    case AdminCommand::aCmdGetSimInfo: {
        if (connObject->conState != Connection::GetSimData) {
            QVariantMap data;

            // Проверяем поднято ли соединение
            QString connectionName;

            if (isModemConnectionUp(connectionName)) {
                data["message"] =
                    QString("Подождите идет разрыв соединения %1...").arg(connectionName);

                adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);

                // Надо опустить соединение
                connObject->disconnectNet();

                data["message"] =
                    QString("Соединение %1 опущено, начинаем проверять данные SIM карты...")
                        .arg(connectionName);
            } else {
                data["message"] = QString("Начинаем проверять данные SIM карты...");
            }

            adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);

            // Тут надо опрашивать модем
            if (config.modemData.port != "") {
                // Занимаем модем
                connObject->conState = Connection::GetSimData;

                clsModem->setPort(config.modemData.port);

                bool nowSimPresent;
                QString signalQuality;
                QString operatorName;
                QString modemComment;
                QString simNumber;
                QString simBalance;
                bool modem_p = false;

                if (clsModem->isItYou(modemComment)) {
                    modem_p = true;
                    // toDebuging("--- EXIT MODEM isItYou ---");
                    nowSimPresent = clsModem->nowSimPresent;
                    signalQuality = clsModem->nowModemQuality;
                    operatorName = clsModem->nowProviderSim;
                    modemComment = clsModem->nowModemComment;

                    // Присваиваем программе
                    config.modemData.comment = modemComment.replace("'", "").replace(".", "");
                    config.modemData.found = true;
                    config.modemData.present = nowSimPresent;
                    config.modemData.provider = operatorName;
                    config.modemData.rate = signalQuality;
                }

                // Проверяем номер и баланс если есть ответ от модема
                if (modem_p) {
                    clsModem->ussdRequestNumberSim = config.simNumberRequest;
                    clsModem->execCommand(ModemProtocolCommands::GetSimNumber, false);
                    simNumber = clsModem->nowNumberSim;
                    // toDebuging("--- Now Number Sim --- " + simNumber);

                    // Баланс
                    // Присваиваем данные
                    clsModem->ussdRequestBalanseSim = config.simBalanceRequest;
                    clsModem->indexBalanceParse = config.indexCheckBalance;
                    clsModem->execCommand(ModemProtocolCommands::GetBalance, false);
                    simBalance = clsModem->nowBalanceSim;
                    // toDebuging("--- Now Balance Sim --- " + simBalance);

                    config.modemData.number = simNumber;
                    config.modemData.balance = simBalance;

                    // Отправляем на сервер данные мониторинга
                    statusDaemons->firstSend = true;
                    oneSendStatus = true;

                    // Показываем в админке
                    data["modem_sim_provider"] = config.modemData.provider;
                    data["modem_sim_number"] = config.modemData.number;
                    data["modem_sim_rate"] = config.modemData.rate + "%";
                    data["modem_sim_balance"] = config.modemData.balance;

                    adminDialog->setDataToAdmin(AdminCommand::aCmdSimInfoData, data);

                    toLog(LoggerLevel::Info,
                          "CONNECTION",
                          "Параметры опроса SIM карты\n\n"
                          "- Провайдер        - " +
                              config.modemData.provider + "\n- Номер SIM карты  - " +
                              config.modemData.number + "\n- Уровень сигнала  - " +
                              config.modemData.rate + "%" + "\n- Баланс SIM карты - " +
                              config.modemData.balance);
                }

                // Освобождаем модем
                data["message"] = QString("Данные успешно проверены...");
                adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
            } else {
                // Нет наименование порта для модема...
                data["message"] = QString("Нет наименование порта для модема...");
                adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, data);
            }

            connObject->conState = Connection::conStateDown;

            // Подымаем соединение
            QTimer::singleShot(1000, this, SLOT(startToConnection()));
        }
    } break;
    case AdminCommand::aCmdSaveConnParam: {
        auto data = adminDialog->settings;

        for (auto &key : data.keys()) {
            settingsSet(key, data.value(key));
        }

        settingsGet();

        toLog(LoggerLevel::Info, "MAIN", "Параметры соединения успешно сохранены.");

        auto msg = QVariantMap({{"message", "Изменения успешно сохранены..."}});
        adminDialog->setDataToAdmin(AdminCommand::aCmdConnectInfo, msg);

        adminDialog->showMsgDialog("Сохранение параметров",
                                   "Параметры соединения успешно сохранены.");
    } break;
    case AdminCommand::aCmdSavePrinterParam: {
        auto data = adminDialog->settings;

        for (auto &key : data.keys()) {
            settingsSet(key, data.value(key));
        }

        settingsGet();

        toLog(LoggerLevel::Info, "MAIN", "Параметры принтера успешно сохранены.");

        adminDialog->showMsgDialog("Сохранение параметров",
                                   "Параметры принтера успешно сохранены.");
    } break;
    case AdminCommand::aCmdGetServices: {
        if (updateHashConfig("")) {
            getServicesRequest();
        }
    } break;
    case AdminCommand::aCmdSaveOtherSetting: {
        auto data = adminDialog->settings;

        for (auto &key : data.keys()) {
            settingsSet(key, data.value(key));
        }

        settingsGet();

        if (adminDialog->unlockNominalDuplicate()) {
            saveLockDuplicateNominal(false);
        }

        toLog(LoggerLevel::Info, "MAIN", "Настройки успешно сохранены.");

        adminDialog->showMsgDialog("Сохранение параметров", "Настройки успешно сохранены.");
    } break;
    case AdminCommand::aCmdSaveTrmNumSett: {
        auto data = adminDialog->settings;

        auto login = data.value("login").toString();
        auto token = data.value("token").toString();

        QVariantMap msg;

        msg = QVariantMap({{"message", "Проверка авторизации ..."}});
        adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, msg);

        if (deleteTerminalData()) {
            sendAuthRequest(login, token);
        } else {
            toLog(LoggerLevel::Error, "AUTH", "Ошибка базы данных при удалении данных терминала");

            msg = QVariantMap({{"message", "Ошибка базы данных"}});

            adminDialog->setDataToAdmin(AdminCommand::aCmdInfoGetServices, msg);
            adminDialog->authButtonSet(true);
        }
    } break;
    case AdminCommand::aCmdSaveUserAvtoriza: {
        auto data = adminDialog->settings;

        auto secretLogin = data.value("secret_login").toString();
        auto secretPassword = data.value("secret_password").toString();

        // Сохранение параметром
        saveAdminAuthData(true, secretLogin, secretPassword);

        // Локализуем данные
        checkUserInBase();

        // Сообщаем админке
        toLog(
            LoggerLevel::Info, "MAIN", "Данные авторизации для входа в админку успешно сохранены.");

        adminDialog->showMsgDialog("Сохранение параметров",
                                   "Данные авторизации для входа в админку успешно сохранены.");
    } break;
    default:
        break;
    }
}

void MainWindow::getBalanceAgentData(QString balance, QString overdraft) {
    QVariantMap data;
    data["balance"] = balance;
    data["overdraft"] = overdraft;
    data["threshold"] = terminalInfo["threshold"].toString();

    if (adminDialog->isVisible()) {
        adminDialog->setDataToAdmin(AdminCommand::aCmdGetBalance, data);
    }
}

void MainWindow::restartTerminalInit() {
    cmdExec = CommandInit::cRebootTerminal;
    cmdExecTimer.start(10000);
}

void MainWindow::validatorStatusGet() {
    clsValidator->execCommand(ValidatorCommands::Poll);
}

void MainWindow::validatorStatusGetOnMain() {
    auto page = mainPage->getStepByStepPage();

    if (page != PageIn::InputNumber && page != PageIn::InputSum && page != PageIn::PrintDialog) {
        // Находимся нa остальных страницах
        validatorStatusGet();
    }
}

void MainWindow::coinAcceptorStatusGet() {
    clsCoinAcceptor->execCommand(AcceptorCommands::Poll);
}

void MainWindow::coinAcceptorStatusGetOnMain() {
    auto page = mainPage->getStepByStepPage();

    if (page != PageIn::InputNumber && page != PageIn::InputSum && page != PageIn::PrintDialog) {
        // Находимся нa остальных страницах
        coinAcceptorStatusGet();
    }
}

void MainWindow::nominalGet(int nominal) {

    if (config.lockDuplicateNominal) {
        return;
    }

    // qDebug()<<"*******NOMINAL INPUT*******"<< nominal;

    toLog(LoggerLevel::Info,
          "VALIDATOR",
          QString("Вставлена купюра номиналом - %1 сом.").arg(nominal));

    if (mainPage->getStepByStepPage() == PageIn::InputSum) {
        mainPage->inputNominal(nominal);
    } else {
        toLog(LoggerLevel::Error,
              "MONEY_OUT",
              QString("Произошла купюра мимо номиналом %1 на номер %2")
                  .arg(nominal)
                  .arg(mainPage->originalNumber));
    }
}

void MainWindow::nominalDuplicateGet(int nominal) {
    toLog(LoggerLevel::Warning,
          "VALIDATOR",
          QString("Похож на дубликат номиналом %1 смн").arg(nominal));

    mainPage->loadMainPage();

    lockUnlockCenter(Lock::ErrorDublicateNominal, true);

    saveLockDuplicateNominal(true);
}

void MainWindow::coinGet(int coin) {
    if (coin == 0) {
        return;
    }

    if (coin < 100) {
        toLog(LoggerLevel::Info, "COIN_ACCEPTOR", QString("Вставлена монета - %1 дирам").arg(coin));
    } else {
        toLog(LoggerLevel::Info,
              "COIN_ACCEPTOR",
              QString("Вставлена монета - %1 сомони").arg(coin / 100));
    }

    if (mainPage->getStepByStepPage() == PageIn::InputSum) {
        mainPage->inputNominal(coin, true);
    } else {
        toLog(LoggerLevel::Error,
              "COIN_OUT",
              QString("Произошла монета мимо %1 на номер %2")
                  .arg(coin)
                  .arg(mainPage->originalNumber));
    }
}

void MainWindow::coinDuplicateGet(int coin) {
    toLog(LoggerLevel::Warning,
          "COIN_ACCEPTOR",
          QString("Похож на дубликат монеты %1 дирам").arg(coin));

    mainPage->loadMainPage();

    lockUnlockCenter(Lock::ErrorDublicateNominal, true);

    saveLockDuplicateNominal(true);
}

void MainWindow::incameStatusFromValidator(int sts, QString comment) {
    config.validatorData.state = sts;

    if (sts >= 1 && sts < 30) {
        // Записываем ошибку в лог
        toLog(LoggerLevel::Error, "VALIDATOR", comment);

        // Есть ошибка в купюроприемнике
        // Если не существует монетоприемник, то блокируем при замятие
        if (config.coinAcceptorData.state == CCtalkStatus::Errors::NotAvailable) {
            lockUnlockCenter(Lock::ErrorValidator, true);
        }

        if (sts == VStatus::Errors::BadStackerPosition) {
            lockUnlockCenter(Lock::ErrorValidator, true);
            openValidatorBox();

            auto dt = QDateTime::currentDateTime();
            int offset = dt.offsetFromUtc();
            dt.setOffsetFromUtc(offset);

            if (saveBillValidatorEvent("CASHBOX-OPENED", dt.toString(Qt::ISODateWithMs))) {
                bValidatorEventCheck();
            }
        }

        // Если есть замятие то надо оповестить
        if (sts == VStatus::Errors::ValidatorJammed || sts == VStatus::Errors::StackerJammed) {
            toLog(LoggerLevel::Error,
                  "MONEY_JAM",
                  QString("Произошло замятие купюры предположительно на номер %1")
                      .arg(mainPage->originalNumber));
        }
    } else {

        // Warnings
        if (sts >= 47 && sts <= 48) {
            toLog(LoggerLevel::Warning, "VALIDATOR", comment);
        }

        // Ошибок нет
        lockUnlockCenter(Lock::ErrorValidator, false);
    }
}

void MainWindow::incameStatusFromCoinAcceptor(int sts, QString comment) {
    config.coinAcceptorData.state = sts;

    if (sts >= 1 && sts < 35) {

        // Записываем ошибку в лог
        toLog(LoggerLevel::Error, "COIN ACCEPTOR", comment);

        // Если не существует монетоприемник, то блокировкуем при ошибках валидатора
        if (config.coinAcceptorData.state == CCtalkStatus::Errors::NotAvailable) {
            if (config.validatorData.state >= 1 && config.validatorData.state < 30) {
                lockUnlockCenter(Lock::ErrorValidator, true);
            }
        }
    }
}

void MainWindow::validatorInit(bool action) {
    if (config.validatorData.state == VStatus::Errors::NotAvailable) {
        return;
    }

    if (action) {
        loggerValidator->account = mainPage->originalNumber;

        clsValidator->maxSum = mainPage->serviceMaxSum();
        clsValidator->execCommand(ValidatorCommands::StartPolling);
    } else {
        clsValidator->execCommand(ValidatorCommands::StopPolling);
    }
}

void MainWindow::coinAcceptorInit(bool action) {
    if (config.coinAcceptorData.state == CCtalkStatus::Errors::NotAvailable) {
        return;
    }

    if (terminalInfo["coin_acceptor"].toInt() != 1) {
        return;
    }

    if (action) {
        clsCoinAcceptor->execCommand(AcceptorCommands::StartPolling);
    } else {
        clsCoinAcceptor->execCommand(AcceptorCommands::StopPolling);
    }
}

bool MainWindow::saveLockDuplicateNominal(bool lock) {
    settingsSet("lock_duplicate_nominal", lock);

    config.lockDuplicateNominal = lock;

    lockUnlockCenter(Lock::ErrorDublicateNominal, lock);

    return true;
}

void MainWindow::deviceSearchResult(
    int device, int result, QString dev_name, QString dev_comment, QString dev_port) {
    if (registrationForm) {
        registrationForm->deviceSearchResult(device, result, dev_name, dev_comment, dev_port);
        return;
    }

    switch (device) {
    case SearchDev::search_validator: {
        switch (result) {
        case SearchDev::start_search: {
            sDevicesForm->setValidatorSearchText(SearchDev::start_search,
                                                 interfaceText("validator_info_searching"));
            toLog(LoggerLevel::Info, senderName, interfaceText("validator_info_searching"));
        } break;
        case SearchDev::device_found: {
            auto inf = interfaceText("validator_info_found")
                           .replace("[v1]", QString("%1 %2").arg(dev_name, dev_comment))
                           .replace("[v2]", dev_port);
            sDevicesForm->setValidatorSearchText(SearchDev::device_found, inf);
            toLog(LoggerLevel::Info, senderName, inf);
        } break;
        case SearchDev::device_notfound: {
            sDevicesForm->setValidatorSearchText(SearchDev::device_notfound,
                                                 interfaceText("validator_info_notfound"));
            toLog(LoggerLevel::Error, senderName, interfaceText("validator_info_notfound"));
        } break;
        }
    } break;
    case SearchDev::search_coin_acceptor: {
        switch (result) {
        case SearchDev::start_search: {
            sDevicesForm->setCoinAcceptorSearchText(SearchDev::start_search,
                                                    interfaceText("coin_acceptor_info_searching"));
            toLog(LoggerLevel::Info, senderName, interfaceText("coin_acceptor_info_searching"));
        } break;
        case SearchDev::device_found: {
            auto inf = interfaceText("coin_acceptor_info_found")
                           .replace("[v1]", QString("%1 %2").arg(dev_name, dev_comment))
                           .replace("[v2]", dev_port);
            sDevicesForm->setCoinAcceptorSearchText(SearchDev::device_found, inf);
            toLog(LoggerLevel::Info, senderName, inf);
        } break;
        case SearchDev::device_notfound: {
            sDevicesForm->setCoinAcceptorSearchText(SearchDev::device_notfound,
                                                    interfaceText("coin_acceptor_info_notfound"));
            toLog(LoggerLevel::Error, senderName, interfaceText("coin_acceptor_info_notfound"));
        } break;
        }
    } break;
    case SearchDev::search_printer: {
        switch (result) {
        case SearchDev::start_search: {
            sDevicesForm->setPrinterSearchText(SearchDev::start_search,
                                               interfaceText("printer_info_searching"));
            toLog(LoggerLevel::Info, senderName, interfaceText("printer_info_searching"));
        } break;
        case SearchDev::device_found: {
            auto inf = interfaceText("printer_info_found")
                           .replace("[v1]", QString("%1 %2").arg(dev_name, dev_comment))
                           .replace("[v2]", dev_port);
            sDevicesForm->setPrinterSearchText(SearchDev::device_found, inf);
            toLog(LoggerLevel::Info, senderName, inf);
        } break;
        case SearchDev::device_notfound: {
            sDevicesForm->setPrinterSearchText(SearchDev::device_notfound,
                                               interfaceText("printer_info_notfound"));
            toLog(LoggerLevel::Error, senderName, interfaceText("printer_info_notfound_" + lang));
        } break;
        }
    } break;

    case SearchDev::search_modem: {
        switch (result) {
        case SearchDev::start_search: {
            sDevicesForm->setModemSearchText(SearchDev::start_search,
                                             interfaceText("modem_info_searching"));
            toLog(LoggerLevel::Info, senderName, interfaceText("modem_info_searching"));
        } break;
        case SearchDev::device_found: {
            auto inf = interfaceText("modem_info_found")
                           .replace("[v1]", QString("%1 %2").arg(dev_name, dev_comment))
                           .replace("[v2]", dev_port);
            sDevicesForm->setModemSearchText(SearchDev::device_found, inf);
            toLog(LoggerLevel::Info, senderName, inf);
        } break;
        case SearchDev::device_notfound: {
            sDevicesForm->setModemSearchText(SearchDev::device_notfound,
                                             interfaceText("modem_info_notfound"));
            toLog(LoggerLevel::Error, senderName, interfaceText("modem_info_notfound"));
        } break;
        }
    } break;
    case SearchDev::search_watchdog: {
        switch (result) {
        case SearchDev::start_search: {
            sDevicesForm->setWDSearchText(SearchDev::start_search,
                                          interfaceText("wd_info_searching"));
            toLog(LoggerLevel::Info, senderName, interfaceText("wd_info_searching"));
        } break;
        case SearchDev::device_found: {
            auto inf = interfaceText("wd_info_found")
                           .replace("[v1]", QString("%1 %2").arg(dev_name, dev_comment))
                           .replace("[v2]", dev_port);
            sDevicesForm->setWDSearchText(SearchDev::device_found, inf);
            toLog(LoggerLevel::Info, senderName, inf);
        } break;
        case SearchDev::device_notfound: {
            sDevicesForm->setWDSearchText(SearchDev::device_notfound,
                                          interfaceText("wd_info_notfound"));
            toLog(LoggerLevel::Error, senderName, interfaceText("wd_info_notfound"));
        } break;
        }
    } break;
    }
}

void MainWindow::deviceSearchFinished() {
    QString toLogingData = "\n\n";
    toLogingData += "- Уровень сигнала модема   = " + searchDevices->signalQuality + "%" + "\n";
    toLogingData += "- Коментарий к модему      = " + searchDevices->modemComment + "\n";
    toLogingData +=
        "- Сим карта присутствует   = " + QString("%1").arg(searchDevices->nowSimPresent) + "\n";
    toLogingData += "- Оператор сим карты       = " + searchDevices->operatorName + "\n";
    toLogingData += "- Баланс сим карты         = " + searchDevices->simBalance + "\n";
    toLogingData += "- Номер сим каты           = " + searchDevices->simNumber + "\n";

    toLog(LoggerLevel::Info, "MODEM", toLogingData);

    config.modemData.rate = searchDevices->signalQuality;
    config.modemData.comment = searchDevices->modemComment.replace("'", "");
    config.modemData.provider = searchDevices->operatorName;
    config.modemData.simPresent = searchDevices->nowSimPresent;
    config.modemData.balance = searchDevices->simBalance;
    config.modemData.number = searchDevices->simNumber;
    config.modemData.found = searchDevices->modemFound;

    config.validatorData.partNumber = searchDevices->validatorPartNum.replace("'", "");
    config.validatorData.serialNumber = searchDevices->validatorSerialNum;

    config.coinAcceptorData.partNumber = searchDevices->coinAcceptorPartNum.replace("'", "");
    config.coinAcceptorData.serialNumber = searchDevices->coinAcceptorSerialNum;

    if (registrationForm) {
        registrationForm->deviceSearchFinished();
        return;
    }

    toLog(LoggerLevel::Info, senderName, "Поиск устройств окончен");

    // Вытаскиваем данные об устройствах из базы
    getDeviceFromDB();

    // Тут надо сделать инициализацию устройств
    devicesInitialization();

    // В начале надо проверить соединение с сервером
    connectionCheck();

    // Блокируем терминал из за дубликата купюры
    if (config.lockDuplicateNominal) {
        lockUnlockCenter(Lock::ErrorDublicateNominal, true);
    }
}

void MainWindow::devicesInitialization() {
    // Присваиваем значения купюроприемнику
    clsValidator->setValidator(config.validatorData.name);
    clsValidator->setPortName(config.validatorData.port);
    clsValidator->setPartNumber(config.validatorData.partNumber);
    clsValidator->openPort();

    bool validatorFirmwareMode = config.validatorData.partNumber == "BOOTLDR";

    if (!validatorFirmwareMode) {
        // Начинаем перезагружать купюроприемник
        clsValidator->execCommand(ValidatorCommands::Restart);
    }

    // Присваиваем значения монетоприемнику
    clsCoinAcceptor->setValidator(config.coinAcceptorData.name);
    clsCoinAcceptor->setPortName(config.coinAcceptorData.port);
    clsCoinAcceptor->setPartNumber(config.coinAcceptorData.partNumber);
    clsCoinAcceptor->openPort();

    // Инициализируем монетоприемник
    clsCoinAcceptor->execCommand(AcceptorCommands::SetNominalTable);

    // Тут обявляем какую модель принитера
    clsPrinter->setPrinterModel(config.printerData.name);
    clsPrinter->setPortName(config.printerData.port);
    clsPrinter->winPrinterName = config.printerData.comment;

    clsPrinter->setChekWidth(config.printerData.checkWidth);
    clsPrinter->setSmallText(config.printerData.smallChek);
    clsPrinter->setSmallBeetwenString(config.printerData.smallBeetwenString);
    clsPrinter->setLeftMargin(config.printerData.leftMargin);
    clsPrinter->setCounterPrinterIndicator(config.existCounterPrinterChek);

    clsPrinter->WpWidth = config.winPrtChekWidth;
    clsPrinter->WpHeight = config.winPrtChekHeight;
    clsPrinter->WpFont = config.winPrtChekFontSize;
    clsPrinter->WpLeftMargin = config.winPrtChekLeftMargin;
    clsPrinter->WpRightMargin = config.winPrtChekRightMargin;
    clsPrinter->WpTopMargin = config.winPrtChekTopMargin;
    clsPrinter->WpBottomMargin = config.winPrtChekBottomMargin;

    payDaemons->printerModel = config.printerData.name;

    // Открытие порта
    clsPrinter->printerOpen();

    printText = "";

    if (!testMode) {
        // Запускаем таймер проверки статусов
        if (!lockerTimer->isActive()) {
            lockerTimer->start(5000);
        }

        // Запускаем таймер проверки состояния купюроприёмника
        if (!statusValidatorTimer->isActive() && !validatorFirmwareMode) {
            statusValidatorTimer->start(500);
        }

        // Запускаем таймер проверки состояния монетоприемника
        if (!statusCoinAcceptorTimer->isActive()) {
            statusCoinAcceptorTimer->start(500);
        }
    }

    // Незаконченная прошивка купюроприемника
    if (validatorFirmwareMode) {
        // Заблокируем интерфейс
        lockUnlockCenter(Lock::ErrorValidator, true);

        QTimer::singleShot(10000, this, [=] {
            clsValidator->firmwareVersion = "";
            clsValidator->execCommand(ValidatorCommands::FirmwareUpdate);
        });
    }
}

void MainWindow::deviceTest(int device, QString name, QString port, QString comment) {
    QString msgOk = "";
    QString msgError = "";

    switch (device) {
    case SearchDev::search_validator:
        msgOk = "Валидатор работает";
        msgError = "Валидатор отсутствует или не работает";
        break;
    case SearchDev::search_coin_acceptor:
        msgOk = "Монетоприемник работает";
        msgError = "Монетоприемник отсутствует или не работает";
        break;
    case SearchDev::search_printer:
        msgOk = "Принтер работает";
        msgError = "Принтер отсутствует или не работает";
        searchDevices->prtWinName = comment;
        break;
    case SearchDev::search_watchdog:
        msgOk = "Сторожевой таймер работает";
        msgError = "Сторожевой таймер отсутствует или не работает";
        break;
    case SearchDev::search_modem:
        msgOk = "Модем работает";
        msgError = "Модем отсутствует или порт занят";
        break;
    default:
        break;
    }

    auto receipt = receiptGet(config.tpl)
                       .arg("201801011111110000",
                            "000000",
                            "2018-01-01 11:11:11",
                            "test test",
                            "",
                            "Beeline",
                            "999999999",
                            "00",
                            "00",
                            "0",
                            "44-640-5544");

    searchDevices->receiptTest = receipt;

    if (searchDevices->searchDeviceMethod(device, name, port, comment, true)) {
        if (registrationForm) {
            registrationForm->setLoading(false);
            registrationForm->setStatusText(1, msgOk);
        }
    } else {
        if (registrationForm) {
            registrationForm->setLoading(false);
            registrationForm->setStatusText(2, msgError);
        }
    }
}

bool MainWindow::getDeviceFromDB() {
    QSqlQuery select(db);

    QString strSelect = QString("SELECT * FROM terminal_devices");

    if (!select.exec(strSelect)) {
        return false;
    }

    QSqlRecord record = select.record();

    while (select.next()) {

        int id = select.value(record.indexOf("id")).toInt();
        QString port = select.value(record.indexOf("port")).toString();
        QString name = select.value(record.indexOf("name")).toString();
        QString comment = select.value(record.indexOf("comment")).toString();
        QString state = select.value(record.indexOf("state")).toString();

        switch (id) {
        case 1: {
            config.validatorData.name = name;
            config.validatorData.port = port;
            config.validatorData.comment = comment.replace("'", "");

            if (state == "0") {
                incameStatusFromValidator(VStatus::Errors::NotAvailable, "Валидатор не найден");
            }
        } break;
        case 2: {
            config.printerData.name = name;
            config.printerData.port = port;
            config.printerData.comment = comment.replace("'", "");

            if (state == "0" || !port.contains("COM")) {
                config.printerData.state = PrinterState::PrinterNotAvailable;
            }
        } break;
        case 3: {
            config.modemData.name = name;
            config.modemData.port = port;
        } break;
        case 4: {
            config.WDData.name = name;
            config.WDData.port = port;
            config.WDData.comment = comment;
        } break;
        case 5: {
            config.coinAcceptorData.name = name;
            config.coinAcceptorData.port = port;
            config.coinAcceptorData.comment = comment.replace("'", "");

            if (state == "0") {
                this->incameStatusFromCoinAcceptor(CCtalkStatus::Errors::NotAvailable,
                                                   "Монетоприемник не найден");
            }
        } break;
        }
    }

    return true;
}

QString MainWindow::getWinprinterFromDB() {
    QSqlQuery select(db);

    QString strSelect = QString("SELECT * FROM terminal_devices WHERE id = 2");

    if (!select.exec(strSelect)) {
        return QString();
    }

    QSqlRecord record = select.record();

    QString winprinter;

    if (select.next()) {
        winprinter = select.value(record.indexOf("comment")).toString();
    }

    return winprinter;
}

void MainWindow::toPrintText(QString text) {
    // Берём текст дла отправки на печать
    printText = text;

    // Проверяем статус принтера
    clsPrinter->CMD_GetStatus();
    //    clsPrinter->CGetStatus();
}

void MainWindow::statusPrinter(int status) {
    config.printerData.state = status;

    if (status == PrinterState::PrinterOK || status & PrinterState::PaperNearEnd) {

        mainPage->printerStatus = true;

        // Тут надо отправить на печать
        if (printText != "") {
            clsPrinter->CMD_Print(printText);
        }

        printText = "";
    } else {
        mainPage->printerStatus = false;
    }

    if (getPrinterState) {
        toSendMonitoringStatus();
        getPrinterState = false;
    }
}

QString MainWindow::printerStatusList() {
    QStringList status;
    if (config.printerData.state & PrinterState::PaperNearEnd)
        status << PrinterState::Param::PaperNearEnd;
    if (config.printerData.state & PrinterState::ControlPaperEnd)
        status << PrinterState::Param::ControlPaperEnd;
    if (config.printerData.state & PrinterState::CoverIsOpened)
        status << PrinterState::Param::CoverIsOpened;
    if (config.printerData.state & PrinterState::CutterError)
        status << PrinterState::Param::CutterError;
    if (config.printerData.state & PrinterState::EKLZError)
        status << PrinterState::Param::EKLZError;
    if (config.printerData.state & PrinterState::EKLZNearEnd)
        status << PrinterState::Param::EKLZNearEnd;
    if (config.printerData.state & PrinterState::ElectronicError)
        status << PrinterState::Param::ElectronicError;
    if (config.printerData.state & PrinterState::FiscalMemoryError)
        status << PrinterState::Param::FiscalMemoryError;
    if (config.printerData.state & PrinterState::FiscalMemoryNearEnd)
        status << PrinterState::Param::FiscalMemoryNearEnd;
    if (config.printerData.state & PrinterState::MechanismPositionError)
        status << PrinterState::Param::MechanismPositionError;
    //    if(config.printerData.state & PrinterState::NoStatus)
    //        status << PrinterState::Param::NoStatus;
    if (config.printerData.state & PrinterState::PaperEnd)
        status << PrinterState::Param::PaperEnd;
    if (config.printerData.state & PrinterState::PaperJam)
        status << PrinterState::Param::PaperJam;
    if (config.printerData.state & PrinterState::PortError)
        status << PrinterState::Param::PortError;
    if (config.printerData.state & PrinterState::PowerSupplyError)
        status << PrinterState::Param::PowerSupplyError;
    if (config.printerData.state & PrinterState::PrinterError)
        status << PrinterState::Param::PrinterError;
    if (config.printerData.state & PrinterState::PrinterNotAvailable)
        status << PrinterState::Param::PrinterNotAvailable;
    if (config.printerData.state & PrinterState::PrinterOK)
        status << PrinterState::Param::PrinterOK;
    if (config.printerData.state & PrinterState::PrintingHeadError)
        status << PrinterState::Param::PrintingHeadError;
    if (config.printerData.state & PrinterState::TemperatureError)
        status << PrinterState::Param::TemperatureError;
    if (config.printerData.state & PrinterState::UnknownCommand)
        status << PrinterState::Param::UnknownCommand;
    if (config.printerData.state & PrinterState::UnknownError)
        status << PrinterState::Param::UnknownError;

    // toDebuging("status.count() - " + count);

    return status.count() > 0 ? status.at(0) : "0";
}

void MainWindow::getDataToSendStatus() {
    if (printerStatusList() != "0") {
        // Тут надо опрасить принтер статус
        getPrinterState = true;
        clsPrinter->CMD_GetStatus();
    } else {
        getPrinterState = false;
        toSendMonitoringStatus();
    }
}

void MainWindow::toSendMonitoringStatus() {
    Sender::Data sData;
    sData.lockStatus = getLock();
    sData.version = ConstData::version;
    sData.fullVersion = versionFull();
    sData.firstSend = oneSendStatus;

    // Если первый раз отправляем
    if (statusDaemons->firstSend) {
        oneSendStatus = true;
    }

    if (oneSendStatus) {
        // toDebuging("FIRST_SEND = TRUE");

        // Информация о купюрнике
        sData.validator.name = config.validatorData.name;
        config.validatorData.partNumber = config.validatorData.partNumber;

        if (config.validatorData.comment != "") {
            sData.validator.name += " " + config.validatorData.partNumber;
        }

        sData.validator.port = config.validatorData.port;
        sData.validator.serial = config.validatorData.serialNumber;

        // Информация о монетоприемнике
        sData.coinAcceptor.name = config.coinAcceptorData.name;
        config.coinAcceptorData.partNumber = config.coinAcceptorData.partNumber.replace("'", "");

        if (config.coinAcceptorData.comment != "") {
            sData.coinAcceptor.name += " " + config.coinAcceptorData.partNumber;
        }

        sData.coinAcceptor.port = config.coinAcceptorData.port;
        sData.coinAcceptor.serial = config.coinAcceptorData.serialNumber;

        // Информация о принтере
        sData.printer.name = config.printerData.name;
        config.printerData.comment = config.printerData.comment.replace("'", "");

        if (config.printerData.comment != "") {
            sData.printer.name += " " + config.printerData.comment;
        }

        sData.printer.port = config.printerData.port;

        // Информация о модеме
        sData.modem.name = config.modemData.name;
        config.modemData.comment = config.modemData.comment.replace("'", "");

        if (config.modemData.comment != "") {
            sData.modem.name += " " + config.modemData.comment;
        }

        sData.modem.port = config.modemData.port;
        sData.modem.balance = config.modemData.balance;
        sData.modem.number = config.modemData.number;
        sData.modem.provider = config.modemData.provider;
        sData.modem.signal = config.modemData.rate;
        sData.modem.comment = config.modemData.comment;

        sData.systemInfo = config.systemInfo;
        toLog(LoggerLevel::Info, senderName, "SYSINFO MONITORING");
        QJsonDocument json = QJsonDocument::fromVariant(sData.systemInfo);
        toLog(LoggerLevel::Info, senderName, QString(json.toJson(QJsonDocument::Compact)));

        //        if(config.modemData.Name != "")
        //            sData.connection = "0";

        if (config.vpnName.toUpper() != "LOCAL CONNECTION") {
            sData.connection = "0";
        } else {
            sData.connection = "1";
        }

        oneSendStatus = false;
    }

    // Если идет обновление
    if (downManager->bisyNow) {
        actionList[Action::aPoUpdateNow] = true;
    }

    // toDebuging("sData.Printer.state - " + sData.Printer.state);
    // Статус принтера
    sData.printer.allState = printerStatusList();

    // Статус купюроприёмника
    sData.validator.state = QString::number(config.validatorData.state);

    // Статус монетоприемника
    sData.coinAcceptor.state = QString::number(config.coinAcceptorData.state);

    QStringList actionLst;

    // Список действий
    for (auto &k : actionList.keys()) {
        if (actionList[k]) {
            actionLst << QString("%1").arg(k);
        }
    }

    // Проверяем не пуст ли список действий
    if (actionLst.count() > 0) {
        // toDebuging("- --  --- ACTION > 0");
        sData.actionState = true;
        sData.action = actionLst;

        // clear actions
        for (auto &k : actionList.keys()) {
            actionList[k] = false;
        }
    } else {
        sData.actionState = false;
    }

    // Берем количество денег
    auto nominalInfo = collectDaemons->getNominalInfo();

    sData.validator.billCount = nominalInfo.value("bill_count").toInt();
    sData.validator.billSum = nominalInfo.value("bill_sum").toDouble();
    sData.validator.moneyOutCount = nominalInfo.value("money_out_count").toInt();
    sData.validator.moneyOutSum = nominalInfo.value("money_out_sum").toDouble();
    sData.validator.billInfo = nominalInfo.value("bills").toString();

    sData.coinAcceptor.coinCount = nominalInfo.value("coin_count").toInt();
    sData.coinAcceptor.coinSum = nominalInfo.value("coin_sum").toInt();
    sData.coinAcceptor.coinInfo = nominalInfo.value("coins").toString();

    // Отправляем на сервер
    statusDaemons->sendStatusToServer(sData);
}

void MainWindow::openValidatorBox() {
    // toDebuging("*************INTER TO OPEN BOX****************");
    auto page = mainPage->getStepByStepPage();

    actionList[Action::aOpenValidatorBox] = true;

    // Останавливаем таймер проверки статуса валидатора
    //     statusValidator->stop();

    if (page == PageIn::InputSum) {
        // На странице приема денег

        // Останавливаем поллинг
        validatorInit(false);

        // Проверяем есть ли деньги в системе
        if (mainPage->moneyExistInPay()) {
            // Тут надо провести платеж
            mainPage->payToWhenBoxOpen();
        } else {
            mainPage->loadHtmlPage(PageIn::Main);
        }
    }

    // Переходим на страницу блокировки
    checkLockTerminal();

    // Вставляем информацию о состоянии бокса
    QString nonCollectPay = "0";
    int moneyOutCount = 0;
    double moneyOutSum = 0;
    QString c_id = "";
    QString c_trn = "";
    QString trnFrom = "";
    QString trnTo = "";
    QString htmlCenter = collectDaemons->getHtmlInfoBox(
        nonCollectPay, moneyOutCount, moneyOutSum, "", c_id, c_trn, trnFrom, trnTo);

    // Количество новых платежей
    int count_new = 0;
    payDaemons->getCountPayment(count_new);

    // Делаем небольшой html
    QString header =
        QString("<ul>"
                "<li>Не инкасированных платежей - " +
                nonCollectPay +
                "</li>"
                "<li>Новых платежей             - " +
                QString::number(count_new) +
                "</li>"
                "<li>Количество купюр мимо      - " +
                QString::number(moneyOutCount) + " на сумму - " + QString::number(moneyOutSum) +
                "</li>"
                "</ul>");

    QString allHtml = QString(header + htmlCenter);

    // Вставляем текст в форму
    incasaciyaForm->setHtmlInfoBox(allHtml);

    if (!incasaciyaForm->isVisible() && !adminDialog->isVisible()) {
        // Показываем окно инкасации терминала
        incasaciyaForm->show();
    }
}

void MainWindow::getCommandFromIncash(int cmd) {
    incasaciyaForm->close();

    switch (cmd) {
    case IncashCmd::closeThis:
        break;
    case IncashCmd::doIncash: {

        int page = ui->mainStacker->currentIndex();

        if (page == Page::LoadingMain) {

            // Делаем инкасацию
            QString text = "";

            if (collectDaemons->getCheckText(text, false, "")) {
                if (text != "") {
                    clsPrinter->CMD_Print(text);
                }
            }
        } else {
            QMessageBox msgBox;
            msgBox.setWindowTitle(windowTitle());
            msgBox.setText("Инкасация возможна после авторизации");
            msgBox.exec();
        }
    } break;
    case IncashCmd::doNullingCheck:
        break;
    case IncashCmd::interAdmin: {
        openAdminAuthDialog();
    } break;
    case IncashCmd::testPrint: {
        // Делаем инкасацию
        QString text = payDaemons->getReceiptInfo("");

        if (text != "") {
            clsPrinter->CMD_Print(text);
        }
    } break;
    }

    QCoreApplication::processEvents();
}

void MainWindow::cmdWatchdogDone(bool state, int aCommand) {
    ;
    if (aCommand == WDProtocolCommands::ResetModem) {
        QString stateNote;

        if (state) {
            stateNote = "Модем успешно перегружен";
        } else {
            stateNote = "Не удалось перезагрузить модем";
        }

        toLog(LoggerLevel::Info, "WatchDogs", stateNote);

        if (adminDialog->isVisible()) {
            auto data = QVariantMap({{"message", stateNote}});
            adminDialog->setDataToAdmin(AdminCommand::aCmdInfrmationPanel, data);
        }
    }
}

void MainWindow::setLockList() {

    lockList.clear();

    lockList[Lock::Ok].lock = false;
    lockList[Lock::Ok].comment = "Терминал работает";

    lockList[Lock::NonMoney].lock = false;
    lockList[Lock::NonMoney].comment = "Заблокирован из за недостатка средств";

    lockList[Lock::ErrorValidator].lock = false;
    lockList[Lock::ErrorValidator].comment = "Заблокирован из за ошибки валидатора";

    lockList[Lock::ErrorInterface].lock = false;
    lockList[Lock::ErrorInterface].comment = "Заблокирован из за проблемы с интерфейсом";

    lockList[Lock::LockFromServer].lock = false;
    lockList[Lock::LockFromServer].comment = "Заблокирован по сигналу с сервера";

    lockList[Lock::ErrorDevice].lock = false;
    lockList[Lock::ErrorDevice].comment = "Заблокирован по ошибке определения устройств";

    lockList[Lock::IsActiveLock].lock = false;
    lockList[Lock::IsActiveLock].comment = "Заблокирован по пораметру is_active";

    lockList[Lock::Status_11].lock = false;
    lockList[Lock::Status_11].comment = "Заблокирован по статусу терминал не активен";

    lockList[Lock::Status_12].lock = false;
    lockList[Lock::Status_12].comment = "Заблокирован по статусу агент неактивен";

    lockList[Lock::ErrorAvtorizat].lock = false;
    lockList[Lock::ErrorAvtorizat].comment = "Заблокирован из за ошибки авторизации";

    lockList[Lock::ErrorTypeTrm].lock = false;
    lockList[Lock::ErrorTypeTrm].comment = "Заблокирован по статусу неверный тип терминала";

    lockList[Lock::ErrorNoRoulPay].lock = false;
    lockList[Lock::ErrorNoRoulPay].comment = "Заблокирован по статусу нет прав на прием платежей";

    lockList[Lock::MorePayIn].lock = false;
    lockList[Lock::MorePayIn].comment = "Заблокирован по причине в системе 1 и более платежей";

    lockList[Lock::ErrorOpenAdminP].lock = false;
    lockList[Lock::ErrorOpenAdminP].comment = "Заблокирован по причине открыта админка";

    lockList[Lock::ErrorDublicateNominal].lock = false;
    lockList[Lock::ErrorDublicateNominal].comment = "Заблокирован из за дубликата купюры";

    lockList[Lock::ErrorDatabase].lock = false;
    lockList[Lock::ErrorDatabase].comment = "Заблокирован из за ошибки базы данных";
}

Lock::Data MainWindow::getLock() {
    for (auto &l : lockList.keys()) {
        if (lockList[l].lock) {
            return l;
        }
    }

    return Lock::Ok;
}

void MainWindow::getCommandFromServer(QVariantList cmdList) {

    // Идет обновление
    if (downManager->bisyNow) {
        return;
    }

    this->cmdList = cmdList;

    auto cmdCount = 0;
    QString log;

    for (auto &c : cmdList) {
        auto cmd = c.toMap();
        auto trn = cmd.value("trn").toString();
        auto cmdId = cmd.value("cmd").toInt();
        auto account = cmd.value("account").toString();
        auto comment = cmd.value("comment").toString();

        if (!commandExist(trn)) {
            log += QString("\n\t==> Команда( %1 ) принята с сервера с транзакцией - %2")
                       .arg(cmdId < lstCommandInfo.length() ? lstCommandInfo.at(cmdId)
                                                            : QString::number(cmdId))
                       .arg(trn);

            if (saveCommand(trn, cmdId, account, comment)) {
                cmdCount++;
            }
        }
    }

    if (cmdCount == 0) {
        return;
    }

    log = QString("Количество поступивших команд с сервера = %1").arg(cmdCount) + log;

    toLog(LoggerLevel::Info, "MAIN", log);

    if (!cmdTimer.isActive()) {
        cmdTimer.start();
    }
}

void MainWindow::commandCheck() {
    // Идет обновление
    if (downManager->bisyNow) {
        return;
    }

    QString trn, account, comment, status;
    int cmd = 0;

    getActiveCommand(trn, cmd, account, comment, status);

    if (trn == "") {
        cmdTimer.stop();
        return;
    }

    if (status == "new") {
        cmdTryCount = 0;

        // Меняем статус на executing
        if (commandStatusUpdate(trn, "executing")) {
            // Выполняем команду
            cmdMeta.clear();

            cmdExec = CommandInit::Cmd(cmd);
            cmdMeta["trn"] = trn;
            cmdMeta["account"] = account;
            cmdMeta["comment"] = comment;

            cmdExecTimer.start();
        }
    } else if (status == "executing" || status == "confirming") {
        cmdTryCount++;

        if (cmdTryCount >= 15) {
            cmdTryCount = 0;

            if (status == "executing") {
                commandStatusUpdate(trn, "new");
            } else if (status == "confirming") {
                commandStatusUpdate(trn, "executed");
            }
        }
        return;
    } else if (status == "executed") {
        if (commandStatusUpdate(trn, "confirming")) {
            // Отправляем запрос на подтверждение
            commandConfirm->sendCommandConfirm(trn, cmd);
        }
    }
}

bool MainWindow::getActiveCommand(
    QString &trn, int &cmd, QString &account, QString &comment, QString &status) {
    QSqlQuery sqlQuery(db);

    QString strQuery;

    strQuery = QString("SELECT * FROM terminal_commands WHERE status "
                       "!='confirmed' ORDER by trn ASC LIMIT 1;");

    if (!sqlQuery.exec(strQuery)) {
        qDebug() << sqlQuery.lastError().text();
        return false;
    }

    QSqlRecord sqlRecord = sqlQuery.record();

    if (sqlQuery.next()) {
        trn = sqlQuery.value(sqlRecord.indexOf("trn")).toString();
        cmd = sqlQuery.value(sqlRecord.indexOf("cmd")).toInt();
        account = sqlQuery.value(sqlRecord.indexOf("account")).toString();
        comment = sqlQuery.value(sqlRecord.indexOf("comment")).toString();
        status = sqlQuery.value(sqlRecord.indexOf("status")).toString();
    }

    return true;
}

void MainWindow::commandConfirmed(QString trn) {
    commandStatusUpdate(trn, "confirmed");
}

bool MainWindow::commandStatusUpdate(QString trn, QString status) {
    QSqlQuery updateCollect(db);
    QString strUpdateCollect;
    strUpdateCollect =
        QString("UPDATE terminal_commands SET status='%2' WHERE trn = '%1'").arg(trn, status);

    if (!updateCollect.exec(strUpdateCollect)) {
        return false;
    }

    return true;
}

void MainWindow::commandExecute() {
    // Смотрим на какой странице
    int pageIn = mainPage->getStepByStepPage();

    if (pageIn == PageIn::Main || pageIn == PageIn::LockTerminal) {
        commandExecution(cmdExec, cmdMeta);
        cmdMeta.clear();
    }
}

void MainWindow::commandExecution(CommandInit::Cmd cmd, QVariantMap meta) {
    cmdExecTimer.stop();

    auto trn = meta.value("trn", "").toString();
    auto account = meta.value("account").toString();
    auto comment = meta.value("comment").toString();

    switch (cmd) {
    case CommandInit::cRebootTerminal: {
        // Команда на перезагрузку терминала
        /// Перезагрузка по ошибке соединения
        if (whenRasReboot) {
            auto rCount = rebootCount();
            toLog(LoggerLevel::Info,
                  "MAIN",
                  QString("Ставим параметр счетчика перезагрузки по ошибке 756 равным - %1")
                      .arg(rCount + 1));
            rebootCountSet(rCount + 1);
            whenRasReboot = false;
        }

        toLog(LoggerLevel::Info, "MAIN", "Начинаем перезагружать ASO");

        if (trn != "") {
            commandStatusUpdate(trn, "executed");
        }

        getCommandFromAdmin(AdminCommand::aCmdRestartASO);
        return;
    } break;
    case CommandInit::cTurnOffTerminal: {
        // Команда на выключение терминала
        toLog(LoggerLevel::Info, "MAIN", "Выключение ASO");

        if (trn != "") {
            commandStatusUpdate(trn, "executed");
        }

        QProcess proc;
#ifdef Q_OS_WIN32
        proc.startDetached("c:/windows/system32/cmd.exe",
                           QStringList() << "/c" << "shutdown -s -t 0");
#endif // Q_OS_WIN32
        return;
    } break;
    case CommandInit::cUpdatePO: {
        // Команда на обновление ПО
        toLog(LoggerLevel::Info, "MAIN", "Выполняем команду на получение обновлений...");
    } break;
    case CommandInit::cTurnOnAutoApdate:
        // Команда на включение автообновления
        toLog(LoggerLevel::Info, "MAIN", "Выполняем команду включения автообновления.");
        break;
    case CommandInit::cTurnOffAutoApdate:
        // Команда на выключение автообновления
        toLog(LoggerLevel::Info, "MAIN", "Выполняем команду выключение автообновления.");
        break;
    case CommandInit::cSendLogInfo: {
        // Команда на загрузку лога
        toLog(LoggerLevel::Info,
              "MAIN",
              QString("Выполняем команду отправки лога за %1 число.").arg(comment));

        sendLogInfo->sendLogInfoToServer(trn, comment);
    } break;
    case CommandInit::cRestartValidator: {
        // Команда на перезагрузку купюроприемника
        toLog(LoggerLevel::Info, "MAIN", "Выполняем команду перезагрузка валидатора.");
        clsValidator->execCommand(ValidatorCommands::Restart);
    } break;
    case CommandInit::cGetIncashment: {
        QString log =
            comment.trimmed().isEmpty()
                ? "Выполняем команду отправки отложенной инкасации."
                : QString("Выполняем команду отправки отложенной инкасации с cid: %1").arg(comment);

        toLog(LoggerLevel::Info, "MAIN", log);

        QString text;
        QString cid = comment;

        collectDaemons->getCheckText(text, false, "", cid);
    } break;
    case CommandInit::cSendLogValidator: {
        toLog(LoggerLevel::Info,
              "MAIN",
              QString("Выполняем команду отправки лога валидатора за %1 число и "
                      "аккаунт %2.")
                  .arg(comment, account));
        sendLogInfo->sendLogValidatorToServer(trn, comment, account);
    } break;
    case CommandInit::cBVFirmwareUpdate: {
        toLog(LoggerLevel::Info,
              "MAIN",
              QString("Принята команда обновление прошивки купюроприемника на версию %1")
                  .arg(comment));

        statusValidatorTimer->stop();

        clsValidator->firmwareVersion = comment;
        clsValidator->execCommand(ValidatorCommands::FirmwareUpdate);
    }
    default:
        break;
    }

    if (trn != "") {
        commandStatusUpdate(trn, "executed");
    }
}

void MainWindow::validatorFirmwareResult(QString state) {
    if (state == "cancel") {
        lockUnlockCenter(Lock::ErrorValidator, false);

        if (!statusValidatorTimer->isActive()) {
            statusValidatorTimer->start(500);
        }
    }

    if (state == "start") {
        // Заблокируем интерфейс
        lockUnlockCenter(Lock::ErrorValidator, true);
    }

    if (state == "error") {
        // Перезагружаем программу
        getCommandFromAdmin(AdminCommand::aCmdRestartApp);
    }

    if (state == "success") {
        // Перезагружаем программу
        getCommandFromAdmin(AdminCommand::aCmdRestartApp);
    }
}

void MainWindow::checkHash(QString hash) {
    if (terminalInfo.value("hash").toString() != hash) {
        // Hash не одинаковый надо качать get_services заново
        toLog(LoggerLevel::Info,
              "MAIN",
              "Hash GET_SERVICES изменился начинаем запрашивать конфигурацию.");

        getServicesRequest();
    }
}

void MainWindow::checkUpdateHash(QString hash, QString path) {
    if (hash.trimmed() == "" || path.trimmed() == "") {
        return;
    }

    QString xmlName = QString("aso_%1.xml").arg(path);
    // Даем наименование папки закачки и xml наименование
    downManager->setUpdatePointName(path);
    downManager->setXmlFileName(xmlName);

    bool check = downManager->checkHashMonitor(hash);

    if (check && config.autoUpdateStatus) {
        if (!downManager->bisyNow) {
            toLog(LoggerLevel::Info, "MAIN", "Hash Upadte-xml изменился...");

            // Тут надо запустить updater
            actionList[Action::aPoUpdateNow] = true;

            downManager->startToUpdate();
        }
    }
}

void MainWindow::nonSendPaymentLock(bool lock) {
    lockUnlockCenter(Lock::MorePayIn, lock);
}

void MainWindow::errorDBLock() {
    lockUnlockCenter(Lock::ErrorDatabase, true);

    int pageIn = this->mainPage->getStepByStepPage();

    if (pageIn == PageIn::InputSum) {
        clsValidator->setDBError(true);
    }
}

void MainWindow::avtorizationLockUnlock(bool lock, int sts) {
    if (sts) {
        // Есть какойто статус
        Lock::Data lockType = Lock::Ok;

        switch (sts) {
        case 11:
            lockType = Lock::Status_11;
            break;
        case 12:
            lockType = Lock::Status_12;
            break;
        case 14:
            lockType = Lock::ErrorTypeUser;
            break;
        case 150:
            lockType = Lock::ErrorAvtorizat;
            break;
        case 151:
            lockType = Lock::ErrorAvtorizat;
            break;
        case 245:
            lockType = Lock::ErrorTypeTrm;
            break;
        case 133:
            lockType = Lock::ErrorNoRoulPay;
            break;
        }

        if (lockType != Lock::Ok) {
            lockUnlockCenter(lockType, lock);
        }
    } else {
        // Статус ок
        lockUnlockCenter(Lock::Status_11, false);
        lockUnlockCenter(Lock::Status_12, false);
        lockUnlockCenter(Lock::ErrorTypeUser, false);
        lockUnlockCenter(Lock::ErrorAvtorizat, false);
        lockUnlockCenter(Lock::ErrorTypeTrm, false);
        lockUnlockCenter(Lock::ErrorNoRoulPay, false);
    }
}

void MainWindow::isActiveLock(bool active) {
    lockUnlockCenter(Lock::IsActiveLock, !active);
}

void MainWindow::getBalanceUser(double balance, double overdraft, double threshold) {

    if (threshold == 1.111) {
        threshold = terminalInfo["threshold"].toDouble();
    } else {
        terminalInfo["threshold"] = QString::number(threshold);
    }

    terminalInfo["overdraft"] = QString::number(overdraft);
    terminalInfo["balance"] = QString::number(balance);

    double ff = balance - threshold;

    if (ff < overdraft) {
        lockUnlockCenter(Lock::NonMoney, true);
    } else {
        lockUnlockCenter(Lock::NonMoney, false);
    }
}

void MainWindow::getBanners(QVariantList banners) {
    mainPage->banners = banners;
}

void MainWindow::getTerminalInfo(QVariantMap map) {
    terminalInfo.clear();
    terminalInfo = map;

    terminalInfo["terminal_num"] = config.terminalData.login;
    terminalInfo["default_lang"] = config.langDefault;

    mainPage->terminalInfo = terminalInfo;

    payDaemons->oraganization = terminalInfo["address"].toString();
    payDaemons->kassir = terminalInfo["name_agent"].toString();
    payDaemons->rma = terminalInfo["inn_agent"].toString();
    payDaemons->phone = terminalInfo["phone_agent"].toString();

    applyAuthToModules();

    clsPrinter->setFirmPatern(terminalInfo["address"].toString());

    QString xmlName = QString("aso_%1.xml").arg(terminalInfo["path_name"].toString());

    // Даем наименование папки закачки и xml наименование
    downManager->setUpdatePointName(terminalInfo["path_name"].toString());
    downManager->setXmlFileName(xmlName);
}

void MainWindow::applyAuthToModules() {
    auto token = config.terminalData.token;
    auto uuid = systemHashGet();
    auto login = config.terminalData.login;
    auto version = ConstData::version;

    payDaemons->setAuthData(token, uuid, version);
    statusDaemons->setAuthData(token, uuid, version);
    collectDaemons->setAuthData(token, uuid, version);
    userDaemons->setAuthData(token, uuid, version);
    checkOnline->setAuthData(token, uuid, version);
    sendReceipt->setAuthData(token, uuid, version);
    commandConfirm->setAuthData(token, uuid, version);
    sendLogInfo->setAuthData(token, uuid, version);
    getBalanceAgent->setAuthData(token, uuid, version);
    sendOtp->setAuthData(token, uuid, version);
    confirmOtp->setAuthData(token, uuid, version);
    jsonRequest->setAuthData(token, uuid);

    payDaemons->numTrm = login;
    payDaemons->startTimer(30);
    statusDaemons->startTimer(300);
}

void MainWindow::jsonResponseSuccess(QVariantMap response, QString requestName) {
    if (requestName == "cash-box") {
        // Update
        if (updateBillValidatorEvent("confirmed")) {
            // Check next
            bValidatorEventCheck();
        }
        return;
    }

    mainPage->jsonResponseSuccess(response, requestName);
}

void MainWindow::jsonResponseError(QString error, QString requestName) {
    if (requestName == "cash-box") {
        if (error == "timeout") {
            bValidatorEventCheck();
        }
        return;
    }

    mainPage->jsonResponseError(error, requestName);
}

void MainWindow::setTerminalInfo(QString data) {
    mainPage->setTerminalInfo(data);
}

void MainWindow::unlockAdminOpenSts() {
    lockUnlockCenter(Lock::ErrorOpenAdminP, false);
}

void MainWindow::checkLockTerminal() {
    auto lock = getLock();
    auto pageIn = mainPage->getStepByStepPage();

    // toDebuging("############### -- STATUS -- ############### - " + status);

    if (lock == Lock::Ok) {
        // Нет блокировок

        // Смотрим есть ли блокировка
        if (pageIn == PageIn::LockTerminal) {
            toLog(LoggerLevel::Info, senderName, QString("Разблокируем терминал, нет ошибок."));
            mainPage->loadHtmlPage(PageIn::Main);
        }
    } else {
        // Есть какая то блокировка

        // Смотрим не заблокированы ли мы и заблокировать если находится на странице
        // кроме ввода денег, печати чека
        if (pageIn != PageIn::LockTerminal && pageIn != PageIn::PrintDialog &&
            pageIn != PageIn::InputSum) {
            toLog(LoggerLevel::Error, senderName, QString("%1").arg(lockList[lock].comment));
            mainPage->lockReason = lockList[lock].comment;
            mainPage->loadHtmlPage(PageIn::LockTerminal);
        }
    }
}

void MainWindow::lockUnlockCenter(Lock::Data state, bool lock) {
    bool smsSend = false;

    if (lock) {
        textSms = "";

        // Блокировка
        switch (state) {
        case Lock::ErrorValidator: {
            // Тут проверям сначала включена ли опция отправки смс по ошибке
            // купюроприёмника, кроме открытия купюриника, и разблочена ли система
            if (config.smsErrValidator && !lockList[state].lock) {
                if (config.validatorData.state == VStatus::Errors::BadStackerPosition) {
                    smsSend = true;
                    textSms = config.terminalData.login + "-" +
                              "Терминал заблокирован. Открыта касета купюроприемника.";
                }

                if (config.validatorData.state == VStatus::Errors::ValidatorJammed) {
                    smsSend = true;
                    textSms = config.terminalData.login + "-" +
                              "Терминал заблокирован. Замятие купюры в купюроприемнике.";
                }

                if (config.validatorData.state == VStatus::Errors::StackerJammed) {
                    smsSend = true;
                    textSms = config.terminalData.login + "-" +
                              "Терминал заблокирован. Замятие купюры в боксе.";
                }

                if (config.validatorData.state == VStatus::Errors::StackerJammed) {
                    smsSend = true;
                    textSms = config.terminalData.login + "-" +
                              "Терминал заблокирован. Переполнение стекера.";
                }

                if (smsSend) {
                    toLog(LoggerLevel::Info,
                          "SMS_CENTER",
                          QString("Необходимо отправить СМС по ошибке купюроприемника."));
                }
            }
        } break;
        case Lock::NonMoney: {
            if (config.smsErrValidator && !lockList[state].lock) {

                smsSend = true;
                textSms = config.terminalData.login + "-" +
                          "Терминал заблокирован из за недостатка средств";

                toLog(LoggerLevel::Info,
                      "SMS_CENTER",
                      QString("Необходимо отправить СМС по недостатки средств у агента."));
            }
        } break;
        case Lock::MorePayIn: {
            if (config.smsErrValidator && !lockList[state].lock) {
                smsSend = true;
                textSms = config.terminalData.login + "-" +
                          "Терминал заблокирован по причине в системе 1 и более платежей";
                toLog(LoggerLevel::Info,
                      "SMS_CENTER",
                      QString("Необходимо отправить СМС по причине в системе 1 и более "
                              "платежей."));
            }
        } break;
        case Lock::Status_11: {
            if (config.smsErrValidator && !lockList[state].lock) {
                smsSend = true;
                textSms = config.terminalData.login + "-" +
                          "Терминал заблокирован по статусу Терминал не активен";
                toLog(LoggerLevel::Info,
                      "SMS_CENTER",
                      QString("Необходимо отправить СМС по статусу Терминал не активен(11)"));
            }
        } break;

        default:
            break;
        }

        lockList[state].lock = true;
    } else {
        // Разблокировка
        lockList[state].lock = false;
    }

    // Запускаем таймер отправки смс
    if (smsSend) {
        QTimer::singleShot(20000, this, SLOT(checkToSendSms()));
    }
}

void MainWindow::gotoPage(Page page) {
    switch (page) {
    case Page::LoadingDevices: {
        // Переход на страницу загрузки устройств
        ui->mainStacker->setCurrentIndex(Page::LoadingDevices);
    } break;
    case Page::LoadingGprs: {
        // Переход на страницу загрузки соединения
        ui->mainStacker->setCurrentIndex(Page::LoadingGprs);
    } break;
    case Page::LoadingMain: {
        // Переход на страницу интерфейса
        ui->mainStacker->setCurrentIndex(Page::LoadingMain);
        mainPage->loadHtmlPage(PageIn::Main);
    } break;
    }
}

void MainWindow::getSmsSendStatus(bool state, QStringList lstId) {
    Q_UNUSED(lstId)

    if (state) {
        // СМС Успешно отправлено
        toLog(LoggerLevel::Info, "SMS_CENTER", "SMS успешно отправлено...");
    }

    // Ставим параметр что соединения свободно
    connObject->conState = Connection::conStateDown;

    // Подымаем соединение если оно модемное
    if (config.vpnName.toUpper() != "LOCAL CONNECTION") {
        QTimer::singleShot(5000, this, SLOT(startToConnection()));
    }
}

QVariantList MainWindow::getServicesInputsFromDB() {
    QSqlQuery querySql(db);

    QString sqlQuery = "SELECT * FROM terminal_services_inputs si LEFT JOIN "
                       "terminal_inputs i ON si.input_id = i.id "
                       "WHERE si.input_id > 0 GROUP BY si.input_id, "
                       "si.service_id ORDER BY si.que;";

    if (querySql.exec(sqlQuery)) {
        QSqlRecord record = querySql.record();
        QVariantList inputs;

        while (querySql.next()) {
            QVariantMap input;
            input["id"] = querySql.value(record.indexOf("id"));
            input["service_id"] = querySql.value(record.indexOf("service_id"));
            input["field"] = querySql.value(record.indexOf("field"));
            input["field_type"] = querySql.value(record.indexOf("field_type"));
            input["prefix"] = querySql.value(record.indexOf("prefix"));
            input["que"] = querySql.value(record.indexOf("que"));
            input["input_panel"] = querySql.value(record.indexOf("input_panel"));
            input["regexp"] = querySql.value(record.indexOf("regexp"));
            input["mask"] = querySql.value(record.indexOf("mask"));
            input["help_local"] = querySql.value(record.indexOf("help_local"));
            input["help_secondary"] = querySql.value(record.indexOf("help_secondary"));
            input["help_ru"] = querySql.value(record.indexOf("help_ru"));
            input["help_en"] = querySql.value(record.indexOf("help_en"));
            input["placeholder_local"] = querySql.value(record.indexOf("placeholder_local"));
            input["placeholder_secondary"] =
                querySql.value(record.indexOf("placeholder_secondary"));
            input["placeholder_ru"] = querySql.value(record.indexOf("placeholder_ru"));
            input["placeholder_en"] = querySql.value(record.indexOf("placeholder_en"));

            inputs.append(input);
        }

        return inputs;
    }

    return QVariantList();
}

QVariantList MainWindow::getServicesFieldsFromDB() {
    QSqlQuery querySql(db);

    QString sqlQuery = "SELECT * FROM terminal_services_inputs si WHERE "
                       "si.input_id == 0 ORDER BY si.que;";

    if (querySql.exec(sqlQuery)) {
        QSqlRecord record = querySql.record();
        QVariantList fields;

        while (querySql.next()) {
            QVariantMap field;
            field["service_id"] = querySql.value(record.indexOf("service_id"));
            field["field"] = querySql.value(record.indexOf("field"));
            field["field_type"] = querySql.value(record.indexOf("field_type"));
            field["prefix"] = querySql.value(record.indexOf("prefix"));
            field["que"] = querySql.value(record.indexOf("que"));

            fields.append(field);
        }

        return fields;
    }

    return QVariantList();
}

QVariantList MainWindow::getServicesFromDB() {
    QSqlQuery querySql(db);

    QVariantList servicesInputs = getServicesInputsFromDB();
    QVariantList servicesFields = getServicesFieldsFromDB();

    QString sqlQuery = "SELECT * FROM terminal_services ORDER BY que ASC;";

    if (querySql.exec(sqlQuery)) {
        QSqlRecord record = querySql.record();
        QVariantList services;

        while (querySql.next()) {
            QVariantMap service;
            service["id"] = querySql.value(record.indexOf("services_id"));
            service["enable"] = querySql.value(record.indexOf("services_nbl")) == 1;
            service["otp"] = querySql.value(record.indexOf("services_otp")) == 1;
            service["commission_id"] = querySql.value(record.indexOf("services_cid"));
            service["commission"] = querySql.value(record.indexOf("services_cms"));
            service["category_id"] = querySql.value(record.indexOf("category_id"));
            service["amount_min"] = querySql.value(record.indexOf("amount_min"));
            service["amount_max"] = querySql.value(record.indexOf("amount_max"));
            service["name_local"] = querySql.value(record.indexOf("name_local"));
            service["name_secondary"] = querySql.value(record.indexOf("name_secondary"));
            service["name_ru"] = querySql.value(record.indexOf("name_ru"));
            service["name_en"] = querySql.value(record.indexOf("name_en"));
            service["precheck"] = querySql.value(record.indexOf("precheck")) == 1;
            service["autosum"] = querySql.value(record.indexOf("autosum")) == 1;
            service["presum"] = querySql.value(record.indexOf("presum")) == 1;
            service["sms_code"] = querySql.value(record.indexOf("sms_code")) == 1;
            service["cms_add"] = querySql.value(record.indexOf("cms_add")) == 1;
            service["regions"] = querySql.value(record.indexOf("regions")) == 1;
            service["favorite"] = querySql.value(record.indexOf("favorite"));
            service["cms_warn"] = querySql.value(record.indexOf("cms_warn")) == 1;
            service["return_max"] = querySql.value(record.indexOf("return_max")) == 1;

            QVariantList inputs;

            for (auto &si : servicesInputs) {
                auto input = si.toMap();
                if (input.value("service_id") == service["id"]) {
                    inputs.append(input);
                }
            }

            service["inputs"] = inputs;

            QVariantList fields;

            for (auto &sf : servicesFields) {
                auto field = sf.toMap();
                if (field.value("service_id") == service["id"]) {
                    fields.append(field);
                }
            }

            service["fields"] = fields;

            services.append(service);
        }

        return services;
    }

    return QVariantList();
}

QVariantList MainWindow::getCategoriesFromDB() {
    QSqlQuery querySql(db);

    QString sqlQuery = "SELECT * FROM terminal_categories ORDER BY que ASC;";

    if (querySql.exec(sqlQuery)) {
        QSqlRecord record = querySql.record();
        QVariantList categories;

        while (querySql.next()) {
            QVariantMap category;
            category["id"] = querySql.value(record.indexOf("id"));
            category["name_local"] = querySql.value(record.indexOf("name_local"));
            category["name_secondary"] = querySql.value(record.indexOf("name_secondary"));
            category["name_ru"] = querySql.value(record.indexOf("name_ru"));
            category["name_en"] = querySql.value(record.indexOf("name_en"));
            category["description_local"] = querySql.value(record.indexOf("description_local"));
            category["description_ru"] = querySql.value(record.indexOf("description_ru"));
            category["description_en"] = querySql.value(record.indexOf("description_en"));

            categories.append(category);
        }

        return categories;
    }

    return QVariantList();
}

bool MainWindow::updateHashConfig(QString hash) {
    QSqlQuery sqlQuery(db);
    QString strUpdate = QString("UPDATE terminal_extra SET hash = \"%1\" WHERE id = 1").arg(hash);

    if (!sqlQuery.exec(strUpdate)) {
        return false;
    }

    return true;
}

void MainWindow::createSmsSendTable() {
    // Создаем таблицу смс оповещения если нету
    QSqlQuery createTable(db);
    QString strCreate;

    strCreate = QString("CREATE TABLE IF NOT EXISTS terminal_sms (sms_id "
                        "NUMERIC, sms_state NUMERIC, sms_lock_status NUMERIC);");

    if (!createTable.exec(strCreate)) {
        toLog(
            LoggerLevel::Info, "DB_CONNECT", QString("Не удалось создать таблицу terminal_sms..."));
    }
}

int MainWindow::terminalSmsCount() {
    QSqlQuery selectDevices(db);
    QString strSelect;

    strSelect = QString("SELECT count(*) AS count FROM terminal_sms");

    if (!selectDevices.exec(strSelect)) {
        return 0;
    }

    QSqlRecord record = selectDevices.record();

    int count = 0;

    if (selectDevices.next()) {
        count = selectDevices.value(record.indexOf("count")).toInt();
    }

    return count;
}

void MainWindow::insertSmsContentInf() {
    QSqlQuery userSql(db);
    QString userQuery;

    userQuery =
        QString("INSERT INTO terminal_sms(sms_id, sms_state, sms_lock_status)"
                " VALUES(%1, " +
                QString::number(SmsSend::OK) + ", " + QString::number(SmsSend::Unlock) + ");");

    for (int i = 1; i <= 6; i++) {
        QString vrmQuery = userQuery.arg(i);
        if (!userSql.exec(vrmQuery)) {
            toLog(LoggerLevel::Error, "DB_CONNECT", "Ошибка при вставке смс контента...");
            return;
        }
    }
}

int MainWindow::rebootCount() {
    QSettings settings(settingsPath(), QSettings::IniFormat);
    return settings.value("reboot_count", 99).toInt();
}

void MainWindow::rebootCountSet(int val) {
    QSettings settings(settingsPath(), QSettings::IniFormat);
    auto rebootCount = settings.value("reboot_count");

    settings.setValue("reboot_count", rebootCount.isNull() ? 0 : val);
}

void MainWindow::rebootCountClear() {
    QSettings settings(settingsPath(), QSettings::IniFormat);
    settings.setValue("reboot_count", 0);

    // Если необходима перзагрузка
    if (afterRestartRas) {
        // Тут кидаем на ошибку соединения с кодом 756
        connectionError("756", "Предыдущие попытки перезагрузки не помогли... повторим ещё раз.");
    }
}

QStringList MainWindow::portList() {
    QStringList list;

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        list << serialPortInfo.portName();
    }

    return list;
}

void MainWindow::saveDevice(
    int deviceId, QString deviceName, QString port, QString comment, int state) {
    QSqlQuery updateDevices(db);
    QString strUpdate;

    strUpdate =
        QString("UPDATE terminal_devices SET name = \"%1\", port = "
                "\"%2\", comment = \"%3\", state = %4 WHERE id = %5")
            .arg(deviceName, port, comment, QString::number(state), QString::number(deviceId));

    updateDevices.exec(strUpdate);
}

bool MainWindow::commandExist(QString trn) {
    QSqlQuery sqlQuery(db);

    QString strQuery =
        QString("SELECT count(*) AS count FROM terminal_commands WHERE trn = '%1';").arg(trn);

    if (!sqlQuery.exec(strQuery)) {
        return false;
    }

    QSqlRecord recordCollect = sqlQuery.record();

    int count = 0;

    if (sqlQuery.next()) {
        count = sqlQuery.value(recordCollect.indexOf("count")).toInt();
    }

    return count > 0;
}

bool MainWindow::saveCommand(const QString trn,
                             const int cmd,
                             const QString account,
                             const QString comment) {
    QSqlQuery sqlQuery(db);

    QString strQuery = QString("INSERT INTO terminal_commands(trn, cmd, account, comment, status)"
                               " VALUES('%1', %2, '%3', '%4', 'new');")
                           .arg(trn)
                           .arg(cmd)
                           .arg(account, comment);

    if (!sqlQuery.exec(strQuery)) {
        qDebug() << sqlQuery.lastError().text();
        return false;
    }

    return true;
}

bool MainWindow::saveBillValidatorEvent(const QString event, const QString dateTime) {
    QSqlQuery sqlQuery(db);

    QString strQuery = QString("INSERT INTO terminal_bvalidator(event, date_time, status)"
                               " VALUES('%1', '%2', 'new');")
                           .arg(event, dateTime);

    if (!sqlQuery.exec(strQuery)) {
        qDebug() << sqlQuery.lastError().text();
        return false;
    }

    return true;
}

bool MainWindow::getBillValidatorEvent(QString &dateTime, QString event, QString status) {
    QSqlQuery sqlQuery(db);

    QString strQuery;

    strQuery = QString("SELECT * FROM terminal_bvalidator WHERE event = '%1' AND "
                       "status = '%2' ORDER by date_time ASC LIMIT 1;")
                   .arg(event, status);

    if (!sqlQuery.exec(strQuery)) {
        qDebug() << sqlQuery.lastError().text();
        return false;
    }

    QSqlRecord sqlRecord = sqlQuery.record();

    if (sqlQuery.next()) {
        dateTime = sqlQuery.value(sqlRecord.indexOf("date_time")).toString();
    }

    return true;
}

bool MainWindow::updateBillValidatorEvent(QString status) {

    QString dateTime = "";

    if (getBillValidatorEvent(dateTime, "CASHBOX-OPENED", "new")) {

        if (dateTime != "") {
            QSqlQuery updateCollect(db);
            QString strUpdateCollect = QString("UPDATE terminal_bvalidator SET "
                                               "status='%2' WHERE date_time = '%1'")
                                           .arg(dateTime, status);

            if (!updateCollect.exec(strUpdateCollect)) {
                return false;
            }

            return true;
        }
    }

    return false;
}

void MainWindow::bValidatorEventCheck() {

    QString dateTime = "";

    getBillValidatorEvent(dateTime, "CASHBOX-OPENED", "new");

    if (dateTime == "") {
        return;
    }

    QJsonObject json;

    json["terminal_num"] = terminalInfo["terminal_num"].toInt();
    json["moved_at"] = dateTime;

    jsonRequest->sendRequest(json, "orzu/cash-box", "cash-box", 1, 30);
}

bool MainWindow::clearDataBase() {

    QDate dateTo = QDate::currentDate();
    dateTo = dateTo.addDays(-20);
    QString vrmDateTo = dateTo.toString("yyyy-MM-dd");
    QString vrmDateFrom = QString("%1-01-01").arg(QDate::currentDate().year() - 1);

    QString lstSql1;
    QString lstSql2;
    QString lstSql3;
    QString lstSql4;

    lstSql1 = QString("DELETE FROM terminal_operation"
                      " WHERE operation_is_send = 1"
                      " AND (operation_date_create BETWEEN \"%1 00:00:00\" AND "
                      "\"%2 23:59:59\")"
                      " AND operation_collect_id in (SELECT c.collect_id from "
                      "terminal_collect as c WHERE c.collect_id "
                      "= terminal_operation.operation_collect_id AND c.status = "
                      "\"confirmed\");")
                  .arg(vrmDateFrom, vrmDateTo);

    lstSql2 = QString("DELETE FROM terminal_collect WHERE status = 'confirmed'"
                      " AND date_create BETWEEN \"%1 00:00:00\" AND \"%2 23:59:59\";")
                  .arg(vrmDateFrom, vrmDateTo);

    lstSql3 = QString("DELETE FROM terminal_commands WHERE status = 'confirmed';");

    lstSql4 = QString("DELETE FROM terminal_bvalidator WHERE status = 'confirmed';");

    QSqlQuery updateSql(db);

    if (!updateSql.exec(lstSql1)) {
        toLog(LoggerLevel::Error,
              "DB",
              QString("==> Error DELETE DATA SQL FROM DATABASE (%1)").arg(lstSql1));
        return false;
    }

    if (!updateSql.exec(lstSql2)) {
        toLog(LoggerLevel::Error,
              "DB",
              QString("==> Error DELETE DATA SQL FROM DATABASE (%1)").arg(lstSql2));
        return false;
    }

    if (!updateSql.exec(lstSql3)) {
        toLog(LoggerLevel::Error,
              "DB",
              QString("==> Error DELETE DATA SQL FROM DATABASE (%1)").arg(lstSql3));
        return false;
    }

    if (!updateSql.exec(lstSql4)) {
        toLog(LoggerLevel::Error,
              "DB",
              QString("==> Error DELETE DATA SQL FROM DATABASE (%1)").arg(lstSql4));
        return false;
    }

    if (config.tpl == "tjk") {
        updateSql.exec(QString("UPDATE terminal_operation SET extra_info=''"));
    }

    return true;
}

bool MainWindow::deleteTerminalData() {
    QSqlQuery deleteQuery(db);
    QString strDeleteQuery;

    strDeleteQuery = QString("DELETE FROM terminal_data;");

    // this->toDebuging( strDeleteQuery);
    if (!deleteQuery.exec(strDeleteQuery)) {
        // this->toDebuging(deleteQuery.lastError());
        // this->toDebuging("Error DELETE FROM terminal_user");
        return false;
    }

    return true;
}

void MainWindow::settingsSave() {
    QSettings settings(settingsPath(), QSettings::IniFormat);

    settings.setValue("check_balance_sim", config.checkGetBalanceSim);
    settings.setValue("check_number_sim", config.checkGetNumberSim);
    settings.setValue("vpn_point", config.vpnName);
    settings.setValue("ussd_balance_sim", config.simBalanceRequest);
    settings.setValue("ussd_number_sim", config.simNumberRequest);
    settings.setValue("index_check_balance", config.indexCheckBalance);
    settings.setValue("show_print_dialog", config.showPrintDialog);

    settings.setValue("chek_width", config.printerData.checkWidth);
    settings.setValue("chek_left_size", config.printerData.leftMargin);
    settings.setValue("chek_small_text", config.printerData.smallChek);
    settings.setValue("printing_chek", config.printerData.printCheckUntil);
    settings.setValue("chek_small_beetwen_string", config.printerData.smallBeetwenString);

    settings.setValue("ras_error_interval_reboot", config.timerRasReb);
    settings.setValue("default_lang", config.langDefault);

    settings.setValue("search_validator", config.searchValidator);
    settings.setValue("search_coin_acceptor", config.searchCoinAcceptor);
    settings.setValue("search_printer", config.searchPrinter);
    settings.setValue("search_modem", config.searchModem);
    settings.setValue("search_watchdog", config.searchWD);

    settings.setValue("prt_win_width", config.winPrtChekWidth);
    settings.setValue("prt_win_height", config.winPrtChekHeight);
    settings.setValue("prt_win_font_size", config.winPrtChekFontSize);
    settings.setValue("prt_win_left_margin", config.winPrtChekLeftMargin);
    settings.setValue("prt_win_right_margin", config.winPrtChekRightMargin);
    settings.setValue("prt_win_top_margin", config.winPrtChekTopMargin);
    settings.setValue("prt_win_bottom_margin", config.winPrtChekBottomMargin);

    settings.setValue("exist_counter_printer_chek", config.existCounterPrinterChek);
    settings.setValue("exist_counter_chek", config.existCounterChek);
    settings.setValue("counter_len_rulon", config.counterWidthRulon);
    settings.setValue("counter_len_chek", config.counterWidthChek);
    settings.setValue("counter_ring_value", config.counterCountCheck);

    settings.setValue("sms_send_number", config.smsSendNumber);
    settings.setValue("sms_err_validator", config.smsErrValidator);
    settings.setValue("sms_err_printer", config.smsErrPrinter);
    settings.setValue("sms_err_balance_agent", config.smsErrBalanceAgent);
    settings.setValue("sms_value_balance_agent", config.smsValueBalanceAgent);
    settings.setValue("sms_err_sim_balance", config.smsErrSimBalance);
    settings.setValue("sms_err_lock_terminal", config.smsErrLockTerminal);
    settings.setValue("sms_err_connection", config.smsErrConnection);

    settings.setValue("status_validator_jam_in_box", config.statusValidatorJamInBox);
    settings.setValue("status_validator_jam_in_box_value_counter",
                      config.statusValidatorJamInBoxValueCounter);
    settings.setValue("status_validator_jam_in_box_lockers", config.statusValidatorJamInBoxLockers);

    settings.setValue("auto_update_status", config.autoUpdateStatus);
    settings.setValue("tpl", config.tpl);
    settings.setValue("test", config.test);

    settings.sync();
}

void MainWindow::settingsGet() {
    QSettings settings(settingsPath(), QSettings::IniFormat);

    config.checkGetBalanceSim = settings.value("check_balance_sim").toBool();
    config.checkGetNumberSim = settings.value("check_number_sim").toBool();
    config.vpnName = settings.value("vpn_point").toString();
    config.simBalanceRequest = settings.value("ussd_balance_sim").toString();
    config.simNumberRequest = settings.value("ussd_number_sim").toString();
    config.indexCheckBalance = settings.value("index_check_balance").toInt();
    config.showPrintDialog = settings.value("show_print_dialog").toBool();

    config.printerData.checkWidth = settings.value("chek_width").toInt();
    config.printerData.leftMargin = settings.value("chek_left_size").toInt();
    config.printerData.smallChek = settings.value("chek_small_text").toBool();
    config.printerData.printCheckUntil = settings.value("printing_chek").toInt();
    config.printerData.smallBeetwenString = settings.value("chek_small_beetwen_string").toBool();

    config.timerRasReb = settings.value("ras_error_interval_reboot").toInt();
    config.langDefault = settings.value("default_lang").toString();

    // TODO: Remove after update
    if (config.langDefault == "tj" || config.langDefault == "uz") {
        config.langDefault = "local";
    }

    config.searchValidator = settings.value("search_validator").toBool();
    config.searchCoinAcceptor = settings.value("search_coin_acceptor", true).toBool();
    config.searchPrinter = settings.value("search_printer").toBool();
    config.searchModem = settings.value("search_modem").toBool();
    config.searchWD = settings.value("search_watchdog").toBool();

    config.winPrtChekWidth = settings.value("prt_win_width").toInt();
    config.winPrtChekHeight = settings.value("prt_win_height").toInt();
    config.winPrtChekFontSize = settings.value("prt_win_font_size").toInt();
    config.winPrtChekLeftMargin = settings.value("prt_win_left_margin").toInt();
    config.winPrtChekRightMargin = settings.value("prt_win_right_margin").toInt();
    config.winPrtChekTopMargin = settings.value("prt_win_top_margin").toInt();
    config.winPrtChekBottomMargin = settings.value("prt_win_bottom_margin").toInt();

    config.existCounterPrinterChek = settings.value("exist_counter_printer_chek").toBool();
    config.existCounterChek = settings.value("exist_counter_chek").toBool();
    config.counterWidthRulon = settings.value("counter_len_rulon").toInt();
    config.counterWidthChek = settings.value("counter_len_chek").toDouble();
    config.counterCountCheck = settings.value("counter_ring_value").toInt();

    config.smsSendNumber = settings.value("sms_send_number").toString();
    config.smsErrValidator = settings.value("sms_err_validator").toBool();
    config.smsErrPrinter = settings.value("sms_err_printer").toBool();
    config.smsErrBalanceAgent = settings.value("sms_err_balance_agent").toBool();
    config.smsValueBalanceAgent = settings.value("sms_value_balance_agent").toDouble();
    config.smsErrSimBalance = settings.value("sms_err_sim_balance").toBool();
    config.smsErrLockTerminal = settings.value("sms_err_lock_terminal").toBool();
    config.smsErrConnection = settings.value("sms_err_connection").toBool();

    config.statusValidatorJamInBox = settings.value("status_validator_jam_in_box").toBool();
    config.statusValidatorJamInBoxValueCounter =
        settings.value("status_validator_jam_in_box_value_counter").toInt();
    config.statusValidatorJamInBoxLockers =
        settings.value("status_validator_jam_in_box_lockers").toBool();
    config.lockDuplicateNominal = settings.value("lock_duplicate_nominal", false).toBool();

    config.autoUpdateStatus = settings.value("auto_update_status").toBool();
    config.tpl = settings.value("tpl", "tjk").toString();
    config.test = settings.value("test", false).toBool();
    config.inspect = settings.value("inspect", false).toBool();
}

void MainWindow::settingsSet(const QString key, const QVariant value) {
    QSettings settings(settingsPath(), QSettings::IniFormat);
    settings.setValue(key, value);
}

void MainWindow::tplSelected(QString tpl, bool test) {
    config.tpl = tpl;
    config.test = test;

    auto key = QString("%1_%2").arg(config.tpl, config.test ? "test" : "prod");

    config.serverAddress = serverAddress[key];

    authRequest->setUrl(config.serverAddress + "auth");
    connObject->setEndpoint(5, config.serverAddress + "ping");
}

QString MainWindow::interfaceText(QString key) {

    QVariantMap data;

    data["copyrigh_info"] = tr("Версия ПО");
    data["about_company"] = tr("Платёжная  Система [v1]");
    data["validator_info_searching"] = tr("Купюроприемник");
    data["validator_info_found"] = tr("Купюроприемник [v1] [v2]");
    data["validator_info_notfound"] = tr("Купюроприемник");
    data["coin_acceptor_info_searching"] = tr("Монетоприемник");
    data["coin_acceptor_info_found"] = tr("Монетоприемник [v1] [v2]");
    data["coin_acceptor_info_notfound"] = tr("Монетоприемник");
    data["printer_info_searching"] = tr("Принтер");
    data["printer_info_found"] = tr("Принтер [v1] [v2]");
    data["printer_info_notfound"] = tr("Принтер");
    data["modem_info_searching"] = tr("Модем");
    data["modem_info_found"] = tr("Модем [v1] [v2]");
    data["modem_info_notfound"] = tr("Модем ");
    data["wd_info_searching"] = tr("Идет поиск сторожевого таймера...");
    data["wd_info_found"] = tr("Сторожевой таймер [v1] на [v2] найден.");
    data["wd_info_notfound"] = tr("Сторожевой таймер не найден!");

    return data.value(key).toString();
}

void MainWindow::filesUpdated(QVariantMap files) {
    if (files.value("style.qss").toBool()) {
        // Загружаем интерфейс
        loadStyleSheet();
    }
}

QVariantMap MainWindow::nominalData() {

    QFile file(QString("%1/%2/nominals.json").arg(ConstData::Path::Nominals, config.tpl));

    if (!file.open(QIODevice::ReadOnly)) {
        return QVariantMap();
    }

    auto json = QJsonDocument::fromJson(file.readAll());
    auto data = json.toVariant().toMap();

    file.close();

    return data;
}

void MainWindow::wsConnectionOpen() {
    webSocket.open(QUrl(QString("ws://%1:%2").arg(wsIp).arg(wsPort)));
}

void MainWindow::wsConnected() {
    wsStateQuery();

    if (!wsStateTimer.isActive()) {
        wsStateTimer.start();
    }
}

void MainWindow::wsQuery(const QString query, const QVariantMap data) {
    auto queryData = data;
    queryData.insert("query", query);

    auto json = QJsonDocument::fromVariant(queryData);
    auto bytes = json.toJson(QJsonDocument::Compact);

    if (webSocket.isValid()) {
        webSocket.sendTextMessage(bytes);
    }
}

void MainWindow::wsStateChanged(QAbstractSocket::SocketState state) {
    if (state == QAbstractSocket::UnconnectedState) {
        if (wsReconnectTimer.isActive()) {
            wsReconnectTimer.stop();
        }

        wsReconnectTimer.start();
    } else if (state == QAbstractSocket::ConnectedState) {
        wsReconnectTimer.stop();
    }
}

void MainWindow::wsStateQuery() {
    QVariantMap data;
    data["timeout"] = wsStateInterval;

    wsQuery("state", data);
}

void MainWindow::updaterLock(bool lock) {
    settingsSet("updater_lock", lock);
}

void MainWindow::killSheller() {
    QProcess proc;
    proc.startDetached("taskkill", QStringList() << "/f" << "/IM" << sheller);
}

bool MainWindow::hasProcess(const QString &process) {
    QProcess tasklist;
    tasklist.start("tasklist",
                   QStringList() << "/NH"
                                 << "/FO" << "CSV"
                                 << "/FI" << QString("IMAGENAME eq %1").arg(process));
    tasklist.waitForFinished();

    QString output = tasklist.readAllStandardOutput();

    return output.contains(QString("%1").arg(process));
}

void MainWindow::toLog(int state, QString title, QString text) {
    logger->setLogingText(state, title, text);
}

void MainWindow::toValidatorLog(int state, QByteArray data, QString text) {
    loggerValidator->setLogingText(state, data, text);
}

void MainWindow::loadStyleSheet() {

    // Загружаем интерфейс
    QFile file(QString("%1%2").arg(ConstData::Path::Styles, ConstData::FileName::StyleQSS));

    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
    }
}

QString MainWindow::versionFull() {
    return QString("%1 EPay v %2").arg(ConstData::companyName, ConstData::version);
}

qint32 MainWindow::generateEncodeKey() {
    QString vrmCodeData = "YTBnMFdXUjRXV3BM";
    QByteArray vrm_CodeData = QByteArray::fromBase64(vrmCodeData.toLatin1());
    vrmCodeData = "";
    vrmCodeData.append(vrm_CodeData);
    vrm_CodeData.clear();
    vrm_CodeData = QByteArray::fromBase64(vrmCodeData.toLatin1());
    QString vrm_strCodeData;
    vrm_strCodeData.append(vrm_CodeData);

    return vrm_strCodeData.toInt();
}

#ifdef Q_OS_WIN32
QStringList MainWindow::getWinPrinterNames() {
    QString printerName;
    QStringList printerNames;

    DWORD size;
    DWORD numPrinters;
    PRINTER_INFO_2W *printerInfos = NULL;
    EnumPrintersW(
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &size, &numPrinters);
    printerInfos = (PRINTER_INFO_2W *)malloc(size);

    if (EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                      NULL,
                      2,
                      (LPBYTE)printerInfos,
                      size,
                      &size,
                      &numPrinters)) {
        for (uint i = 0; i < numPrinters; i++) {
            printerName = QString::fromUtf16(
                reinterpret_cast<const char16_t *>(printerInfos[i].pPrinterName));
            printerNames.append(printerName);
        }
        printerNames.sort();
    }

    if (printerInfos)
        free(printerInfos);

    return printerNames;
}
#else
QStringList MainWindow::getWinPrinterNames() {
    // Windows printer enumeration not available on this platform
    return QStringList();
}
#endif // Q_OS_WIN32

void MainWindow::loadWebSettings() {
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));

    // QtWebEngine settings are configured through global web engine configuration
    // Most settings are handled automatically by QtWebEngine
    // For font settings, we can store them for reference but QtWebEngine will use
    // system defaults

    QFont standardFont =
        qvariant_cast<QFont>(settings.value(QLatin1String("standardFont"), QFont()));
    QFont fixedFont = qvariant_cast<QFont>(settings.value(QLatin1String("fixedFont"), QFont()));

    // Note: In QtWebEngine, font configuration is done through CSS/webpage
    // preferences These font settings from Qt4 era are stored but may need to be
    // applied via JavaScript injection if you need custom fonts in web content

    // Basic settings that are enabled by default in QtWebEngine:
    // - JavaScript is enabled
    // - LocalContentCanAccessRemoteUrls equivalent behavior

    // Note: Plugins (Flash, etc.) are not supported in QtWebEngine

    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Останавливаем поиск устройств
    if (searchDevices) {
        searchDevices->terminate();
        searchDevices->wait();
    }

    wsQuery("state_stop");

    // Останавливаем запись в лог
    logger->timer->stop();
    logger->terminate();
    logger->wait();

    // Останавливаем проверку соединения
    connObject->daemonTimer->stop();
    connObject->closeThis();

    // Закрываем порт принтера
    clsPrinter->closeThis();
    clsPrinter->terminate();
    clsPrinter->wait();

    logClean->terminate();
    logClean->wait();

    systemInfo->terminate();
    systemInfo->wait();

    // Остнавливаем опрос купюроприемника
    statusValidatorTimer->stop();

    if (clsCoinAcceptor->pollState()) {
        clsCoinAcceptor->execCommand(AcceptorCommands::StopPolling);
    }

    if (clsValidator->pollState()) {
        // toDebuging("-----POLL STATE ACTIVE-----");
        clsValidator->execCommand(ValidatorCommands::StopPolling);

        QTimer::singleShot(1000, this, SLOT(closeApp()));
        event->accept();
    } else {
        // toDebuging("-----POLL STATE NOT ACTIVE-----");
        event->accept();

        closeApp();
    }
}

void MainWindow::closeApp() {
    // Закрываем порт купюроприёмника
    clsValidator->closeThis();
    clsValidator->terminate();
    clsValidator->wait();

    clsCoinAcceptor->closeThis();
    clsCoinAcceptor->terminate();
    clsCoinAcceptor->wait();

    qApp->closeAllWindows();
    qApp->quit();
}
