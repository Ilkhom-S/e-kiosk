// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPointer>
#include <QtGui/QKeyEvent>
#include <QtGui/QRegularExpressionValidator>
#include <QtWidgets/QMessageBox>
#include <Common/QtHeadersEnd.h>

// Project
#include "changepassword.h"
#include "ui_changepassword.h"

ChangePassword::ChangePassword(QWidget *parent) : QDialog(parent), ui(new Ui::ChangePassword)
{
    ui->setupUi(this);

    QRegularExpressionValidator *secLoginRegValidator =
        new QRegularExpressionValidator(QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->newLogin);
    ui->newLogin->setValidator(secLoginRegValidator);

    QRegularExpressionValidator *secPassRegValidator =
        new QRegularExpressionValidator(QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->newPassword);
    ui->newPassword->setValidator(secPassRegValidator);

    QRegularExpressionValidator *secRepeatPassRegValidator =
        new QRegularExpressionValidator(QRegularExpression("[\\S\\w\\W\\d\\D]{1,30}"), ui->RepeatPass);
    ui->RepeatPass->setValidator(secRepeatPassRegValidator);

    KeyPud = new keyPud(this);
    ui->layoutWgtKeyPud->addWidget(KeyPud);
    this->connect(KeyPud, SIGNAL(characterGenerated(QChar)), this, SLOT(sendCharacter(QChar)));

    connect(ui->btn_ok, SIGNAL(clicked()), SLOT(btnok_clc()));
    connect(ui->btn_cancel, SIGNAL(clicked()), SLOT(btncancel_clc()));
}

ChangePassword::~ChangePassword()
{
    delete ui;
}
void ChangePassword::btnok_clc()
{

    QString login = ui->newLogin->text().trimmed();
    QString password = ui->newPassword->text().trimmed();
    QString password2 = ui->RepeatPass->text().trimmed();

    if (ui->newLogin->text().isEmpty() || ui->newPassword->text().isEmpty() || ui->RepeatPass->text().isEmpty())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Ошибка!!!");
        msgBox.setText("Заполните все поля!");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
        msgBox.exec();
        return;
    }

    if (password != password2)
    {
        QMessageBox msgBox2(this);
        msgBox2.setWindowTitle("Ошибка!!!");
        msgBox2.setText("Пароль несовпадает!");
        msgBox2.setIcon(QMessageBox::Critical);
        msgBox2.setWindowFlags(Qt::WindowStaysOnTopHint);
        msgBox2.exec();
    }
    else
    {
        QMessageBox msgBox3(this);

        msgBox3.setWindowTitle("Успешно!!!");
        msgBox3.setText("Логин и пароль установлены!");
        msgBox3.setIcon(QMessageBox::Information);
        msgBox3.setWindowFlags(Qt::WindowStaysOnTopHint);
        msgBox3.exec();

        ui->newLogin->clear();
        ui->newPassword->clear();
        ui->RepeatPass->clear();

        emit emit_changepass(login, password);

        this->close();
    }
}

void ChangePassword::btncancel_clc()
{
    this->close();
    ui->newLogin->clear();
    ui->newPassword->clear();
    ui->RepeatPass->clear();
}

void ChangePassword::sendCharacter(QChar character)
{
    QPointer<QWidget> w = focusWidget();

    if (!w)
        return;

    int un = character.unicode();

    QString a = QString(character);
    if (un == 15405)
    {
        un = Qt::Key_Backspace;
        a = "";
    }
    if (un == 15934)
    {
        un = Qt::Key_Tab;
        a = "";
    }
    if (un == 15917)
    {
        un = Qt::Key_Enter;
        a = "";
    }
    if (un == 15420)
    {
        return;
    }

    QKeyEvent keyPress(QEvent::KeyPress, un, Qt::NoModifier, a);
    QApplication::sendEvent(w, &keyPress);
}
