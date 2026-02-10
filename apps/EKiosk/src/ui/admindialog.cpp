#include "admindialog.h"

#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore5Compat/QTextCodec>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>

#include "ui_admindialog.h"

AdminDialog::AdminDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AdminDialog) {
    ui->setupUi(this);

    KeyPud = new keyPud(this);
    ui->layoutWgtKeyPud->addWidget(KeyPud);
    connect(KeyPud, SIGNAL(characterGenerated(QChar)), this, SLOT(sendCharacter(QChar)));

    titleDataIncashment = "Не инкасированные платежи";

    //    connect(ui->stackedWidget,SIGNAL(currentChanged(int)),this,SLOT(steckerClicked(int)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(steckerClicked(int)));

    Debugger = 1;

    // Таймер закрытия
    closeTimer = new QTimer(this);
    closeTimer->setSingleShot(true);
    closeTimer->setInterval(180000);
    connect(closeTimer, SIGNAL(timeout()), this, SLOT(closeThis()));

    // Заполняем данные в странице перехода
    lstAdminListTitle << "Основная страница"
                      << "Параметры устройств"
                      << "Параметры соединения"
                      << "Параметры печати"
                      << "Остальные настройки"
                      << "Просмотр логов";

    // Получение баланса
    connect(ui->btnCheckBalance, SIGNAL(clicked()), SLOT(checkBalance()));

    mveAnimateGB = new QMovie(this);
    mveAnimateGB->setFileName(":/assets/images/load.gif");

    // Получение даты инкасации
    connect(ui->cbxAdminTextDateIncashment,
            SIGNAL(currentIndexChanged(QString)),
            this,
            SLOT(getCollectDate(QString)));

    // Выбор опций просмотра логов
    selectCategoryLogView = new SelectCategoryLogView(this);
    connect(selectCategoryLogView,
            SIGNAL(emit_SelectOptions(bool, bool, bool, bool, bool, bool, bool, bool, bool)),
            SLOT(SelectOptionsForSearch(bool, bool, bool, bool, bool, bool, bool, bool, bool)));

    // Создание нового соединения
    createDialupConnection = new CreateDialupConnection(this);
    connect(createDialupConnection,
            SIGNAL(emitDialupParam(QVariantMap)),
            SLOT(getDialupParam(QVariantMap)));

    adminButtons = new AdminButton(this);
    connect(adminButtons, SIGNAL(explorerCliked()), SLOT(showExplorer()));
    connect(adminButtons, SIGNAL(keyboardClicked()), SLOT(showKeyPud()));
    connect(adminButtons, SIGNAL(closeClicked()), SLOT(closeThis()));

    // Сделать инкасацию
    connect(ui->btnAdminCreateIncashment, SIGNAL(clicked()), SLOT(doCollectExec()));
    connect(ui->btnAdminCreatePreIncashment, SIGNAL(clicked()), SLOT(doCollectDateExec()));
    connect(ui->btnAdminTestRestartValidator, SIGNAL(clicked()), SLOT(restartValidator()));
    connect(ui->btnAdminTestPrintCheck, SIGNAL(clicked()), SLOT(printTestCheck()));
    connect(ui->btnAdminTestRestartModem, SIGNAL(clicked()), SLOT(restartModem()));
    connect(ui->btnRestartAppTypeDevices, SIGNAL(clicked()), SLOT(restartApp()));
    connect(ui->btnRestartTypeDevices, SIGNAL(clicked()), SLOT(restartASO()));
    connect(ui->btnPowerOffTypeDevices, SIGNAL(clicked()), SLOT(shutDounASO()));
    connect(ui->btnSaveChangeTypeDevices, SIGNAL(clicked()), SLOT(saveDeviceParam()));
    connect(ui->btnAdminCheckConnection, SIGNAL(clicked()), SLOT(checkConnection()));
    connect(ui->btnAdminGetDataFromModem, SIGNAL(clicked()), SLOT(getModem_DataInfo()));
    connect(ui->btnAdminSaveTerminalAccept, SIGNAL(clicked()), SLOT(saveTrm_AutorizationData()));
    connect(ui->btnAdminSelectLogParam, SIGNAL(clicked()), SLOT(openSelectCategory()));
    connect(ui->btnAdminUpdateLogData, SIGNAL(clicked()), SLOT(openLogInfoDate()));
    connect(ui->btnAdminNavigationUp, SIGNAL(clicked()), SLOT(go_to_up_log()));
    connect(ui->btnAdminNavigationDown, SIGNAL(clicked()), SLOT(go_to_doun_log()));
    connect(ui->btnAdminSearchKeyParam, SIGNAL(clicked()), SLOT(searchWithKeyParam()));
    connect(ui->btnAdminCreateNewConnections, SIGNAL(clicked()), SLOT(createNewConnection()));
    connect(ui->btnAdminGetActiveConnection, SIGNAL(clicked()), SLOT(getActiveRasCon()));
    connect(ui->btnAdminRestartNetwork, SIGNAL(clicked()), SLOT(cmdRestartNet()));
    connect(ui->btnAdminSaveConnectionParams, SIGNAL(clicked()), SLOT(saveConnectionParam()));
    connect(ui->btnAdminSavePrinterParam, SIGNAL(clicked()), SLOT(savePrinterParam()));
    connect(ui->btnAdminGetServicesData, SIGNAL(clicked()), SLOT(getServices()));
    connect(ui->btnAdminSaveTrmSettings2, SIGNAL(clicked()), SLOT(saveOtherSettings()));
    connect(ui->btnAdminSaveTerminaSecret, SIGNAL(clicked()), SLOT(saveUserAutorizationData()));
    connect(ui->btnAdminTestRestartModemReal, SIGNAL(clicked()), SLOT(restartModem()));
    connect(
        ui->cbxAdminNamePrinter, SIGNAL(currentIndexChanged(int)), SLOT(printerNameChanged(int)));

    hideObjects();

    ui->cbxAdminPortSpeed->setVisible(false);
    ui->lblAdminTitleSpeed->setVisible(false);
    ui->cbxAdminWinprinter->setVisible(false);
}

void AdminDialog::restartModem() {
    closeTimer->start();

    // ui->lblAdminInformationTest->setText("Начинаем перезагружать модем...");

    emit emit_execToMain(AdminCommand::aCmdRestartModem);
}

void AdminDialog::hideObjects() {
    ui->chbxAdminShowPrintDialog->setVisible(false);
    ui->cancel_PrintingChek->setVisible(false);
    ui->cbxPrintingChek->setVisible(false);

    ui->lblInfoAdminNumberNewPay->setVisible(false);
    ui->lblTextAdminNumberNewPay->setVisible(false);
    ui->grpAdminSetSettingsWD->setVisible(false);
    ui->grpAdminSearchDeviesParam->setVisible(false);

    ui->lblAdminTitleDevice2->setVisible(false);
    ui->lblAdminTitleDevice3->setVisible(false);
    ui->lblAdminTitleDevice5->setVisible(false);
    ui->lblAdminTitleDevice6->setVisible(false);
    ui->lblAdminTitleDevice8->setVisible(false);
    ui->lblAdminTitleDevice9->setVisible(false);
    ui->lblAdminTitleDevice11->setVisible(false);
    ui->lblAdminTitleDevice12->setVisible(false);

    ui->chbxAdminDoLockCodValidatorJam->setVisible(false);
    ui->chbxAdminSendErrorCodValidatorJam->setVisible(false);

    ui->cbxAdminSelectCountStatusJam->setVisible(false);

    ui->lblAdminResultCounterInfo->setVisible(false);
}

void AdminDialog::setLangList(QStringList langList) {
    ui->cbxAdminSelectMainLang->clear();
    ui->cbxAdminSelectMainLang->addItems(langList);
}

void AdminDialog::saveOtherSettings() {
    closeTimer->start();

    settings.clear();

    settings["status_validator_jam_in_box"] = ui->chbxAdminSendErrorCodValidatorJam->isChecked();
    settings["status_validator_jam_in_box_value_counter"] =
        ui->cbxAdminSelectCountStatusJam->currentText().trimmed();
    settings["status_validator_jam_in_box_lockers"] =
        ui->chbxAdminDoLockCodValidatorJam->isChecked();
    settings["default_lang"] = ui->cbxAdminSelectMainLang->currentText().trimmed();
    settings["auto_update_status"] = ui->chbxAdminTornOnAvtoUpdate->isChecked();
    settings["printing_chek"] = ui->cbxPrintingChek->currentIndex();

    if (ui->chbxAdminNominalDuplicate->isChecked()) {
        settings["lock_duplicate_nominal"] = false;
    }

    emit emit_execToMain(AdminCommand::aCmdSaveOtherSetting);
}

bool AdminDialog::unlockNominalDuplicate() {
    return ui->chbxAdminNominalDuplicate->isChecked();
}

void AdminDialog::getServices() {
    closeTimer->start();

    auto data = QVariantMap({{"message", "Начинаем получать конфигурацию..."}});
    setDataToAdmin(AdminCommand::aCmdInfoGetServices, data);

    emit emit_execToMain(AdminCommand::aCmdGetServices);
}

void AdminDialog::savePrinterParam() {
    closeTimer->start();

    settings.clear();
    settings["show_print_dialog"] = ui->chbxAdminShowPrintDialog->isChecked();
    settings["prt_win_width"] = ui->cbxAdminWidthWinprtCheck->currentText().trimmed().toInt();
    settings["prt_win_height"] = ui->cbxAdminHeightWinprtCheck->currentText().trimmed().toInt();
    settings["prt_win_font_size"] = ui->cbxAdminFontWinprtCheck->currentText().trimmed().toInt();
    settings["prt_win_left_margin"] =
        ui->cbxAdminLeftMarginWinprtCheck->currentText().trimmed().toInt();
    settings["prt_win_right_margin"] =
        ui->cbxAdminRightMarginWinprtCheck->currentText().trimmed().toInt();
    settings["prt_win_top_margin"] =
        ui->cbxAdminTopMarginWinprtCheck->currentText().trimmed().toInt();
    settings["prt_win_bottom_margin"] =
        ui->cbxAdminBottomMarginWinprtCheck->currentText().trimmed().toInt();
    settings["chek_small_beetwen_string"] = ui->chbxAdminSmallLineText->isChecked();
    settings["chek_small_text"] = ui->chbxAdminSmallCheck->isChecked();
    settings["chek_width"] = ui->cbxAdminWidthCheck->currentText().trimmed().toInt();
    settings["chek_left_size"] = ui->cbxAdminLeftMarginCheck->currentText().trimmed().toInt();
    settings["exist_counter_chek"] = ui->chbxAdminCounterExist->isChecked();
    settings["counter_len_rulon"] = ui->editAdminLengthOfRulon->text().trimmed();
    settings["counter_len_chek"] = ui->editAdminLengthOfCheck->text().trimmed();
    settings["counter_ring_value"] = ui->editAdminLengthOfCounter->text().trimmed().toInt();
    settings["exist_counter_printer_chek"] = ui->chbxAdminCounterPrinterExist->isChecked();
    settings["printing_chek"] = ui->cbxPrintingChek->currentIndex();

    emit emit_execToMain(AdminCommand::aCmdSavePrinterParam);
}

void AdminDialog::saveConnectionParam() {
    closeTimer->start();

    QString phoneNumber = ui->editSmsNumberPhone->text().trimmed();

    if (ui->chbxAdminSMSerrValidator->isChecked() && phoneNumber.length() != 9) {
        QMessageBox messageBox(this);
        messageBox.setWindowTitle("Сохранение параметров.");
        messageBox.setText("Номер телефона состоит из 9 цифр.\n");
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.move(500, 200);

        messageBox.exec();

        return;
    }

    settings.clear();
    settings["vpn_point"] = ui->cbxAdminConnectionList->currentText().trimmed();
    settings["ras_error_interval_reboot"] =
        ui->cbxAdminRasErrorReboot->currentText().trimmed().toInt();
    settings["check_balance_sim"] = ui->chbxAdminCheckBalanceSim->isChecked();
    settings["check_number_sim"] = ui->chbxAdminCheckNumberSim->isChecked();
    settings["ussd_balance_sim"] = ui->cbxAdminRequestBalanceSim->currentText().trimmed();
    settings["index_check_balance"] = ui->cbxAdminPositionBalanceSim->currentText().trimmed();
    settings["ussd_number_sim"] = ui->cbxAdminRequestNumberSim->currentText().trimmed();
    settings["sms_send_number"] = phoneNumber;
    settings["sms_err_validator"] = ui->chbxAdminSMSerrValidator->isChecked();
    settings["printing_chek"] = ui->cbxPrintingChek->currentIndex();

    emit emit_execToMain(AdminCommand::aCmdSaveConnParam);
}

void AdminDialog::cmdRestartNet() {
    closeTimer->start();
    emit emit_execToMain(AdminCommand::aCmdRestartDialupCon);
}

void AdminDialog::getActiveRasCon() {
    closeTimer->start();

    auto data = QVariantMap({{"message", "Проверяем активное соединение..."}});
    setDataToAdmin(AdminCommand::aCmdConnectInfo, data);

    emit emit_execToMain(AdminCommand::aCmdGetActiveDialup);
}

void AdminDialog::showMsgDialog(QString title, QString text) {
    QMessageBox messageBox(this);
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.setIcon(QMessageBox::Information);
    messageBox.move(500, 200);

    messageBox.exec();
}

void AdminDialog::getDialupParam(QVariantMap val) {
    data.clear();
    data = val;
    emit emit_execToMain(AdminCommand::aCmdRasConnCreate);
}

void AdminDialog::createNewConnection() {
    createDialupConnection->conList = connListInfData;
    createDialupConnection->devList = dialupDevice;

    createDialupConnection->setWindowModality(Qt::ApplicationModal);
    createDialupConnection->setWindowTitle("Диалог создания Dialup соединения");
    createDialupConnection->openThis();
}

void AdminDialog::SelectOptionsForSearch(bool SelectValidatorJam,
                                         bool SelectMoneyOut,
                                         bool SelectERROR,
                                         bool SelectPayDaemon,
                                         bool SelectStatusAso,
                                         bool SelectStatusPrinter,
                                         bool SelectStatusValidator,
                                         bool SelectConnectionState,
                                         bool SelectUpdater) {
    closeTimer->start();

    QString searchKey;
    if (SelectValidatorJam) {
        searchKey += "MONEY_JAM;";
    }
    if (SelectMoneyOut) {
        searchKey += "MONEY_OUT;";
    }
    if (SelectERROR) {
        searchKey += "ERROR;";
    }
    if (SelectPayDaemon) {
        searchKey += "PAY_DAEMONS;";
    }
    if (SelectStatusAso) {
        searchKey += "STATUS_DAEMONS;";
    }
    if (SelectStatusPrinter) {
        searchKey += "PRINTER;";
    }
    if (SelectStatusValidator) {
        searchKey += "VALIDATOR;";
    }
    if (SelectConnectionState) {
        searchKey += "CONNECTION;";
    }
    if (SelectUpdater) {
        searchKey += "UPDATER;";
    }

    if (searchKey.right(1) == ";") {
        searchKey = searchKey.left(searchKey.length() - 1);
    }

    // Вставляем в поле поиска
    ui->editSearchKeyParam->setText(searchKey);

    // Нажимаем на кнопку поиск
    ui->btnAdminSearchKeyParam->click();
}

void AdminDialog::searchWithKeyParam() {
    closeTimer->start();

    QStringList lstLog;
    QString string;
    getLogDataFrom_File(lstLog, string);

    // Попробуем посмотреть что в поле поиска
    QString searchString = ui->editSearchKeyParam->text();
    if (searchString != "") {
        string = "";
        QStringList searchList = searchString.split(";");
        // Делаем поиск
        for (int i = 0; i < lstLog.count(); i++) {
            QString line = lstLog.at(i);
            bool add = false;
            for (int j = 0; j < searchList.count(); j++) {
                if (line.contains(searchList.at(j))) {
                    if (!add) {
                        string += line + "\n";
                        add = true;
                    } else {
                        break;
                    }
                }
            }
        }
    }

    ui->editBrowserLogView->setText(string);
}

void AdminDialog::openLogInfoDate() {
    closeTimer->start();

    // Очищаем поле поиска
    ui->editSearchKeyParam->setText("");

    // Устанавливаем сегодняшную дату
    ui->dEditAdminSelectDataLog->setDate(QDate::currentDate());

    QStringList lstLog;
    QString string;
    getLogDataFrom_File(lstLog, string);

    ui->editBrowserLogView->setText(string);

    QTimer::singleShot(500, this, SLOT(go_to_end_text_edit()));
}

void AdminDialog::getLogDataFrom_File(QStringList &logLst, QString &all) {
    logLst.clear();
    all = "";
    // дата создания
    QString vrm_Date = ui->dEditAdminSelectDataLog->text();
    QByteArray ba;
    QFile fileInfo(QString("log/%1.txt").arg(vrm_Date));

    if (!fileInfo.open(QIODevice::ReadOnly)) {
        ba = QString("File Log Not Present %1").arg(vrm_Date).toLatin1();
    } else {
        ba.append(fileInfo.readAll());
    }

    QTextCodec *codec = QTextCodec::codecForLocale();
    all = codec->toUnicode(ba);
    logLst = all.split("\n");
    fileInfo.close();
}

void AdminDialog::go_to_end_text_edit() {

    QScrollBar *scrollBar = ui->editBrowserLogView->verticalScrollBar();
    scrollBar->setSliderPosition(scrollBar->maximum());
}

void AdminDialog::go_to_up_log() {
    closeTimer->start();
    QScrollBar *scrollBar = ui->editBrowserLogView->verticalScrollBar();
    int pos = scrollBar->sliderPosition() - 500;
    scrollBar->setSliderPosition(pos);
}

void AdminDialog::go_to_doun_log() {
    closeTimer->start();
    QScrollBar *scrollBar = ui->editBrowserLogView->verticalScrollBar();
    int pos = scrollBar->sliderPosition() + 500;
    scrollBar->setSliderPosition(pos);
}

void AdminDialog::openSelectCategory() {
    closeTimer->start();
    selectCategoryLogView->setWindowModality(Qt::ApplicationModal);
    //    selectCategoryLogView->setWindowFlags(Qt::WindowTitleHint);
    selectCategoryLogView->setWindowTitle("Выберите опции для просмотра");
    selectCategoryLogView->show();
}

void AdminDialog::saveTrm_AutorizationData() {
    closeTimer->start();

    QString login = ui->editAdminLoginTrm->text().trimmed();
    QString password = ui->editAdminPassTrm->text().trimmed();

    if (login == "" || password == "") {
        QMessageBox messageBox1(this);
        messageBox1.setWindowTitle("Сохранение параметров.");
        messageBox1.setText("Проверьте правильность ввода данных\n");
        messageBox1.setStandardButtons(QMessageBox::Ok);
        messageBox1.setDefaultButton(QMessageBox::Ok);
        messageBox1.setIcon(QMessageBox::Warning);
        messageBox1.move(500, 200);

        messageBox1.exec();

        return;
    }

    settings.clear();
    settings["login"] = login;
    settings["token"] = password;

    authButtonSet(false);

    emit emit_execToMain(AdminCommand::aCmdSaveTrm_Num_Sett);
}

void AdminDialog::authButtonSet(bool enable) {
    ui->btnAdminSaveTerminalAccept->setEnabled(enable);
}

void AdminDialog::saveUserAutorizationData() {
    closeTimer->start();

    QString secretLogin = ui->editAdminSecretLogin->text().trimmed();
    QString secretPassword = ui->editAdminSecretPass->text().trimmed();

    if (secretLogin == "" || secretPassword == "") {
        QMessageBox messageBox1(this);
        messageBox1.setWindowTitle("Сохранение параметров.");
        messageBox1.setText("Проверьте правильность ввода данных\n");
        messageBox1.setStandardButtons(QMessageBox::Ok);
        messageBox1.setDefaultButton(QMessageBox::Ok);
        messageBox1.setIcon(QMessageBox::Warning);
        messageBox1.move(500, 200);

        messageBox1.exec();

        return;
    }

    settings.clear();
    settings["secret_login"] = secretLogin;
    settings["secret_password"] = secretPassword;

    emit emit_execToMain(AdminCommand::aCmdSaveUserAvtoriza);
}

void AdminDialog::checkConnection() {
    closeTimer->start();

    auto data = QVariantMap({{"message", "Начинаем проверять соединение..."}});
    setDataToAdmin(AdminCommand::aCmdConnectInfo, data);

    emit emit_execToMain(AdminCommand::aCmdCheckConnect);
}

void AdminDialog::getModem_DataInfo() {
    closeTimer->start();

    if (ui->cbxAdminConnectionList->currentText().toUpper() != "LOCAL CONNECTION") {
        // Спрашиваем хочет ли он сохранить данные
        QMessageBox messageBox1(this);
        messageBox1.setWindowTitle("Выполнение параметров.");
        messageBox1.setText("Перед проверкой параметров SIM карты\n"
                            "программа отключит соединение с сервером\n"
                            "вы хотите проверить параметры SIM карты?\n");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        QPushButton *yesButton = messageBox1.addButton(QString("Да"), QMessageBox::AcceptRole);
        QPushButton *cancelButton =
            messageBox1.addButton(QString("Отмена"), QMessageBox::RejectRole);
        messageBox1.setDefaultButton(yesButton);
        messageBox1.setEscapeButton(cancelButton);
#else
        messageBox1.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        messageBox1.setDefaultButton(QMessageBox::Yes);
        messageBox1.setEscapeButton(QMessageBox::Cancel);
        messageBox1.setButtonText(QMessageBox::Yes, QString("Да"));
        messageBox1.setButtonText(QMessageBox::Cancel, QString("Отмена"));
#endif
        messageBox1.setIcon(QMessageBox::Question);
        messageBox1.move(500, 200);

        // Необходимо удалить данный платеж
        int rr = messageBox1.exec();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        if (rr == QMessageBox::Accepted) {
#else
        if (rr == QMessageBox::Yes) {
#endif

            // Надо дать команду сохранение с перезагрузкой
            auto data = QVariantMap({{"message", "Начинаем проверять данные SIM карты..."}});

            setDataToAdmin(AdminCommand::aCmdConnectInfo, data);

            emit emit_execToMain(AdminCommand::aCmdGetSim_Info);
        }
    } else {
        showMsgDialog("Админ панель", "Используется локальное соединение");
    }
}

void AdminDialog::restartApp() {
    closeTimer->start();
    // Спрашиваем хочет ли он сохранить данные
    QMessageBox messageBox1(this);
    messageBox1.setWindowTitle("Выполнение параметров.");
    messageBox1.setText("Вы действительно хотите перезагрузить программу\n");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QPushButton *yesButton = messageBox1.addButton(QString("Да"), QMessageBox::AcceptRole);
    QPushButton *cancelButton = messageBox1.addButton(QString("Отмена"), QMessageBox::RejectRole);
    messageBox1.setDefaultButton(yesButton);
    messageBox1.setEscapeButton(cancelButton);
#else
    messageBox1.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    messageBox1.setDefaultButton(QMessageBox::Yes);
    messageBox1.setEscapeButton(QMessageBox::Cancel);
    messageBox1.setButtonText(QMessageBox::Yes, QString("Да"));
    messageBox1.setButtonText(QMessageBox::Cancel, QString("Отмена"));
#endif
    messageBox1.setIcon(QMessageBox::Question);
    messageBox1.move(500, 200);

    // Необходимо удалить данный платеж
    int rr = messageBox1.exec();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    if (rr == QMessageBox::Accepted) {
#else
    if (rr == QMessageBox::Yes) {
#endif

        // Надо дать команду сохранение с перезагрузкой
        emit emit_execToMain(AdminCommand::aCmdRestartApp);
    }
}

void AdminDialog::restartASO() {
    closeTimer->start();
    // Спрашиваем хочет ли он сохранить данные
    QMessageBox messageBox1(this);
    messageBox1.setWindowTitle("Выполнение параметров.");
    messageBox1.setText("Вы действительно хотите перезагрузить Терминал\n");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QPushButton *yesButton = messageBox1.addButton(QString("Да"), QMessageBox::AcceptRole);
    QPushButton *cancelButton = messageBox1.addButton(QString("Отмена"), QMessageBox::RejectRole);
    messageBox1.setDefaultButton(yesButton);
    messageBox1.setEscapeButton(cancelButton);
#else
    messageBox1.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    messageBox1.setDefaultButton(QMessageBox::Yes);
    messageBox1.setEscapeButton(QMessageBox::Cancel);
    messageBox1.setButtonText(QMessageBox::Yes, QString("Да"));
    messageBox1.setButtonText(QMessageBox::Cancel, QString("Отмена"));
#endif
    messageBox1.setIcon(QMessageBox::Question);
    messageBox1.move(500, 200);

    // Необходимо удалить данный платеж
    int rr = messageBox1.exec();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    if (rr == QMessageBox::Accepted) {
#else
    if (rr == QMessageBox::Yes) {
#endif

        // Надо дать команду сохранение с перезагрузкой
        emit emit_execToMain(AdminCommand::aCmdRestartASO);
    }
}

void AdminDialog::saveDeviceParam() {
    closeTimer->start();
    auto cmd = AdminCommand::aCmdSaveDeviceParam;

    settings.clear();
    settings["validator_name"] = ui->cbxAdminNameValidator->currentText();
    settings["validator_port"] = ui->cbxAdminPortValidator->currentText();
    settings["coin_acceptor_name"] = ui->cbxAdminNameAcceptor->currentText();
    settings["coin_acceptor_port"] = ui->cbxAdminPortAcceptor->currentText();

    auto printerName = ui->cbxAdminNamePrinter->currentText();
    settings["printer_name"] = printerName;
    settings["printer_comment"] =
        printerName == "WindowsPrinter" ? ui->cbxAdminWinprinter->currentText() : "";
    settings["printer_port"] = ui->cbxAdminPortPrinter->currentText();
    settings["printer_port_speed"] = ui->cbxAdminPortSpeed->currentIndex();

    settings["modem_port"] = ui->cbxAdminPortModem->currentText();
    settings["watchdog_port"] = ui->cbxAdminPortWD->currentText();

    if (settings != deviceParam) {

        // Спрашиваем хочет ли он сохранить данные
        QMessageBox messageBox1(this);
        messageBox1.setWindowTitle("Сохранение параметров.");
        messageBox1.setText("Для того чтобы изменённые данные\n"
                            "вступили в силу необходимо перезагрузить программу");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        QPushButton *yesButton = messageBox1.addButton(QString("Да"), QMessageBox::AcceptRole);
        QPushButton *cancelButton =
            messageBox1.addButton(QString("Отмена"), QMessageBox::RejectRole);
        messageBox1.setDefaultButton(yesButton);
        messageBox1.setEscapeButton(cancelButton);
#else
        messageBox1.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        messageBox1.setDefaultButton(QMessageBox::Yes);
        messageBox1.setEscapeButton(QMessageBox::Cancel);
        messageBox1.setButtonText(QMessageBox::Yes, QString("Да"));
        messageBox1.setButtonText(QMessageBox::Cancel, QString("Отмена"));
#endif
        messageBox1.setIcon(QMessageBox::Question);
        messageBox1.move(500, 200);

        int rr = messageBox1.exec();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        if (rr == QMessageBox::Accepted) {
#else
        if (rr == QMessageBox::Yes) {
#endif

            // Надо дать команду сохранение с перезагрузкой
            cmd = AdminCommand::aCmdSaveDeviceParam_R;
        } else if (rr == QMessageBox::Cancel) {

            ui->cbxAdminNameValidator->setCurrentText(
                deviceParam.value("validator_name").toString());
            ui->cbxAdminPortValidator->setCurrentText(
                deviceParam.value("validator_port").toString());

            ui->cbxAdminNameAcceptor->setCurrentText(
                deviceParam.value("coin_acceptor_name").toString());
            ui->cbxAdminPortAcceptor->setCurrentText(
                deviceParam.value("coin_acceptor_port").toString());

            ui->cbxAdminNamePrinter->setCurrentText(deviceParam.value("printer_name").toString());
            ui->cbxAdminPortPrinter->setCurrentText(deviceParam.value("printer_port").toString());

            ui->cbxAdminPortSpeed->setCurrentIndex(deviceParam["printer_comment"].toInt());
            ui->cbxAdminPortModem->setCurrentText(deviceParam.value("modem_port").toString());
            ui->cbxAdminPortWD->setCurrentText(deviceParam.value("watchdog_port").toString());

            ui->cbxAdminWinprinter->setCurrentText(deviceParam["printer_comment"].toString());
            settings.clear();
            return;
        }
    }

    settings["search_validator"] = ui->chbxAdminSearchValidator->isChecked();
    settings["search_coin_acceptor"] = ui->chbxAdminSearchCoinAcceptor->isChecked();
    settings["search_printer"] = ui->chbxAdminSearchPrinter->isChecked();
    settings["search_modem"] = ui->chbxAdminSearchModem->isChecked();
    settings["search_watchdog"] = ui->chbxAdminSearchWD->isChecked();

    emit emit_execToMain(cmd);
}

void AdminDialog::shutDounASO() {
    closeTimer->start();
    // Спрашиваем хочет ли он сохранить данные
    QMessageBox messageBox1(this);
    messageBox1.setWindowTitle("Выполнение параметров.");
    messageBox1.setText("Вы действительно выключить терминал?\n");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QPushButton *yesButton = messageBox1.addButton(QString("Да"), QMessageBox::AcceptRole);
    QPushButton *cancelButton = messageBox1.addButton(QString("Отмена"), QMessageBox::RejectRole);
    messageBox1.setDefaultButton(yesButton);
    messageBox1.setEscapeButton(cancelButton);
#else
    messageBox1.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    messageBox1.setDefaultButton(QMessageBox::Yes);
    messageBox1.setEscapeButton(QMessageBox::Cancel);
    messageBox1.setButtonText(QMessageBox::Yes, QString("Да"));
    messageBox1.setButtonText(QMessageBox::Cancel, QString("Отмена"));
#endif
    messageBox1.setIcon(QMessageBox::Question);
    messageBox1.move(500, 200);

    // Необходимо удалить данный платеж
    int rr = messageBox1.exec();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    if (rr == QMessageBox::Accepted) {
#else
    if (rr == QMessageBox::Yes) {
#endif

        // Надо дать команду сохранение с перезагрузкой
        emit emit_execToMain(AdminCommand::aCmdShutDounASO);
    }
}

void AdminDialog::printTestCheck() {
    closeTimer->start();

    if (ui->cbxAdminNamePrinter->currentText() != "") {
        printerName = ui->cbxAdminNamePrinter->currentText();
        printerPort = ui->cbxAdminPortPrinter->currentText();
        printerPortSpeed = QString("%1").arg(ui->cbxAdminPortSpeed->currentIndex());
        printerComment =
            printerName == "WindowsPrinter" ? ui->cbxAdminWinprinter->currentText() : "";

        emit emit_execToMain(AdminCommand::aCmdPrintTestCheck);
    }
}

void AdminDialog::restartValidator() {
    closeTimer->start();

    emit emit_execToMain(AdminCommand::aCmdRestartValidator);
}

void AdminDialog::hideExplorer() {
    closeTimer->start();
    emit emit_execToMain(AdminCommand::aCmdHideExplorer);
}

void AdminDialog::showExplorer() {
    closeTimer->start();
    emit emit_execToMain(AdminCommand::aCmdShowExplorer);
}

void AdminDialog::showKeyPud() {
    closeTimer->start();
    emit emit_execToMain(AdminCommand::aCmdShowKeyPud);
}

void AdminDialog::doCollectExec() {
    closeTimer->start();
    emit emit_execToMain(AdminCommand::aCmdExecIncashmant);
}

void AdminDialog::doCollectDateExec() {
    closeTimer->start();
    QString vrm_Date = ui->cbxAdminTextDateIncashment->currentText();

    if (vrm_Date != titleDataIncashment) {
        dateCollectParam = vrm_Date;
        emit emit_execToMain(AdminCommand::aCmdExecDateIncash);
    } else {
        QMessageBox msgBox(this);

        msgBox.move(500, 200);

        msgBox.setText("Для распечатки чека\n"
                       "за предыдущие инкасации\n"
                       "выберите её в списке \"Дата инкасации:\".");

        msgBox.exec();
    }
}

void AdminDialog::closeEvent(QCloseEvent *event) {
    emit emit_unlockOpenAdminSts();
    event->accept();
}

void AdminDialog::getCollectDate(QString date) {
    dateCollectParam = date;

    emit emit_execToMain(AdminCommand::aCmdHtmlIncash);
}

void AdminDialog::setDataToAdmin(int cmd, QVariantMap data) {
    switch (cmd) {
    case AdminCommand::aCmdGetBalance: {
        mveAnimateGB->stop();
        //            QMovie *movie;
        ui->lblAnimateAdminBalance->setVisible(false);
        //            ui->lblAnimateAdminBalance->setMovie(movie);
        ui->lblAnimateAdminBalance->setText("---");

        // Вставляем значение баланса
        ui->lblAdminTextCurentBalance->setText(data.value("balance").toString());
        // Вставляем значение овердрафта
        ui->lblAdminTextOverdraft->setText(data.value("overdraft").toString());
        // Порог отключения
        ui->lblAdminTextPorocOtclucheniya->setText(data.value("threshold").toString());
    } break;
    case AdminCommand::aCmdGetNewOperation: {
        // Новые платежи
        ui->lblTextAdminNumberNewPay->setText(data.value("payment_count").toString());
    } break;
    case AdminCommand::aCmdListAllIncash: {
        // Делаем список инкассаций по датам
        ui->cbxAdminTextDateIncashment->clear();
        ui->cbxAdminTextDateIncashment->addItems(data.value("encash_list").toStringList());
    } break;
    case AdminCommand::aCmdHtmlIncash: {
        auto html = data.value("html").toString();
        auto cId = data.value("c_id").toString();
        auto cTrn = data.value("c_trn").toString();
        auto trnFrom = data.value("trn_from").toString();
        auto trnTo = data.value("trn_to").toString();

        // Делаем HTML инкассаций
        ui->textBrowser->setHtml(html);
        ui->lblAdminTextNumberIncashment->setText(cId);
        ui->lblAdminTextIdIncashment->setText(cTrn);
        ui->lblAdminTextTranzactionPay->setText(
            QString("%1<font color=\"red\">/</font>%2").arg(trnFrom, trnTo));
    } break;
    case AdminCommand::aCmdShowAsoStatus: {
        // Показываем статус асо
        // ui->lblAdminGlobalStatus->setText(data.at(0));
    } break;
    case AdminCommand::aCmdValidatorInform: {
        // Показываем информацию о валидаторе
        ui->lblAdminTextDeviceInf1->setText(data.value("validator_info").toString());

        // Список Валидаторов
        ui->cbxAdminNameValidator->clear();
        ui->cbxAdminNameValidator->addItems(data.value("validator_list").toStringList());

        // Устанавливаем текущий
        ui->cbxAdminNameValidator->setCurrentText(data.value("validator_name").toString());

        deviceParam.clear();
        deviceParam["validator_name"] = ui->cbxAdminNameValidator->currentText();

        // Список сом портов
        ui->cbxAdminPortValidator->clear();
        ui->cbxAdminPortValidator->addItems(data.value("validator_port_list").toStringList());

        // Текущий порт
        int index = ui->cbxAdminPortValidator->findText(data.value("validator_port").toString());
        ui->cbxAdminPortValidator->setCurrentIndex(index);

        deviceParam["validator_port"] = ui->cbxAdminPortValidator->currentText();
    } break;
    case AdminCommand::aCmdCoinAcceptorInf: {
        // Показываем информацию о монетоприемнике
        ui->lblAdminTextAcceptor->setText(data.value("coin_acceptor_info").toString());

        // Список Валидаторов
        ui->cbxAdminNameAcceptor->clear();
        ui->cbxAdminNameAcceptor->addItems(data.value("coin_acceptor_list").toStringList());

        // Устанавливаем текущий
        ui->cbxAdminNameAcceptor->setCurrentText(data.value("coin_acceptor_name").toString());

        deviceParam["coin_acceptor_name"] = ui->cbxAdminNameAcceptor->currentText();

        // Список сом портов
        ui->cbxAdminPortAcceptor->clear();
        ui->cbxAdminPortAcceptor->addItems(data.value("coin_acceptor_port_list").toStringList());

        // Текущий порт
        int index = ui->cbxAdminPortAcceptor->findText(data.value("coin_acceptor_port").toString());
        ui->cbxAdminPortAcceptor->setCurrentIndex(index);

        deviceParam["coin_acceptor"] = ui->cbxAdminPortAcceptor->currentText();
    } break;
    case AdminCommand::aCmdPrinterInform: {
        // Показываем информацию
        auto printerComment = data.value("printer_comment").toString();
        ui->lblAdminTextDeviceInf2->setText(printerComment);
        ui->cbxAdminPortSpeed->setCurrentIndex(printerComment.toInt());

        // Список
        ui->cbxAdminNamePrinter->clear();
        ui->cbxAdminNamePrinter->addItems(data.value("printer_list").toStringList());

        // Win принтеры
        ui->cbxAdminWinprinter->clear();
        ui->cbxAdminWinprinter->addItems(data.value("winprinter_list").toStringList());

        // Устанавливаем текущий
        auto printerName = data.value("printer_name").toString();
        ui->cbxAdminNamePrinter->setCurrentText(printerName);

        ui->cbxAdminWinprinter->setCurrentText(printerComment);

        ui->gbxAdminWindowsPrinterParam->setVisible(printerName == "WindowsPrinter");

        deviceParam["printer_name"] = ui->cbxAdminNamePrinter->currentText();
        deviceParam["printer_comment"] = printerComment;

        // Список сом портов
        ui->cbxAdminPortPrinter->clear();
        ui->cbxAdminPortPrinter->addItems(data.value("printer_port_list").toStringList());

        // Текущий порт
        int index = ui->cbxAdminPortPrinter->findText(data.value("printer_port").toString());
        ui->cbxAdminPortPrinter->setCurrentIndex(index);

        deviceParam["printer_port"] = ui->cbxAdminPortPrinter->currentText();
        deviceParam["printer_port_speed"] = ui->cbxAdminPortSpeed->currentIndex();
    } break;
    case AdminCommand::aCmdModem_Inform: {
        // Показываем информацию
        ui->lblAdminTextDeviceInf3->setText(data.value("modem_comment").toString());
        ui->lblAdminTextDeviceInf4->setText(data.value("modem_name").toString());

        // Список сом портов
        ui->cbxAdminPortModem->clear();
        ui->cbxAdminPortModem->addItems(data.value("modem_port_list").toStringList());

        // Текущий порт
        int index = ui->cbxAdminPortModem->findText(data.value("modem_port").toString());
        ui->cbxAdminPortModem->setCurrentIndex(index);

        deviceParam["modem_port"] = ui->cbxAdminPortModem->currentText();
    } break;
    case AdminCommand::aCmdWDInform: {
        // Показываем информацию
        ui->lblAdminTextDeviceInfWD1->setText(data.value("watchdog_comment").toString());
        ui->lblAdminTextDeviceInfWD2->setText(data.value("watchdog_name").toString());

        // Список сом портов
        ui->cbxAdminPortWD->clear();
        ui->cbxAdminPortWD->addItems(data.value("watchdog_port_list").toStringList());

        // Текущий порт
        int index = ui->cbxAdminPortWD->findText(data.value("watchdog_port").toString());
        ui->cbxAdminPortWD->setCurrentIndex(index);

        deviceParam["watchdog_port"] = ui->cbxAdminPortWD->currentText();
    } break;
    case AdminCommand::aCmdSearchParam_Ref: {
        ui->chbxAdminSearchValidator->setChecked(data.value("search_validator").toBool());
        ui->chbxAdminSearchPrinter->setChecked(data.value("search_printer").toBool());
        ui->chbxAdminSearchModem->setChecked(data.value("search_modem").toBool());
        ui->chbxAdminSearchWD->setChecked(data.value("search_watchdog").toBool());
    } break;
    case AdminCommand::aCmdInfrmationPanel: {
        //            ui->lblAdminInformationTest->setText(data.value("message").toString());
    } break;
    case AdminCommand::aCmdSim_InfoData: {
        ui->lblAdminTextPrvSim->setText(data.value("modem_sim_provider").toString());
        ui->lblAdminTextNumberSim->setText(data.value("modem_sim_number").toString());
        ui->lblAdminTextUrovanSignala->setText(data.value("modem_sim_rate").toString());
        ui->lblAdminTextBalanceSim->setText(data.value("modem_sim_balance").toString());
    } break;
    case AdminCommand::aCmdRasConnlist: {
        // Список Соединений
        // Для проверки при создание соединения
        connListInfData.clear();
        connListInfData = data.value("connection_list").toStringList();

        ui->cbxAdminConnectionList->clear();
        ui->cbxAdminConnectionList->addItems(connListInfData);

        // Текущий порт соединения
        ui->cbxAdminConnectionList->setCurrentText(data.value("vpn_point").toString());
    } break;
    case AdminCommand::aCmdErrorRasReb: {
        auto val = data.value("ras_error_interval_reboot").toString();
        ui->cbxAdminRasErrorReboot->setCurrentText(val);
    } break;
    case AdminCommand::aCmdConnectInfo: {
        auto msg = data.value("message").toString();
        ui->lblAdminInfoGetDataFromModem->setText(msg);
    } break;
    case AdminCommand::aCmdModem_InfData: {
        ui->chbxAdminCheckBalanceSim->setChecked(data.value("check_balance_sim").toBool());
        ui->chbxAdminCheckNumberSim->setChecked(data.value("check_number_sim").toBool());
        ui->cbxAdminRequestBalanceSim->setCurrentText(data.value("ussd_balance_sim").toString());
        ui->cbxAdminPositionBalanceSim->setCurrentText(
            data.value("index_check_balance").toString());
        ui->cbxAdminRequestNumberSim->setCurrentText(data.value("ussd_number_sim").toString());
    } break;
    case AdminCommand::aCmdPrinterInfData: {
        ui->chbxAdminShowPrintDialog->setChecked(data.value("show_print_dialog").toBool());
        ui->chbxAdminSmallLineText->setChecked(data.value("chek_small_beetwen_string").toBool());
        ui->chbxAdminSmallCheck->setChecked(data.value("chek_small_text").toBool());
        ui->cbxAdminWidthCheck->setCurrentText(data.value("chek_width").toString());
        ui->cbxAdminLeftMarginCheck->setCurrentText(data.value("chek_left_size").toString());
        ui->cbxPrintingChek->setCurrentIndex(data.value("printing_chek").toInt());
    } break;
    case AdminCommand::aCmdSmsSending: {
        ui->editSmsNumberPhone->setText(data.value("sms_send_number").toString());
        ui->chbxAdminSMSerrValidator->setChecked(data.value("sms_err_validator").toBool());
    } break;
    case AdminCommand::aCmdWinPrinterParam: {
        ui->cbxAdminWidthWinprtCheck->setCurrentText(data.value("prt_win_width").toString());
        ui->cbxAdminHeightWinprtCheck->setCurrentText(data.value("prt_win_height").toString());
        ui->cbxAdminFontWinprtCheck->setCurrentText(data.value("prt_win_font_size").toString());
        ui->cbxAdminLeftMarginWinprtCheck->setCurrentText(
            data.value("prt_win_left_margin").toString());
        ui->cbxAdminRightMarginWinprtCheck->setCurrentText(
            data.value("prt_win_right_margin").toString());
        ui->cbxAdminTopMarginWinprtCheck->setCurrentText(
            data.value("prt_win_top_margin").toString());
        ui->cbxAdminBottomMarginWinprtCheck->setCurrentText(
            data.value("prt_win_bottom_margin").toString());
    } break;
    case AdminCommand::aCmdCounterCheckInf: {
        ui->chbxAdminCounterExist->setChecked(data.value("exist_counter_chek").toBool());
        ui->editAdminLengthOfRulon->setText(data.value("counter_len_rulon").toString());
        ui->editAdminLengthOfCheck->setText(data.value("counter_len_chek").toString());
        ui->editAdminLengthOfCounter->setText(data.value("counter_ring_value").toString());
    } break;
    case AdminCommand::aCmdCounterCheckVal: {
        ui->lblAdminResultCounterInfo->setText(data.value("counter_info").toString());
    } break;
    case AdminCommand::aCmdAvtorizationTrm_P: {
        auto login = data.value("login").toString();
        auto secretLogin = data.value("secret_login").toString();

        ui->editAdminLoginTrm->setText(login);
        ui->editAdminSecretLogin->setText(secretLogin);
    } break;
    case AdminCommand::aCmdOtherSettings: {
        ui->chbxAdminSendErrorCodValidatorJam->setChecked(
            data.value("status_validator_jam_in_box").toBool());
        ui->chbxAdminDoLockCodValidatorJam->setChecked(
            data.value("status_validator_jam_in_box_lockers").toBool());
        ui->cbxAdminSelectCountStatusJam->setCurrentText(
            data.value("status_validator_jam_in_box_value_counter").toString());
        ui->cbxAdminSelectMainLang->setCurrentText(data.value("default_lang").toString());
        ui->chbxAdminTornOnAvtoUpdate->setChecked(data.value("auto_update_status").toBool());
        ui->chbxAdminCounterPrinterExist->setChecked(
            data.value("exist_counter_printer_chek").toBool());
        ui->chbxAdminNominalDuplicate->setChecked(!data.value("lock_duplicate_nominal").toBool());
    } break;
    case AdminCommand::aCmdInfoGetServices: {
        auto msg = data.value("message").toString();
        ui->lblAdminServicesDataInfo->setText(msg);
    } break;
    default:
        break;
    }
}

void AdminDialog::checkBalance() {
    closeTimer->start();
    mveAnimateGB->start();
    ui->lblAnimateAdminBalance->setVisible(true);
    ui->lblAnimateAdminBalance->setMovie(mveAnimateGB);

    emit emit_execToMain(AdminCommand::aCmdGetBalance);
}

AdminDialog::~AdminDialog() {
    delete ui;
}

void AdminDialog::steckerClicked(int stk) {
    closeTimer->start();

    // Тут можно делать проверки какая страница нажата
    switch (stk) {
    case AdminLisTitle::lSettingsLogInfo: {
        openLogInfoDate();
    } break;
    default:
        break;
    }

    // Тут отображаем тайтл
    //    ui->grpBoxAdminText->setTitle(lstAdminListTitle.at(stk));
}

void AdminDialog::closeThis() {
    closeTimer->stop();

    if (selectCategoryLogView->isVisible()) {
        selectCategoryLogView->close();
    }

    if (createDialupConnection->isVisible()) {
        createDialupConnection->close();
    }

    this->close();
}

void AdminDialog::openThis() {
    closeTimer->start();

    this->showFullScreen();

    if (width() < 1200) {
        ui->wgtAdminBaground->layout()->setContentsMargins(6, 250, 6, 2);
    }

    adminButtons->move(width() - adminButtons->width() - 10, 8);

    ui->editAdminPassTrm->setText("");
    authButtonSet(true);

    ui->tabWidget->setCurrentIndex(0);
}

void AdminDialog::sendCharacter(QChar character) {
    bool a_key = false;
    QPointer<QWidget> w = focusWidget();

    if (!w) {
        return;
    }

    int un = character.unicode();

    QString a = QString(character);

    if (un == 15405) {
        a_key = true;
        un = Qt::Key_Backspace;
        a = "";
    }

    if (un == 15934) {
        a_key = true;
        un = Qt::Key_Tab;
        a = "";
    }

    if (un == 15917) {
        a_key = true;
        un = Qt::Key_Enter;
        a = "";
    }

    if (un == 15420) {
        //        un = Qt::Key_Backspace;
        a = "";
        // clearTextIn();
        return;
    }

    if (a_key) {
        QKeyEvent keyPress(QEvent::KeyPress, un, Qt::NoModifier, a);
        QApplication::sendEvent(w, &keyPress);
    } else {
        if (ui->editAdminLoginTrm->hasFocus() || ui->editAdminPassTrm) {
            QKeyEvent keyPress(QEvent::KeyPress, un, Qt::NoModifier, a);
            QApplication::sendEvent(w, &keyPress);
        }
    }
}

void AdminDialog::printerNameChanged(int index) {
    auto show_port_speed = index == 7;

    ui->cbxAdminPortSpeed->setVisible(show_port_speed);
    ui->lblAdminTitleSpeed->setVisible(show_port_speed);

    auto isWinprinter = index == 8;

    ui->cbxAdminPortPrinter->setVisible(!isWinprinter);
    ui->cbxAdminWinprinter->setVisible(isWinprinter);
}
