// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// Project
#include "avtorizationtoadminin.h"
#include "ui_avtorizationtoadminin.h"

AvtorizationToAdminIn::AvtorizationToAdminIn(QWidget *parent) : QDialog(parent), ui(new Ui::AvtorizationToAdminIn) {
    ui->setupUi(this);

    KeyPud = new keyPud(this);
    ui->layoutWgtKeyPud->addWidget(KeyPud);

    this->connect(KeyPud, SIGNAL(characterGenerated(QChar)), this, SLOT(sendCharacter(QChar)));

    countCheckIn = 0;

    connect(ui->btnEnterAdmin, SIGNAL(clicked()), SLOT(checkInputData()));

    //    connect(ui->btnCancelReg1,SIGNAL(clicked()),SLOT(close()));
}

void AvtorizationToAdminIn::checkInputData() {
    if (countCheckIn > 5) {
        countCheckIn = 0;
        this->close();
    }

    QString vrmLogin = ui->editLoginReg->text().trimmed();
    QString vrmPass = ui->editPasswordReg->text().trimmed();

    if (vrmLogin == "" || vrmPass == "") {

        countCheckIn++;

        QMessageBox messageBox1(this);
        messageBox1.setWindowTitle("Диалог авторизации");
        messageBox1.setText("Введите параметры авторизации");
        messageBox1.setStandardButtons(QMessageBox::Ok);
        messageBox1.setDefaultButton(QMessageBox::Ok);
        messageBox1.setIcon(QMessageBox::Warning);
        messageBox1.setButtonText(QMessageBox::Ok, QString("OK"));

        messageBox1.exec();

        return;
    }

    if (vrmLogin == this->loginIn && vrmPass == this->passIn) {
        countCheckIn = 0;
        emit this->emit_openAdminDialog();
        this->close();
        return;
    } else {
        countCheckIn++;

        QMessageBox messageBox1(this);
        messageBox1.setWindowTitle("Диалог авторизации");
        messageBox1.setText("Неверно введены параметры авторизации");
        messageBox1.setStandardButtons(QMessageBox::Ok);
        messageBox1.setDefaultButton(QMessageBox::Ok);
        messageBox1.setIcon(QMessageBox::Critical);
        messageBox1.setButtonText(QMessageBox::Ok, QString("OK"));

        messageBox1.exec();

        return;
    }
}

AvtorizationToAdminIn::~AvtorizationToAdminIn() {
    delete ui;
}

void AvtorizationToAdminIn::setAuthParam(QString login, QString pass) {
    loginIn = login;
    passIn = pass;

    ui->editLoginReg->setText("");
    ui->editPasswordReg->setText("");

    ui->editLoginReg->setFocus();

    QTimer::singleShot(180000, this, SLOT(close()));
}

void AvtorizationToAdminIn::sendCharacter(QChar character) {
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
