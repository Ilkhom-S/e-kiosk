#include "avtorizationtoadminin.h"

#include <QtCore/QTextStream>
#include <QtCore/QTimer>

#include "ui_avtorizationtoadminin.h"

AvtorizationToAdminIn::AvtorizationToAdminIn(QWidget *parent)
    : QDialog(parent), ui(new Ui::AvtorizationToAdminIn), KeyPud(new keyPud(this)), countCheckIn(0) {
    ui->setupUi(this);

    
    ui->layoutWgtKeyPud->addWidget(KeyPud);

    AvtorizationToAdminIn::connect(KeyPud, SIGNAL(characterGenerated(QChar)), this, SLOT(sendCharacter(QChar)));

    

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        QPushButton *okButton = messageBox1.addButton(QString("OK"), QMessageBox::AcceptRole);
        messageBox1.setDefaultButton(okButton);
#else
        messageBox1.setStandardButtons(QMessageBox::Ok);
        messageBox1.setDefaultButton(QMessageBox::Ok);
        messageBox1.setButtonText(QMessageBox::Ok, QString("OK"));
#endif
        messageBox1.setIcon(QMessageBox::Warning);

        messageBox1.exec();

        return;
    }

    if (vrmLogin == this->loginIn && vrmPass == this->passIn) {
        countCheckIn = 0;
        emit this->emit_openAdminDialog();
        this->close();
        return;
    }         countCheckIn++;

        QMessageBox messageBox1(this);
        messageBox1.setWindowTitle("Диалог авторизации");
        messageBox1.setText("Неверно введены параметры авторизации");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        QPushButton *okButton = messageBox1.addButton(QString("OK"), QMessageBox::AcceptRole);
        messageBox1.setDefaultButton(okButton);
#else
        messageBox1.setStandardButtons(QMessageBox::Ok);
        messageBox1.setDefaultButton(QMessageBox::Ok);
        messageBox1.setButtonText(QMessageBox::Ok, QString("OK"));
#endif
        messageBox1.setIcon(QMessageBox::Critical);

        messageBox1.exec();
  
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
