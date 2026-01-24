// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextStream>
#include <Common/QtHeadersEnd.h>

// Project
#include "registrationdialog.h"
#include "ui_registrationdialog.h"

RegistrationDialog::RegistrationDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegistrationDialog) {
    ui->setupUi(this);

    // Установка кодировки для интерфейса
    //    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));

    KeyPud = new keyPud(this);
    ui->layoutWgtKeyPud->addWidget(KeyPud);

    this->connect(KeyPud, SIGNAL(characterGenerated(QChar)), this, SLOT(sendCharacter(QChar)));

    this->connect(ui->btnTabReg1, SIGNAL(clicked()), this, SLOT(tabToRegistration1()));
    this->connect(ui->btnTabReg2, SIGNAL(clicked()), this, SLOT(tabToRegistration2()));

    ui->btnSaveReg1->click();
    // Надо поставить регулярные выражения
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    // Номер терминала
    QRegularExpressionValidator *NumTrmReg =
        new QRegularExpressionValidator(QRegularExpression("[1-9][0-9]{4,5}"), ui->editNumTrmReg);
    ui->editNumTrmReg->setValidator(NumTrmReg);

    // Логин пользователя
    QRegularExpressionValidator *loginRegValidator =
        new QRegularExpressionValidator(QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->editLoginReg);
    ui->editLoginReg->setValidator(loginRegValidator);

    // Пароль пользователя
    QRegularExpressionValidator *passRegValidator =
        new QRegularExpressionValidator(QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->editPasswordReg);
    ui->editPasswordReg->setValidator(passRegValidator);

    QRegularExpressionValidator *confPassRegValidator =
        new QRegularExpressionValidator(QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->editPasswordReg);
    ui->editConfirmPasswordReg->setValidator(confPassRegValidator);

    // Секретный номер
    // QRegExpValidator* secretNumValidator = new
    // QRegExpValidator(QRegExp("[1-9][0-9]{1,20}"), ui->editSecretNumReg);
    // ui->editSecretNumReg->setValidator(secretNumValidator);

    // Логин пользователя
    QRegularExpressionValidator *secLoginRegValidator =
        new QRegularExpressionValidator(QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->editSecretLoginReg);
    ui->editSecretLoginReg->setValidator(secLoginRegValidator);

    // Пароль пользователя
    QRegularExpressionValidator *secPassRegValidator =
        new QRegularExpressionValidator(QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->editSecretPasswordReg);
    ui->editSecretPasswordReg->setValidator(secPassRegValidator);

    QRegularExpressionValidator *secConfPassRegValidator = new QRegularExpressionValidator(
        QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->editSecretConfirmPasswordReg);
    ui->editSecretConfirmPasswordReg->setValidator(secConfPassRegValidator);

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    movie = new QMovie(this);

    // Сохранить данные пользователя
    this->connect(ui->btnSaveReg1, SIGNAL(clicked()), this, SLOT(save1BtnReg()));

    this->connect(ui->btnSaveReg2, SIGNAL(clicked()), this, SLOT(save2BtnReg()));

    // Закрываем форму
    //    this->connect(ui->btnCancelReg1,SIGNAL(clicked()),this,SLOT(closeForm()));
    //    this->connect(ui->btnCancelReg2,SIGNAL(clicked()),this,SLOT(closeForm()));

    // Создание нового соединения
    createDialupConnection = new CreateDialupConnection(this);
    connect(createDialupConnection, SIGNAL(emitDialupParam(QVariantMap)), SIGNAL(emitCreateNewConnection(QVariantMap)));

    connect(ui->btnAdminCreateNewConnection, SIGNAL(clicked()), SLOT(createNewConnection()));

    connect(ui->btnRegCloseThis, SIGNAL(clicked()), SLOT(closeForm()));

    connect(ui->btnAdminSaveConnectionParam, SIGNAL(clicked()), SLOT(save3BtnReg()));

    connect(ui->btnTest, SIGNAL(clicked()), SLOT(btnTesTclc()));

    ui->tbnAddTerminalData->setTabEnabled(3, false);

    connect(ui->btnBack, SIGNAL(clicked()), SLOT(btnBackClc()));

    // connect(ui->btnStartTest,SIGNAL(clicked()),SLOT(starttest()));

    connect(ui->tbnAddTerminalData, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    ui->btnNumTrmRegClear->setVisible(false);
    ui->btnLoginRegClear->setVisible(false);
    ui->btnPasswordRegClear->setVisible(false);
    ui->btnConfirmPasswordRegClear->setVisible(false);
    ui->btnTest->setVisible(false);

    ui->editNumTrmReg->setFocus();
}

void RegistrationDialog::tabChanged(int page) {
    switch (page) {
        case 0:
            ui->layoutWgtKeyPud->addWidget(KeyPud);
            break;
        case 1:
            ui->layoutWgtKeyPud_2->addWidget(KeyPud);
            break;
        case 2:
            ui->layoutWgtKeyPud_3->addWidget(KeyPud);
            break;
        case 3:
            break;
    }
}

void RegistrationDialog::setDataListConnection(QStringList list) {
    connListInfData.clear();
    connListInfData = list;

    ui->cbxAdminConnectionList->clear();
    ui->cbxAdminConnectionList->addItems(connListInfData);
}

void RegistrationDialog::showMsgDialog(QString title, QString text) {

    QMessageBox messageBox1(this);
    messageBox1.setWindowTitle(title);
    messageBox1.setText(text);
    messageBox1.setStandardButtons(QMessageBox::Ok);
    messageBox1.setDefaultButton(QMessageBox::Ok);
    messageBox1.setIcon(QMessageBox::Information);
    messageBox1.setButtonText(QMessageBox::Ok, QString("OK"));

    messageBox1.exec();

    return;
}

void RegistrationDialog::createNewConnection() {
    createDialupConnection->conList = connListInfData;
    createDialupConnection->devList = dialupDevice;

    createDialupConnection->setWindowModality(Qt::ApplicationModal);
    createDialupConnection->setWindowTitle("Диалог создания Dialup соединения");
    createDialupConnection->openThis();
}

RegistrationDialog::~RegistrationDialog() {
    delete ui;
}

void RegistrationDialog::closeForm() {
    // Спрашиваем хочет ли он сохранить данные
    QMessageBox messageBox1(this);
    messageBox1.setWindowTitle("Закрыть приложение.");
    messageBox1.setText("Вы действительно хотите закрыть приложение?");
    messageBox1.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    messageBox1.setDefaultButton(QMessageBox::Yes);
    messageBox1.setEscapeButton(QMessageBox::Cancel);
    messageBox1.setIcon(QMessageBox::Question);
    messageBox1.setButtonText(QMessageBox::Yes, QString("Да"));
    messageBox1.setButtonText(QMessageBox::Cancel, QString("Отмена"));

    int rr = messageBox1.exec();

    if (rr == QMessageBox::Yes) {

        // Тут надо сохранить данные.
        this->close();

        return;
    } else if (rr == QMessageBox::Cancel) {

        // Тут ни чего не делаем

        return;
    }
}

void RegistrationDialog::save1BtnReg() {
    qDebug() << "save1BtnReg";

    // Проверяем все условия
    if (ui->editNumTrmReg->hasAcceptableInput() && ui->editLoginReg->hasAcceptableInput() &&
        ui->editPasswordReg->hasAcceptableInput() && ui->editConfirmPasswordReg->hasAcceptableInput() &&
        (ui->editPasswordReg->text() == ui->editConfirmPasswordReg->text())) {
        // Запись в переменные

        qDebug() << "save1BtnReg - true";
        this->gblNumTrm = ui->editNumTrmReg->text();
        this->gblLogin = ui->editLoginReg->text();
        this->gblPass = ui->editPasswordReg->text();

        // Переходим на второй шаг
        ui->tbnAddTerminalData->setCurrentIndex(1);

        return;
    } else {
        qDebug() << "save1BtnReg - false";
        // Что та не заполнина
        if (!ui->editNumTrmReg->hasAcceptableInput()) {
            ui->editNumTrmReg->setToolTip("Номер терминала состоит из 5-6 цифр.");
            //            ui->editNumTrmReg->setPlaceholderText("Номер терминала
            //            состоит из 5-6 цифр.");
        } else {
            ui->editNumTrmReg->setToolTip("Номер терминала введен верно.");
            //            ui->editNumTrmReg->setPlaceholderText("Номер терминала
            //            введен верно.");
        }

        if (!ui->editLoginReg->hasAcceptableInput()) {
            ui->editLoginReg->setToolTip("Логин состоит из 1-30 цифр/букв/знаков.");
            //            ui->editLoginReg->setPlaceholderText("Логин состоит из 1-30
            //            цифр/букв/знаков.");
        } else {
            ui->editLoginReg->setToolTip("Логин введен верно.");
            //            ui->editLoginReg->setPlaceholderText("Логин введен верно.");
        }

        if (ui->editPasswordReg->text() != ui->editConfirmPasswordReg->text()) {
            ui->editPasswordReg->setToolTip("Пароли не совпадают.");
            //            ui->editPasswordReg->setPlaceholderText("Пароли не
            //            совпадают.");
        } else {
            ui->editPasswordReg->setToolTip("Пароли совпадают.");
            //            ui->editPasswordReg->setPlaceholderText("Пароли
            //            совпадают.");
        }
        if (ui->editPasswordReg->text() == "") {
            ui->editPasswordReg->setToolTip("Пароль состоит из 1-30 цифр/букв/знаков.");
            //            ui->editPasswordReg->setPlaceholderText("Пароль состоит из
            //            1-30 цифр/букв/знаков.");
        } else {
            ui->editPasswordReg->setToolTip("Пароль введен верно.");
            //            ui->editPasswordReg->setPlaceholderText("Пароль введен
            //            верно.");
        }

        if (ui->editConfirmPasswordReg->text() == "") {
            ui->editConfirmPasswordReg->setToolTip("Пароль состоит из 1-30 цифр/букв/знаков.");
            //            ui->editConfirmPasswordReg->setPlaceholderText("Пароль
            //            состоит из 1-30 цифр/букв/знаков.");
        } else {
            ui->editConfirmPasswordReg->setToolTip("Пароль введен верно.");
            //            ui->editConfirmPasswordReg->setPlaceholderText("Пароль
            //            введен верно.");
        }
        return;
    }
}

void RegistrationDialog::save2BtnReg() {
    // Проверяем все условия
    if (ui->editNumTrmReg->hasAcceptableInput() && ui->editLoginReg->hasAcceptableInput() &&
        ui->editPasswordReg->hasAcceptableInput() && ui->editConfirmPasswordReg->hasAcceptableInput() &&
        (ui->editPasswordReg->text() == ui->editConfirmPasswordReg->text())) {
        // Запись в переменные
        this->gblNumTrm = ui->editNumTrmReg->text();
        this->gblLogin = ui->editLoginReg->text();
        this->gblPass = ui->editPasswordReg->text();
    } else {
        // Что та не заполнина
        if (!ui->editNumTrmReg->hasAcceptableInput()) {
            ui->editNumTrmReg->setToolTip("Номер терминала состоит из 5-6 цифр.");
            //            ui->editNumTrmReg->setPlaceholderText("Номер терминала
            //            состоит из 5-6 цифр.");
        } else {
            ui->editNumTrmReg->setToolTip("Номер терминала введен верно.");
            //            ui->editNumTrmReg->setPlaceholderText("Номер терминала
            //            введен верно.");
        }

        if (!ui->editLoginReg->hasAcceptableInput()) {
            ui->editLoginReg->setToolTip("Логин состоит из 1-30 цифр/букв/знаков.");
            //            ui->editLoginReg->setPlaceholderText("Логин состоит из 1-30
            //            цифр/букв/знаков.");
        } else {
            ui->editLoginReg->setToolTip("Логин введен верно.");
            //            ui->editLoginReg->setPlaceholderText("Логин введен верно.");
        }

        if (ui->editPasswordReg->text() != ui->editConfirmPasswordReg->text()) {
            ui->editPasswordReg->setToolTip("Пароли не совпадают.");
            //            ui->editPasswordReg->setPlaceholderText("Пароли не
            //            совпадают.");
        } else {
            ui->editPasswordReg->setToolTip("Пароли совпадают.");
            //            ui->editPasswordReg->setPlaceholderText("Пароли
            //            совпадают.");
        }
        if (ui->editPasswordReg->text() == "") {
            ui->editPasswordReg->setToolTip("Пароль состоит из 1-30 цифр/букв/знаков.");
            //            ui->editPasswordReg->setPlaceholderText("Пароль состоит из
            //            1-30 цифр/букв/знаков.");
        } else {
            ui->editPasswordReg->setToolTip("Пароль введен верно.");
            //            ui->editPasswordReg->setPlaceholderText("Пароль введен
            //            верно.");
        }

        if (ui->editConfirmPasswordReg->text() == "") {
            ui->editConfirmPasswordReg->setToolTip("Пароль состоит из 1-30 цифр/букв/знаков.");
            //            ui->editConfirmPasswordReg->setPlaceholderText("Пароль
            //            состоит из 1-30 цифр/букв/знаков.");
        } else {
            ui->editConfirmPasswordReg->setToolTip("Пароль введен верно.");
            //            ui->editConfirmPasswordReg->setPlaceholderText("Пароль
            //            введен верно.");
        }

        // Переходим на второй шаг
        ui->tbnAddTerminalData->setCurrentIndex(0);

        return;
    }

    // Проверяем все условия
    if (ui->editSecretLoginReg->hasAcceptableInput() && ui->editSecretPasswordReg->hasAcceptableInput() &&
        ui->editSecretConfirmPasswordReg->hasAcceptableInput() &&
        (ui->editSecretPasswordReg->text() == ui->editSecretConfirmPasswordReg->text())) {
        // Запись в переменные
        // this->gblSecretNum      = ui->editSecretNumReg->text();
        this->gblSecretLogin = ui->editSecretLoginReg->text();
        this->gblSecretPass = ui->editSecretPasswordReg->text();

        // Тут переходим на страницу праметров соединения
        ui->tbnAddTerminalData->setCurrentIndex(2);
    } else {
        // Что та не заполнина
        //        if(!ui->editSecretNumReg->hasAcceptableInput()){
        //            ui->editSecretNumReg->setToolTip("Секретный номер состоит из
        //            3-20 цифр.");
        //            ui->editSecretNumReg->setPlaceholderText("Секретный номер
        //            состоит из 3-20 цифр.");
        //        }

        if (!ui->editSecretLoginReg->hasAcceptableInput()) {
            ui->editSecretLoginReg->setToolTip("Логин состоит из 1-30 цифр/букв/знаков.");
            //            ui->editSecretLoginReg->setPlaceholderText("Логин состоит из
            //            1-30 цифр/букв/знаков.");
        }

        if (ui->editSecretPasswordReg->text() != ui->editSecretConfirmPasswordReg->text()) {
            ui->editSecretPasswordReg->setToolTip("Пароли не совпадают.");
            //            ui->editSecretPasswordReg->setPlaceholderText("Пароли не
            //            совпадают.");
        }
        if (ui->editSecretPasswordReg->text() == "") {
            ui->editSecretPasswordReg->setToolTip("Секретный пароль состоит из 1-30 цифр/букв/знаков.");
            //            ui->editSecretPasswordReg->setPlaceholderText("Секретный
            //            пароль состоит из 1-30 цифр/букв/знаков.");
        }
        if (ui->editSecretConfirmPasswordReg->text() == "") {
            ui->editSecretConfirmPasswordReg->setToolTip("Секретный пароль состоит из 1-30 цифр/букв/знаков.");
            //            ui->editSecretConfirmPasswordReg->setPlaceholderText("Секретный
            //            пароль состоит из 1-30 цифр/букв/знаков.");
        }
        return;
    }
}

void RegistrationDialog::save3BtnReg() {
    QVariantMap data;
    data["terminal_number"] = gblNumTrm;
    data["login"] = gblLogin;
    data["password"] = gblPass;
    data["admin_login"] = gblSecretLogin;
    data["admin_password"] = gblSecretPass;
    data["connection"] = ui->cbxAdminConnectionList->currentText().trimmed();
    data["sim_balance_req"] = ui->cbxAdminRequestBalanceSim->currentText().trimmed();
    data["sim_balance_position"] = ui->cbxAdminPositionBalanceSim->currentText().trimmed();
    data["sim_number_req"] = ui->cbxAdminRequestNumberSim->currentText().trimmed();
    data["tpl"] = ui->radioTplUzb->isChecked() ? "uzb" : "tjk";

    // Спрашиваем хочет ли он сохранить данные
    QMessageBox messageBox1(this);
    messageBox1.setWindowTitle("Сохранение данных.");
    messageBox1.setText("Вы действительно хотите сохранить данные?");
    messageBox1.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    messageBox1.setDefaultButton(QMessageBox::Yes);
    messageBox1.setEscapeButton(QMessageBox::Cancel);
    messageBox1.setIcon(QMessageBox::Question);
    messageBox1.setButtonText(QMessageBox::Yes, QString("Да"));
    messageBox1.setButtonText(QMessageBox::Cancel, QString("Отмена"));

    // Необходимо удалить данный платеж
    int rr = messageBox1.exec();

    if (rr == QMessageBox::Yes) {
        emit emitRegistrationData(data);
    }
}

void RegistrationDialog::btnTesTclc() {
    ui->tbnAddTerminalData->setCurrentIndex(3);
    ui->tbnAddTerminalData->setTabEnabled(3, true);
    ui->tbnAddTerminalData->setTabEnabled(0, false);
    ui->tbnAddTerminalData->setTabEnabled(1, false);
    ui->tbnAddTerminalData->setTabEnabled(2, false);

    this->starttest();
}

void RegistrationDialog::starttest() {
    // Тут бере значение из и сохраняем
    QString conName = ui->cbxAdminConnectionList->currentText().trimmed();
    QString balanceReq = ui->cbxAdminRequestBalanceSim->currentText().trimmed();
    QString positionReq = ui->cbxAdminPositionBalanceSim->currentText().trimmed();
    QString numberReq = ui->cbxAdminRequestNumberSim->currentText().trimmed();

    ui->lblValidatorText->setText("");
    ui->lblPrinterText->setText("");
    ui->lblWatchdogText->setText("");
    ui->lblModemText->setText("");
    ui->lblSearchDevStatusTest->setText("");
    ui->lblConnecStatusTest->setText("");
    ui->lblAvtorizationStatusTest->setText("");

    emit emitStartTest(this->gblNumTrm, this->gblLogin, this->gblPass, this->gblSecretLogin, this->gblSecretPass,
                       conName, balanceReq, positionReq, numberReq);
}

void RegistrationDialog::btnBackClc() {
    ui->tbnAddTerminalData->setCurrentIndex(2);
    ui->tbnAddTerminalData->setTabEnabled(3, false);
    ui->tbnAddTerminalData->setTabEnabled(0, true);
    ui->tbnAddTerminalData->setTabEnabled(1, true);
    ui->tbnAddTerminalData->setTabEnabled(2, true);

    ui->lblValidatorText->setText("");
    ui->lblPrinterText->setText("");
    ui->lblWatchdogText->setText("");
    ui->lblModemText->setText("");
    ui->lblSearchDevStatusTest->setText("");
    ui->lblConnecStatusTest->setText("");
    ui->lblAvtorizationStatusTest->setText("");
}

void RegistrationDialog::tabToRegistration1() {
    if (ui->editNumTrmReg->hasFocus()) {
        ui->editLoginReg->setFocus();
        return;
    }
    if (ui->editLoginReg->hasFocus()) {
        ui->editPasswordReg->setFocus();
        return;
    }
    if (ui->editPasswordReg->hasFocus()) {
        ui->editConfirmPasswordReg->setFocus();
        return;
    }

    ui->editNumTrmReg->setFocus();
    return;
}

void RegistrationDialog::tabToRegistration2() {
    //    if(ui->editSecretNumReg->hasFocus()){
    //        ui->editSecretLoginReg->setFocus(Qt::TabFocusReason);
    //        return;
    //    }
    if (ui->editSecretLoginReg->hasFocus()) {
        ui->editSecretPasswordReg->setFocus(Qt::TabFocusReason);
        return;
    }
    if (ui->editSecretPasswordReg->hasFocus()) {
        ui->editSecretConfirmPasswordReg->setFocus(Qt::TabFocusReason);
        return;
    }

    //    ui->editSecretNumReg->setFocus(Qt::TabFocusReason);
    //    return;
}

void RegistrationDialog::setValidatorText(QString status) {
    ui->lblValidatorText->setText(status);
}

void RegistrationDialog::setPrinterText(QString status) {
    ui->lblPrinterText->setText(status);
}
void RegistrationDialog::setWatchdogText(QString status) {
    ui->lblWatchdogText->setText(status);
}
void RegistrationDialog::setModemText(QString status) {
    ui->lblModemText->setText(status);
}
void RegistrationDialog::logForTest(int sts, QString log) {
    switch (sts) {
        case 1:
            ui->lblSearchDevStatusTest->setText(log);
            break;

        case 2:
            ui->lblConnecStatusTest->setText(log);
            break;
        case 3:
            ui->lblAvtorizationStatusTest->setText(log);
            break;
    }
    if (ui->lblAvtorizationStatusTest->text() == "")
        ui->lblAvtorizationStatusTest->setVisible(false);
    else
        ui->lblAvtorizationStatusTest->setVisible(true);
}

void RegistrationDialog::LoadingAnim(bool start) {

    movie->setFileName("assets/images/testing.gif");

    if (start) {
        ui->lblloadingTest->setMovie(movie);
        // ui->btnStartTest->setEnabled(false);
        ui->btnBack->setEnabled(false);
        movie->start();
    } else {
        movie->stop();
        ui->lblloadingTest->clear();
        // ui->btnStartTest->setEnabled(true);
        ui->btnBack->setEnabled(true);
    }
}

void RegistrationDialog::sendCharacter(QChar character) {
    QPointer<QWidget> w = focusWidget();

    if (!w)
        return;

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

void RegistrationDialog::on_btnNumTrmRegClear_clicked() {

    //    if(ui->editNumTrmReg->hasFocus()){

    //        ui->editNumTrmReg.
    //        KeyPud->clickBackspace();
    //        return;
    //    }
    //    if(ui->editLoginReg->hasFocus()){
    //        ui->editPasswordReg->setFocus();
    //        return;
    //    }
    //    if(ui->editPasswordReg->hasFocus()){
    //        ui->editConfirmPasswordReg->setFocus();
    //        return;
    //    }

    //    ui->editNumTrmReg->setFocus();
    //    return;
    //    qDebug()<<"adasd";
    //    if(ui->editNumTrmReg->hasFocus()){
    //        ui->editNumTrmReg->setFocus();
    //    }
}
