#include "registrationform.h"

#include "ui_registrationform.h"

RegistrationForm::RegistrationForm(QWidget *parent)
    : QDialog(parent), ui(new Ui::RegistrationForm) {
    ui->setupUi(this);

    // Создание нового соединения
    createDialupConnection = new CreateDialupConnection(this);
    connect(createDialupConnection,
            SIGNAL(emitDialupParam(QVariantMap)),
            SIGNAL(emitCreateNewConnection(QVariantMap)));

    KeyPud = new keyPud(this);
    ui->layoutWgtKeyPud->addWidget(KeyPud);

    connect(KeyPud, SIGNAL(characterGenerated(QChar)), this, SLOT(sendCharacter(QChar)));

    connect(ui->btnEditValidator, SIGNAL(clicked(bool)), SLOT(showValidatorData(bool)));
    connect(ui->btnEditCoinAcceptor, SIGNAL(clicked(bool)), SLOT(showCoinAcceptorData(bool)));
    connect(ui->btnEditPrinter, SIGNAL(clicked(bool)), SLOT(showPrinterData(bool)));
    connect(ui->btnEditWatchDog, SIGNAL(clicked(bool)), SLOT(showWatchdogData(bool)));
    connect(ui->btnEditModem, SIGNAL(clicked(bool)), SLOT(showModem_Data(bool)));

    connect(ui->chbValidatorSearch, SIGNAL(toggled(bool)), SLOT(searchValidatorToggle(bool)));
    connect(
        ui->chbCoinAcceptorSearch, SIGNAL(toggled(bool)), SLOT(searchCointAcceptorToggle(bool)));
    connect(ui->chbPrinterSearch, SIGNAL(toggled(bool)), SLOT(searchPrinterToggle(bool)));
    connect(ui->chbWatchDogSearch, SIGNAL(toggled(bool)), SLOT(searchWatchdogToggle(bool)));
    connect(ui->chbModemSearch, SIGNAL(toggled(bool)), SLOT(searchModem_Toggle(bool)));

    connect(ui->btnTestValidator, SIGNAL(clicked()), SLOT(validatorTest()));
    connect(ui->btnTestCoinAcceptor, SIGNAL(clicked()), SLOT(coinAcceptorTest()));
    connect(ui->btnTestPrinter, SIGNAL(clicked()), SLOT(printerTest()));
    connect(ui->btnTestWatchDog, SIGNAL(clicked()), SLOT(watchdogTest()));
    connect(ui->btnTestModem, SIGNAL(clicked()), SLOT(modem_Test()));

    connect(ui->btnCloseReg, SIGNAL(clicked()), SLOT(closeConfirm()));
    connect(ui->btnBackReg, SIGNAL(clicked()), SLOT(btnBackClicked()));
    connect(ui->btnNextReg, SIGNAL(clicked()), SLOT(btnNextClicked()));

    connect(ui->btnSearchStart, SIGNAL(clicked()), SLOT(btnSearchStartClc()));

    connect(ui->btnNewConnection, SIGNAL(clicked()), SLOT(createNewConnection()));

    connect(ui->cbxValidatorName, SIGNAL(activated(int)), SLOT(validatorSave()));
    connect(ui->cbxValidatorPort, SIGNAL(activated(int)), SLOT(validatorSave()));

    connect(ui->cbxCoinAcceptorName, SIGNAL(activated(int)), SLOT(coinAcceptorSave()));
    connect(ui->cbxCoinAcceptorPort, SIGNAL(activated(int)), SLOT(coinAcceptorSave()));

    connect(ui->cbxPrinterName, SIGNAL(activated(int)), SLOT(printerSave()));
    connect(ui->cbxPrinterPort, SIGNAL(activated(int)), SLOT(printerSave()));
    connect(ui->cbxWinprinter, SIGNAL(activated(int)), SLOT(printerSave()));

    connect(ui->cbxWatchdogPort, SIGNAL(activated(int)), SLOT(watchdogSave()));

    connect(ui->cbxModemPort, SIGNAL(activated(int)), SLOT(modem_Save()));

    connect(ui->radioDialUpConnection, SIGNAL(toggled(bool)), SLOT(showDialUpParams(bool)));
    connect(ui->listWidgetConnectionList,
            SIGNAL(currentTextChanged(QString)),
            SLOT(connectionSelect(QString)));

    auto *loginRegValidator = new QRegularExpressionValidator(
        QRegularExpression(R"([\S\w\W\d\D]{4,15})"), ui->editLoginReg);
    ui->editLoginReg->setValidator(loginRegValidator);

    auto *passRegValidator = new QRegularExpressionValidator(
        QRegularExpression(R"([\S\w\W\d\D]{4,15})"), ui->editOtpReg);
    ui->editOtpReg->setValidator(passRegValidator);

    auto *secretLoginValidator = new QRegularExpressionValidator(
        QRegularExpression(R"([\S\w\W\d\D]{1,15})"), ui->editSecretLoginReg);
    ui->editSecretLoginReg->setValidator(secretLoginValidator);

    auto *secretPassValidator = new QRegularExpressionValidator(
        QRegularExpression(R"([\S\w\W\d\D]{1,15})"), ui->editSecretPasswordReg);
    ui->editSecretPasswordReg->setValidator(secretPassValidator);

    auto *secretPassConfirm_Validator = new QRegularExpressionValidator(
        QRegularExpression(R"([\S\w\W\d\D]{1,15})"), ui->editSecretPasswordConfirmReg);
    ui->editSecretPasswordConfirmReg->setValidator(secretPassConfirm_Validator);

    connect(ui->editLoginReg, SIGNAL(textChanged(QString)), SLOT(checkAuthInput(QString)));
    connect(ui->editOtpReg, SIGNAL(textChanged(QString)), SLOT(checkAuthInput(QString)));
    connect(ui->editSecretLoginReg, SIGNAL(textChanged(QString)), SLOT(checkAuthInput(QString)));
    connect(ui->editSecretPasswordReg, SIGNAL(textChanged(QString)), SLOT(checkAuthInput(QString)));
    connect(ui->editSecretPasswordConfirmReg,
            SIGNAL(textChanged(QString)),
            SLOT(checkAuthInput(QString)));

    connect(ui->btnSecretPassword, SIGNAL(toggled(bool)), SLOT(secretPassView(bool)));
    connect(
        ui->btnSecretPasswordConfirm, SIGNAL(toggled(bool)), SLOT(secretPassConfirm_View(bool)));

    connect(ui->radioTplTJK, SIGNAL(clicked()), SLOT(tplCheck()));
    connect(ui->radioTplUZB, SIGNAL(clicked()), SLOT(tplCheck()));
    connect(ui->tplImageTJK, SIGNAL(clicked()), SLOT(tplTJKSet()));
    connect(ui->tplImageUZB, SIGNAL(clicked()), SLOT(tplUZBSet()));

    tplStyleEnable = "border: 2px solid;"
                     "border-color: #ff850e;"
                     "border-radius: 8px;";

    tplStyleDisable = "border: 2px solid;"
                      "border-color: #e5e5e5;"
                      "border-radius: 8px;";

    ui->tplImageTJK->setStyleSheet(tplStyleDisable);
    ui->tplImageUZB->setStyleSheet(tplStyleDisable);

    QPixmap pixmap("assets/images/screen_tjk.jpg");
    ui->tplImageTJK->setIcon(QIcon(pixmap));
    ui->tplImageTJK->setIconSize(pixmap.rect().size() * 0.96);

    pixmap.load("assets/images/screen_uzb.jpg");
    ui->tplImageUZB->setIcon(QIcon(pixmap));
    ui->tplImageUZB->setIconSize(pixmap.rect().size() * 0.96);

    //    setWindowFlag(Qt::WindowStaysOnTopHint);

    ui->progressBar->setVisible(false);
    ui->lblStatus->setVisible(false);
    ui->chbTest->setVisible(false);

    ui->lblTitleWinprinter->setVisible(false);
    ui->cbxWinprinter->setVisible(false);
}

RegistrationForm::~RegistrationForm() {
    delete ui;
}

void RegistrationForm::setDB(QSqlDatabase db) {
    this->db = db;

    //    searchDevices->setDbName(db);
    //    getServices->setDbConnect(this->db);
}

void RegistrationForm::btnBackClicked() {
    clearStatusText();

    switch (ui->stackedWgtReg->currentIndex()) {
    case Page::SelectTpl: {
        closeConfirm();
        return;
    }
    case Page::SearchDevice: {
        ui->lblStatus->setVisible(false);
        ui->stackedWgtReg->setCurrentIndex(Page::SelectTpl);
    } break;
    case Page::SelectConnection:
        ui->stackedWgtReg->setCurrentIndex(Page::SearchDevice);
        break;
    case Page::EnterAuthData:
        ui->stackedWgtReg->setCurrentIndex(Page::SelectConnection);
        break;
    default:
        break;
    }

    addPageData();
}

void RegistrationForm::btnNextClicked() {
    clearStatusText();

    switch (ui->stackedWgtReg->currentIndex()) {
    case Page::SelectTpl: {
        ui->lblStatus->setVisible(true);
        ui->stackedWgtReg->setCurrentIndex(Page::SearchDevice);
        addPageData();

        const auto *tpl = ui->radioTplUZB->isChecked() ? "uzb" : "tjk";
        auto test = ui->chbTest->isChecked();
        emit emitTpl(tpl, test);
    } break;
    case Page::SearchDevice:
        if (searchClicked) {
            ui->stackedWgtReg->setCurrentIndex(Page::SelectConnection);
            addPageData();
            searchClicked = false;
            return;
        }

        deviceSearchStart();
        break;
    case Page::SelectConnection: {
        // Тип подключения
        QString connectionName = ui->radioDialUpConnection->isChecked()
                                     ? ui->listWidgetConnectionList->currentItem()->text()
                                     : "Local Connection";

        setLoading(true);
        emit emitStartToConnect(connectionName);
    } break;
    case Page::EnterAuthData: {
        auto secretPass = ui->editSecretPasswordReg->text().trimmed();
        auto secretPassConfirm = ui->editSecretPasswordConfirmReg->text().trimmed();

        if (secretPass != secretPassConfirm) {
            setStatusText(Status::Error, "Не совпадает пароль входа в админ");
            return;
        }

        setLoading(true);
        auto login = ui->editLoginReg->text().trimmed();
        auto otp = ui->editOtpReg->text().trimmed();

        emit emitSendAuthRequest(login, otp);
    } break;
        //        case Page::EnterAdminAuthData:
        //            // Запуск программы
        //            break;
    default:
        break;
    }

    //    addPageData();
}

void RegistrationForm::authResponse(const QString resultCode,
                                    const QString token,
                                    const QString message) {
    setLoading(false);

    if (resultCode != "" && resultCode.toInt() == 0 && token != "") {
        ui->btnBackReg->setEnabled(false);
        ui->btnNextReg->setEnabled(false);

        auto msg = message.isEmpty() ? "Авторизация прошла успешно" : message;

        setStatusText(Status::Success, msg);
        emit emitToLog(0, "AUTH", msg);

        sleep(1000);

        QVariantMap data;
        data["login"] = ui->editLoginReg->text();
        data["token"] = token;
        data["admin_login"] = ui->editSecretLoginReg->text();
        data["admin_password"] = ui->editSecretPasswordReg->text();
        data["connection"] = ui->radioDialUpConnection->isChecked()
                                 ? ui->listWidgetConnectionList->currentItem()->text()
                                 : "Local Connection";
        data["sim_balance_req"] = ui->cbxRequestBalanceSimReg->currentText().trimmed();
        data["sim_balance_position"] = ui->cbxPositionBalanceSimReg->currentText().trimmed();
        data["sim_number_req"] = ui->cbxRequestNumberSimReg->currentText().trimmed();

        data["test"] = ui->chbTest->isChecked();
        data["tpl"] = ui->radioTplUZB->isChecked() ? "uzb" : "tjk";

        emit emitRegistrationData(data);
        return;
    }

    if (resultCode == "") {
        setStatusText(Status::Error, "Сервер недоступен или нет связи");
        emit emitToLog(1, "AUTH", "Сервер недоступен или нет связи");
    } else {
        auto msg = message.isEmpty() ? "Ошибка авторизации" : message;
        setStatusText(Status::Error, msg);
        emit emitToLog(2, "AUTH", msg);
    }
}

void RegistrationForm::setConnectionState(bool connectionOk, QString message) {
    setLoading(false);

    if (connectionOk) {
        setStatusText(Status::Success, "Соединение успешно");

        sleep(1000);

        // Переход на страницу ввода авторизационных данных
        ui->stackedWgtReg->setCurrentIndex(Page::EnterAuthData);
        clearStatusText();
        addPageData();
    } else {
        setStatusText(Status::Error, message);
    }
}

void RegistrationForm::setLoading(bool val) {
    ui->progressBar->setVisible(val);
    ui->btnBackReg->setEnabled(!val);
    ui->btnNextReg->setEnabled(!val);
}

void RegistrationForm::setStatusText(int status, QString text) {
    QString style = "";

    switch (status) {
    case Status::Info:
        style = "color: #000000;";
        break;
    case Status::Success:
        style = "color: #228B22;";
        break;
    case Status::Error:
        style = "color: #FF0000;";
        break;
    default:
        break;
    }

    ui->lblStatus->setStyleSheet(style);
    ui->lblStatus->setText(text);
}

void RegistrationForm::clearStatusText() {
    ui->lblStatus->clear();
}

void RegistrationForm::hideGroupDevice() {
    ui->groupBoxValidator->setVisible(false);
    ui->groupBoxCoinAcceptor->setVisible(false);
    ui->groupBoxPrinter->setVisible(false);
    ui->groupBoxWatchDog->setVisible(false);
    ui->groupBoxModem->setVisible(false);
}

void RegistrationForm::addPageData() {
    int curentId = ui->stackedWgtReg->currentIndex();
    QString title = "";
    QString btnBack = "";
    QString btnNext = "";
    bool btnNextEnabled = false;

    switch (curentId) {
    case Page::SelectTpl:
        title = tr("%1 1/4 %2 Выбор интерфейса<br>\n");
        btnBack = tr("Закрыть");
        btnNext = tr("Далее");

        btnNextEnabled = ui->radioTplTJK->isChecked() || ui->radioTplUZB->isChecked();
        break;
    case Page::SearchDevice: {
        title = tr("%1 2/4 %2 Настройки устройств терминала<br>\n");

        btnBack = tr("Назад");
        btnNext = tr("Далее");

        btnNextEnabled = true;

        hideGroupDevice();
    } break;
    case Page::SelectConnection: {
        title = tr("%1 3/4 %2 Выберите тип соединения и нажмите<br>кнопку далее");
        btnBack = tr("Назад");
        btnNext = tr("Далее");
        btnNextEnabled = isValidConnectionType();
    } break;
    case Page::EnterAuthData: {
        title = tr("%1 4/4 %2 Введите авторизационные данные терминала<br>и "
                   "нажмите кнопку далее");
        btnBack = tr("Назад");
        btnNext = tr("Далее");

        btnNextEnabled = isValidAuthInput();
    } break;
        //        case Page::EnterAdminAuthData:
        //        {
        //            title = tr("%2 4/5 %3 Введите секретные данные%1(такие как
        //            секретный номер телефона, секретный логин и пароль)%1для входа
        //            в админку и нажмите на кнопку далее"); btnBack = tr("Назад");
        //            btnNext = tr("Сохранить");

        //            if (ui->editSecretLoginReg->hasAcceptableInput() &&
        //            ui->editSecretPasswordReg->hasAcceptableInput() &&
        //            ui->editSecretPasswordConfirmReg->hasAcceptableInput() &&
        //            (ui->editSecretPasswordReg->text() ==
        //            ui->editSecretPasswordConfirmReg->text())) {
        //                btnNextEnabled = true;
        //            }
        //        }
        //        break;
    default:
        break;
    }

    title = title.arg("<font color=\"red\"><b>", "</font></b>");

    ui->btnNextReg->setText(btnNext);
    ui->btnBackReg->setText(btnBack);

    // Включаем кнопку
    ui->btnNextReg->setEnabled(btnNextEnabled);

    // Рисуем тайтл
    ui->lblRegTitle->setText(title);

    showKeyPud();
}

void RegistrationForm::showValidatorData(bool show) {
    ui->groupBoxValidator->setVisible(show);

    ui->btnEditCoinAcceptor->setChecked(false);
    ui->btnEditPrinter->setChecked(false);
    ui->btnEditWatchDog->setChecked(false);
    ui->btnEditModem->setChecked(false);

    ui->groupBoxCoinAcceptor->setVisible(false);
    ui->groupBoxPrinter->setVisible(false);
    ui->groupBoxWatchDog->setVisible(false);
    ui->groupBoxModem->setVisible(false);

    clearStatusText();
}

void RegistrationForm::showCoinAcceptorData(bool show) {
    ui->groupBoxCoinAcceptor->setVisible(show);

    ui->btnEditValidator->setChecked(false);
    ui->btnEditPrinter->setChecked(false);
    ui->btnEditWatchDog->setChecked(false);
    ui->btnEditModem->setChecked(false);

    ui->groupBoxValidator->setVisible(false);
    ui->groupBoxPrinter->setVisible(false);
    ui->groupBoxWatchDog->setVisible(false);
    ui->groupBoxModem->setVisible(false);

    clearStatusText();
}

void RegistrationForm::showPrinterData(bool show) {
    ui->groupBoxPrinter->setVisible(show);

    ui->btnEditValidator->setChecked(false);
    ui->btnEditCoinAcceptor->setChecked(false);
    ui->btnEditWatchDog->setChecked(false);
    ui->btnEditModem->setChecked(false);

    ui->groupBoxValidator->setVisible(false);
    ui->groupBoxCoinAcceptor->setVisible(false);
    ui->groupBoxWatchDog->setVisible(false);
    ui->groupBoxModem->setVisible(false);

    clearStatusText();
}

void RegistrationForm::showWatchdogData(bool show) {
    ui->groupBoxWatchDog->setVisible(show);

    ui->btnEditValidator->setChecked(false);
    ui->btnEditCoinAcceptor->setChecked(false);
    ui->btnEditPrinter->setChecked(false);
    ui->btnEditModem->setChecked(false);

    ui->groupBoxValidator->setVisible(false);
    ui->groupBoxCoinAcceptor->setVisible(false);
    ui->groupBoxPrinter->setVisible(false);
    ui->groupBoxModem->setVisible(false);

    clearStatusText();
}

void RegistrationForm::showModem_Data(bool show) {
    ui->groupBoxModem->setVisible(show);

    ui->btnEditValidator->setChecked(false);
    ui->btnEditCoinAcceptor->setChecked(false);
    ui->btnEditPrinter->setChecked(false);
    ui->btnEditWatchDog->setChecked(false);

    ui->groupBoxValidator->setVisible(false);
    ui->groupBoxCoinAcceptor->setVisible(false);
    ui->groupBoxPrinter->setVisible(false);
    ui->groupBoxWatchDog->setVisible(false);

    clearStatusText();
}

void RegistrationForm::searchValidatorToggle(bool value) {
    ui->labelDeviceValidator->setEnabled(value);
    ui->btnEditValidator->setEnabled(value);

    if (!value) {
        showValidatorData(false);
        ui->btnEditValidator->setChecked(false);
    }

    ui->labelDeviceValidator->setText(
        QString("<font color=\"%2\">%1</font>").arg("Купюроприемник", value ? "black" : "gray"));
}

void RegistrationForm::searchCointAcceptorToggle(bool value) {
    ui->labelDeviceCoinAcceptor->setEnabled(value);
    ui->btnEditCoinAcceptor->setEnabled(value);

    if (!value) {
        showCoinAcceptorData(false);
        ui->btnEditCoinAcceptor->setChecked(false);
    }

    ui->labelDeviceCoinAcceptor->setText(
        QString("<font color=\"%2\">%1</font>").arg("Монетоприемник", value ? "black" : "gray"));
}

void RegistrationForm::searchPrinterToggle(bool value) {
    ui->labelDevicePrinter->setEnabled(value);
    ui->btnEditPrinter->setEnabled(value);

    if (!value) {
        showPrinterData(false);
        ui->btnEditPrinter->setChecked(false);
    }

    ui->labelDevicePrinter->setText(
        QString("<font color=\"%2\">%1</font>").arg("Принтер", value ? "black" : "gray"));
}

void RegistrationForm::searchWatchdogToggle(bool value) {
    ui->labelDeviceWatchDog->setEnabled(value);
    ui->btnEditWatchDog->setEnabled(value);

    if (!value) {
        showWatchdogData(false);
        ui->btnEditWatchDog->setChecked(false);
    }

    ui->labelDeviceWatchDog->setText(
        QString("<font color=\"%2\">%1</font>").arg("Сторожевой таймер", value ? "black" : "gray"));
}

void RegistrationForm::searchModem_Toggle(bool value) {
    ui->labelDeviceModem->setEnabled(value);
    ui->btnEditModem->setEnabled(value);

    if (!value) {
        showModem_Data(false);
        ui->btnEditModem->setChecked(false);
    }

    ui->labelDeviceModem->setText(
        QString("<font color=\"%2\">%1</font>").arg("Модем", value ? "black" : "gray"));
}

void RegistrationForm::validatorTest() {
    QString devName = ui->cbxValidatorName->currentText();
    QString port = ui->cbxValidatorPort->currentText();
    QString comment = "";

    setLoading(true);
    clearStatusText();
    emit emitDeviceTest(SearchDev::search_validator, devName, port, comment);
}

void RegistrationForm::coinAcceptorTest() {
    QString devName = ui->cbxCoinAcceptorName->currentText();
    QString port = ui->cbxCoinAcceptorPort->currentText();
    QString comment = "";

    setLoading(true);
    clearStatusText();
    emit emitDeviceTest(SearchDev::search_coin_acceptor, devName, port, comment);
}

void RegistrationForm::printerTest() {
    QString devName = ui->cbxPrinterName->currentText();
    QString port = ui->cbxPrinterPort->currentText();
    QString comment = ui->cbxWinprinter->currentText();

    setLoading(true);
    clearStatusText();
    emit emitDeviceTest(SearchDev::search_printer, devName, port, comment);
}

void RegistrationForm::watchdogTest() {
    QString devName = "OSMP1";
    QString port = ui->cbxWatchdogPort->currentText();
    QString comment = "";

    setLoading(true);
    clearStatusText();
    emit emitDeviceTest(SearchDev::search_watchdog, devName, port, comment);
}

void RegistrationForm::modem_Test() {
    QString devName = "AT-Modem";
    QString port = ui->cbxModemPort->currentText();
    QString comment = "";

    setLoading(true);
    clearStatusText();
    emit emitDeviceTest(SearchDev::search_modem, devName, port, comment);
}

void RegistrationForm::validatorSave() {
    if (com_PortList.size() > 0) {
        QString devName = ui->cbxValidatorName->currentText();
        QString portName = ui->cbxValidatorPort->currentText();
        QString comment = "";

        emit emitSaveDevice(1, devName, portName, comment, 1);
    }
}

void RegistrationForm::coinAcceptorSave() {
    if (com_PortList.size() > 0) {
        QString devName = ui->cbxCoinAcceptorName->currentText();
        QString portName = ui->cbxCoinAcceptorPort->currentText();
        QString comment = "";

        emit emitSaveDevice(5, devName, portName, comment, 1);
    }
}

void RegistrationForm::printerSave() {
    QString devName = ui->cbxPrinterName->currentText();

    auto isWinprinter = devName == "WindowsPrinter";

    ui->lblTitlePrinterPort->setVisible(!isWinprinter);
    ui->cbxPrinterPort->setVisible(!isWinprinter);
    ui->lblTitleWinprinter->setVisible(isWinprinter);
    ui->cbxWinprinter->setVisible(isWinprinter);

    QString portName = ui->cbxPrinterPort->currentText();
    QString comment = isWinprinter ? ui->cbxWinprinter->currentText() : "";

    emit emitSaveDevice(2, devName, portName, comment, 1);
}

void RegistrationForm::watchdogSave() {
    if (com_PortList.size() > 0) {
        QString devName = "OSMP1";
        QString portName = ui->cbxWatchdogPort->currentText();
        QString comment = "";

        emit emitSaveDevice(4, devName, portName, comment, 1);
    }
}

void RegistrationForm::modem_Save() {
    if (com_PortList.size() > 0) {
        QString devName = "AT-Modem";
        QString portName = ui->cbxModemPort->currentText();
        QString comment = "";

        emit emitSaveDevice(3, devName, portName, comment, 1);
    }
}

// void RegistrationForm::closeValidator() {
//     //Закрываем порт купюроприёмника
//     validator->closeThis();
//     validator->terminate();
//     validator->wait();
// }

void RegistrationForm::setSearchDeviceParams(QVariantMap data) {
    //    searchDevices->takeBalanceSim          =
    //    data.value("check_balance_sim").toBool(); searchDevices->takeSim_Number
    //    = data.value("check_number_sim").toBool();
    //    searchDevices->s_ussdRequestBalanseSim =
    //    data.value("ussd_balance_sim").toString();
    //    searchDevices->s_ussdRequestNumberSim  =
    //    data.value("ussd_number_sim").toString();
    //    searchDevices->s_indexBalanceParse     =
    //    data.value("index_check_balance").toInt();

    auto searchValidator = data.value("search_validator").toBool();
    auto searchCoinAcceptor = data.value("search_coin_acceptor").toBool();
    auto searchPrinter = data.value("search_printer").toBool();
    auto searchWD = data.value("search_watchdog").toBool();
    auto searchModem = data.value("search_modem").toBool();

    //    searchDevices->modem_ConUp        =
    //    data.value("modem_connection_up").toBool();
    //    searchDevices->searchValidator   = searchValidator;
    //    searchDevices->searchPrinter     = searchPrinter;
    //    searchDevices->searchWD          = searchWD;
    //    searchDevices->searchModem       = searchModem;
    //    searchDevices->testMode          = true;

    ui->chbValidatorSearch->setChecked(searchValidator);
    searchValidatorToggle(searchValidator);

    ui->chbCoinAcceptorSearch->setChecked(searchCoinAcceptor);
    searchCointAcceptorToggle(searchCoinAcceptor);

    ui->chbPrinterSearch->setChecked(searchPrinter);
    searchPrinterToggle(searchPrinter);

    ui->chbWatchDogSearch->setChecked(searchWD);
    searchWatchdogToggle(searchWD);

    ui->chbModemSearch->setChecked(searchModem);
    searchModem_Toggle(searchModem);
}

void RegistrationForm::btnSearchStartClc() {
    searchClicked = true;
    deviceSearchStart();
}

void RegistrationForm::deviceSearchStart() {
    ui->btnSearchStart->setEnabled(false);
    setLoading(true);

    ui->labelDeviceValidator->setText("Купюроприемник");
    ui->labelDeviceCoinAcceptor->setText("Монетоприемник");
    ui->labelDevicePrinter->setText("Принтер");
    ui->labelDeviceWatchDog->setText("Сторожевой таймер");
    ui->labelDeviceModem->setText("Модем");

    QVariantMap data;
    data["search_validator"] = ui->chbValidatorSearch->isChecked();
    data["search_coin_acceptor"] = ui->chbCoinAcceptorSearch->isChecked();
    data["search_printer"] = ui->chbPrinterSearch->isChecked();
    data["search_watchdog"] = ui->chbWatchDogSearch->isChecked();
    data["search_modem"] = ui->chbModemSearch->isChecked();

    emit emitStartSearchDevices(data);
}

void RegistrationForm::deviceSearchResult(
    int device, int result, QString dev_name, QString dev_comment, QString dev_port) {
    switch (device) {
    case SearchDev::search_validator: {
        switch (result) {
        case SearchDev::start_search: {
            QString vrmContentStart = tr("Идет поиск купюроприемник ...");
            ui->labelDeviceValidator->setText(vrmContentStart);
            //            emit emit_toLoging(TypeState::Info,
            //            LogFileName::SEARCH_DEVICE, vrmContentStart);
        } break;
        case SearchDev::device_found: {
            ui->btnNextReg->setEnabled(true);

            int curPos = -1;
            for (int i = 0; i < com_PortList.size(); i++) {
                if (dev_port == com_PortList.at(i)) {
                    curPos = i;
                    break;
                }
            }
            // Устанавливаем текущую позицию
            if (curPos >= 0) {
                ui->cbxValidatorPort->setCurrentIndex(curPos);
            }

            //                    validator->setValidator(dev_name);
            //                    validator->setPortName(dev_port);

            dev_comment = QString("(%1)").arg(dev_comment);
            QString vrmContent1 = tr("Купюроприемник [v1] на [v2] найден.");
            QString vrmContent2 = vrmContent1;
            //            emit emit_toLoging(TypeState::Info,
            //            LogFileName::SEARCH_DEVICE,vrmContent1.replace("[v1]",QString("%1
            //            %2").arg(dev_name).arg(dev_comment)).replace("[v2]",QString("%1").arg(dev_port)));
            ui->labelDeviceValidator->setText(
                vrmContent2.replace("[v1]", QString("<b>%1</b> %2").arg(dev_name, dev_comment))
                    .replace("[v2]", QString("<b>%1</b>").arg(dev_port)));
        } break;
        case SearchDev::device_notfound: {
            QString vrmContentFined = tr("Купюроприемник не найден!");
            ui->labelDeviceValidator->setText(
                QString("<font color=\"red\">%1</font>").arg(vrmContentFined));
            //            emit emit_toLoging(TypeState::Error,
            //            LogFileName::SEARCH_DEVICE, vrmContentFined);
        } break;
        }
    } break;
    case SearchDev::search_coin_acceptor: {
        switch (result) {
        case SearchDev::start_search: {
            QString vrmContentStart = tr("Идет поиск монетоприемника ...");
            ui->labelDeviceCoinAcceptor->setText(vrmContentStart);
        } break;
        case SearchDev::device_found: {
            ui->btnNextReg->setEnabled(true);

            int curPos = -1;
            for (int i = 0; i < com_PortList.size(); i++) {
                if (dev_port == com_PortList.at(i)) {
                    curPos = i;
                    break;
                }
            }
            // Устанавливаем текущую позицию
            if (curPos >= 0) {
                ui->cbxCoinAcceptorPort->setCurrentIndex(curPos);
            }

            dev_comment = QString("(%1)").arg(dev_comment);
            QString vrmContent1 = tr("Монетоприемник [v1] на [v2] найден.");
            QString vrmContent2 = vrmContent1;
            ui->labelDeviceCoinAcceptor->setText(
                vrmContent2.replace("[v1]", QString("<b>%1</b> %2").arg(dev_name, dev_comment))
                    .replace("[v2]", QString("<b>%1</b>").arg(dev_port)));
        } break;
        case SearchDev::device_notfound: {
            QString vrmContentFined = tr("Монетоприемник не найден!");
            ui->labelDeviceCoinAcceptor->setText(
                QString("<font color=\"red\">%1</font>").arg(vrmContentFined));
        } break;
        }
    } break;
    case SearchDev::search_printer: {
        switch (result) {
        case SearchDev::start_search: {
            QString vrmContentStart = tr("Идет поиск принтера ...");
            ui->labelDevicePrinter->setText(vrmContentStart);
            //            emit emit_toLoging(TypeState::Info,
            //            LogFileName::SEARCH_DEVICE, vrmContentStart);
        } break;
        case SearchDev::device_found: {
            int curPos = -1;
            for (int i = 0; i < com_PortList.size(); i++) {
                if (dev_port == com_PortList.at(i)) {
                    curPos = i;
                    break;
                }
            }
            // Устанавливаем текущую позицию
            if (curPos >= 0) {
                ui->cbxPrinterPort->setCurrentIndex(curPos);
            }

            int curPosName = -1;
            for (int i = 0; i < printerList.size(); i++) {
                if (dev_port == printerList.at(i)) {
                    curPosName = i;
                    break;
                }
            }
            // Устанавливаем текущую позицию
            if (curPosName >= 0) {
                ui->cbxPrinterName->setCurrentIndex(curPosName);
            }

            if (dev_comment != "" && dev_comment != " ") {
                dev_comment = QString("(%1)").arg(dev_comment);
            }
            QString vrmContent1 = tr("Принтер [v1] на [v2] найден.");
            QString vrmContent2 = vrmContent1;

            //                    printer->setPrinterModel(dev_name);
            //                    printer->setPortName(dev_port);

            ui->labelDevicePrinter->setText(
                vrmContent2.replace("[v1]", QString("<b>%1</b> %2").arg(dev_name, dev_comment))
                    .replace("[v2]", QString("<b>%1</b>").arg(dev_port)));
            //                emit emit_toLoging(TypeState::Info,
            //                LogFileName::SEARCH_DEVICE,vrmContent1.replace("[v1]",QString("%1
            //                %2").arg(dev_name).arg(dev_comment)).replace("[v2]",QString("%1").arg(dev_port)));
        } break;
        case SearchDev::device_notfound: {
            QString vrmContentFined = tr("Принтер не найден!");
            ui->labelDevicePrinter->setText(
                QString("<font color=\"red\">%1</font>").arg(vrmContentFined));
            //                emit emit_toLoging(TypeState::Error,
            //                LogFileName::SEARCH_DEVICE, vrmContentFined);
        } break;
        }
    } break;

    case SearchDev::search_modem: {
        switch (result) {
        case SearchDev::start_search: {
            QString vrmContentStart = tr("Идет поиск модема ...");
            ui->labelDeviceModem->setText(vrmContentStart);
            //            emit emit_toLoging(TypeState::Info,
            //            LogFileName::SEARCH_DEVICE, vrmContentStart);
        } break;
        case SearchDev::device_found: {

            int curPos = -1;
            for (int i = 0; i < com_PortList.size(); i++) {
                if (dev_port == com_PortList.at(i)) {
                    curPos = i;
                    break;
                }
            }
            // Устанавливаем текущую позицию
            if (curPos >= 0) {
                ui->cbxModemPort->setCurrentIndex(curPos);
            }

            ui->lblAdminTextDeviceInfModem->setText(
                QString("<b>%1</b> %2").arg(dev_name, dev_comment));
            QString vrmContent1 = tr("Модем [v1] на [v2] найден.");
            QString vrmContent2 = vrmContent1;

            ui->labelDeviceModem->setText(
                vrmContent2.replace("[v1]", QString("<b>%1</b> %2").arg(dev_name, dev_comment))
                    .replace("[v2]", QString("<b>%1</b>").arg(dev_port)));

            //            emit emit_toLoging(TypeState::Info,
            //            LogFileName::SEARCH_DEVICE,vrmContent1.replace("[v1]",QString("%1
            //            %2").arg(dev_name).arg(dev_comment)).replace("[v2]",QString("%1").arg(dev_port)));
        } break;
        case SearchDev::device_notfound: {
            QString vrmContentFined = tr("Модем не найден! или порт занят.");
            ui->labelDeviceModem->setText(
                QString("<font color=\"red\">%1</font>").arg(vrmContentFined));
            //            emit emit_toLoging(TypeState::Error,
            //            LogFileName::SEARCH_DEVICE, vrmContentFined);
        } break;
        }
    } break;
    case SearchDev::search_watchdog: {
        switch (result) {
        case SearchDev::start_search: {
            QString vrmContentStart = tr("Идет поиск Сторожевого таймера ...");
            ui->labelDeviceWatchDog->setText(vrmContentStart);
            //            emit emit_toLoging(TypeState::Info,
            //            LogFileName::SEARCH_DEVICE, vrmContentStart);
        } break;
        case SearchDev::device_found: {

            int curPos = -1;
            for (int i = 0; i < com_PortList.size(); i++) {
                if (dev_port == com_PortList.at(i)) {
                    curPos = i;
                    break;
                }
            }
            // Устанавливаем текущую позицию
            if (curPos >= 0) {
                ui->cbxWatchdogPort->setCurrentIndex(curPos);
            }

            ui->lblAdminTextDeviceInfWD->setText(
                QString("<b>%1</b> %2").arg(dev_name, dev_comment));

            dev_comment = QString("(%1)").arg(dev_comment);
            QString vrmContent1 = tr("Сторожевой таймер [v1] на [v2] найден.");
            QString vrmContent2 = vrmContent1;

            ui->labelDeviceWatchDog->setText(
                vrmContent2.replace("[v1]", QString("<b>%1</b> %2").arg(dev_name, dev_comment))
                    .replace("[v2]", QString("<b>%1</b>").arg(dev_port)));
            //                    emit emit_toLoging(TypeState::Info,
            //                    LogFileName::SEARCH_DEVICE,vrmContent1.replace("[v1]",QString("%1
            //                    %2").arg(dev_name).arg(dev_comment)).replace("[v2]",QString("%1").arg(dev_port)));
        } break;
        case SearchDev::device_notfound: {
            QString vrmContentFined = tr("Сторожевой таймер не найден! или занят.");
            ui->labelDeviceWatchDog->setText(
                QString("<font color=\"red\">%1</font>").arg(vrmContentFined));
            //                    emit emit_toLoging(TypeState::Error,
            //                    LogFileName::SEARCH_DEVICE, vrmContentFined);
        } break;
        }
    } break;
    default:
        break;
    }
}

void RegistrationForm::deviceSearchFinished() {
    ui->btnSearchStart->setEnabled(true);
    setLoading(false);

    if (!searchClicked) {
        sleep(1000);
        ui->stackedWgtReg->setCurrentIndex(Page::SelectConnection);
        addPageData();
    }
}

bool RegistrationForm::isValidConnectionType() {
    return ui->radioLocalConnection->isChecked() ||
           ui->listWidgetConnectionList->currentIndex().isValid();
}

void RegistrationForm::showDialUpParams(bool show) {
    ui->wgtDialUpConnectionData->setVisible(show);
    ui->wgtLocalConnectionData->setVisible(!show);

    ui->wgtKeyPudReg->setVisible(show);

    // Выбрано локальное соединение
    ui->btnNextReg->setEnabled(isValidConnectionType());

    clearStatusText();
}

void RegistrationForm::connectionSelect(QString connection) {
    if (connection == "") {
        return;
    }

    //    ui->lblAdminTitleSelectConnection->setText(tr("Selected"));

    //    selConString = con;
    //    this->lblSelectedConReg->setText(selConString);
    ui->btnNextReg->setEnabled(true);
}

void RegistrationForm::createNewConnection() {
    createDialupConnection->conList = connectionList;
    createDialupConnection->devList = dialupDevice;

    createDialupConnection->setWindowModality(Qt::ApplicationModal);
    createDialupConnection->setWindowTitle("Диалог создания Dialup соединения");
    createDialupConnection->openThis();
}

void RegistrationForm::setDataListConnection(QStringList list) {
    list.removeFirst();
    connectionList = list;

    ui->listWidgetConnectionList->clear();
    ui->listWidgetConnectionList->addItems(connectionList);
}

void RegistrationForm::showMe() {
    // Даем фокус на кнопку отменить
    //    ui->stackedWgtReg->setCurrentIndex(PageReg::SelectDevice);
    QVariantMap devices;
    getDeviceFrom_DB(devices);

    ui->cbxValidatorName->clear();
    ui->cbxValidatorName->addItems(validatorList);

    ui->cbxCoinAcceptorName->clear();
    ui->cbxCoinAcceptorName->addItems(coinAcceptorList);

    ui->cbxPrinterName->clear();
    ui->cbxPrinterName->addItems(printerList);

    ui->cbxWinprinter->clear();
    ui->cbxWinprinter->addItems(winprinterList);

    ui->cbxValidatorPort->clear();
    ui->cbxCoinAcceptorPort->clear();
    ui->cbxPrinterPort->clear();
    ui->cbxModemPort->clear();
    ui->cbxWatchdogPort->clear();

    com_PortList.prepend("");

    ui->cbxValidatorPort->addItems(com_PortList);
    ui->cbxCoinAcceptorPort->addItems(com_PortList);
    ui->cbxPrinterPort->addItems(com_PortList);
    ui->cbxModemPort->addItems(com_PortList);
    ui->cbxWatchdogPort->addItems(com_PortList);

    ui->cbxValidatorName->setCurrentText(devices.value("validator_name").toString());
    ui->cbxValidatorPort->setCurrentText(devices.value("validator_port").toString());

    ui->cbxCoinAcceptorName->setCurrentText(devices.value("coin_acceptor_name").toString());
    ui->cbxCoinAcceptorPort->setCurrentText(devices.value("coin_acceptor_port").toString());

    auto printerName = devices.value("printer_name").toString();
    ui->cbxPrinterName->setCurrentText(printerName);
    ui->cbxPrinterPort->setCurrentText(devices.value("printer_port").toString());
    ui->cbxWinprinter->setCurrentText(devices.value("printer_comment").toString());

    auto isWinprinter = printerName == "WindowsPrinter";

    ui->lblTitlePrinterPort->setVisible(!isWinprinter);
    ui->cbxPrinterPort->setVisible(!isWinprinter);
    ui->lblTitleWinprinter->setVisible(isWinprinter);
    ui->cbxWinprinter->setVisible(isWinprinter);

    ui->cbxWatchdogPort->setCurrentText(devices.value("watchdog_port").toString());

    ui->cbxModemPort->setCurrentText(devices.value("modem_port").toString());

    //    searchDevices->setCom_ListInfo(com_PortList);

    addPageData();
    showFullScreen();
}

bool RegistrationForm::getDeviceFrom_DB(QVariantMap &devices) {
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
        //        QString state = select.value(record.indexOf("state")).toString();

        switch (id) {
        case 1: {
            devices["validator_name"] = name;
            devices["validator_port"] = port;
        } break;
        case 2: {
            devices["printer_name"] = name;
            devices["printer_port"] = port;
            devices["printer_comment"] = comment;
        } break;
        case 3: {
            devices["modem_name"] = name;
            devices["modem_port"] = port;
        } break;
        case 4: {
            devices["watchdog_name"] = name;
            devices["watchdog_port"] = port;
        } break;
        case 5: {
            devices["coin_acceptor_name"] = name;
            devices["coin_acceptor_port"] = port;
        } break;
        default:
            break;
        }
    }

    return true;
}

void RegistrationForm::checkAuthInput(QString text) {
    Q_UNUSED(text)

    ui->btnNextReg->setEnabled(isValidAuthInput());
}

bool RegistrationForm::isValidAuthInput() {
    bool validAuthData =
        ui->editLoginReg->hasAcceptableInput() && ui->editOtpReg->hasAcceptableInput();
    bool validAuthAdminData = ui->editSecretLoginReg->hasAcceptableInput() &&
                              ui->editSecretPasswordReg->hasAcceptableInput() &&
                              ui->editSecretPasswordConfirmReg->hasAcceptableInput();

    return validAuthData && validAuthAdminData;
}

void RegistrationForm::tplCheck() {
    if (ui->radioTplTJK->isChecked()) {
        ui->tplImageTJK->setStyleSheet(tplStyleEnable);
        ui->tplImageUZB->setStyleSheet(tplStyleDisable);

        ui->btnNextReg->setEnabled(true);
    }

    if (ui->radioTplUZB->isChecked()) {
        ui->tplImageUZB->setStyleSheet(tplStyleEnable);
        ui->tplImageTJK->setStyleSheet(tplStyleDisable);

        ui->btnNextReg->setEnabled(true);
    }

    if (!ui->chbTest->isVisible()) {
        ui->chbTest->setVisible(true);
    }
}

void RegistrationForm::tplTJKSet() {
    ui->radioTplTJK->setChecked(true);
    tplCheck();
}

void RegistrationForm::tplUZBSet() {
    ui->radioTplUZB->setChecked(true);
    tplCheck();
}

void RegistrationForm::sleep(const int msec) {
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, &QEventLoop::quit);
    loop.exec();
}

void RegistrationForm::showKeyPud() {
    auto page = ui->stackedWgtReg->currentIndex();

    if (page == Page::SelectConnection && ui->radioDialUpConnection->isChecked()) {
        ui->wgtKeyPudReg->setVisible(true);
    } else if (page == Page::EnterAuthData) {
        ui->wgtKeyPudReg->setVisible(true);
    } else {
        ui->wgtKeyPudReg->setVisible(false);
    }
}

void RegistrationForm::sendCharacter(QChar character) {
    QPointer<QWidget> w = focusWidget();

    if (!w) {
        return;
    }

    int un = character.unicode();

    QString a = QString(character);
    if (un == 15405) {
        un = Qt::Key_Backspace;
        a = "";
    }

    if (un == 15934) {
        un = Qt::Key_Tab;
        a = "";
    }

    if (un == 15917) {
        un = Qt::Key_Enter;
        a = "";
    }

    if (un == 15420) {
        return;
    }

    QKeyEvent keyPress(QEvent::KeyPress, un, Qt::NoModifier, a);
    QApplication::sendEvent(w, &keyPress);
}

void RegistrationForm::secretPassView(bool show) {
    if (show) {
        ui->editSecretPasswordReg->setEchoMode(QLineEdit::Normal);
    } else {
        ui->editSecretPasswordReg->setEchoMode(QLineEdit::Password);
    }

    ui->editSecretPasswordReg->setFocus();
}

void RegistrationForm::secretPassConfirm_View(bool show) {
    if (show) {
        ui->editSecretPasswordConfirmReg->setEchoMode(QLineEdit::Normal);
    } else {
        ui->editSecretPasswordConfirmReg->setEchoMode(QLineEdit::Password);
    }

    ui->editSecretPasswordConfirmReg->setFocus();
}

void RegistrationForm::closeConfirm() {
    QMessageBox messageBox(this);
    messageBox.setWindowTitle(tr("Выход из программы."));
    messageBox.setText(tr("Вы действительно хотите выйти?<br/>"));

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QPushButton *yesButton = messageBox.addButton(tr("Да"), QMessageBox::AcceptRole);
    QPushButton *cancelButton = messageBox.addButton(tr("Отмена"), QMessageBox::RejectRole);
    messageBox.setDefaultButton(yesButton);
    messageBox.setEscapeButton(cancelButton);
#else
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Yes);
    messageBox.setEscapeButton(QMessageBox::Cancel);
    messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
    messageBox.setButtonText(QMessageBox::Cancel, tr("Отмена"));
#endif
    messageBox.setIcon(QMessageBox::Question);

    int r = messageBox.exec();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    if (r == QMessageBox::Accepted) {
#else
    if (r == QMessageBox::Yes) {
#endif
        closeMe();
    }
}

void RegistrationForm::closeMe() {
    this->close();
}
