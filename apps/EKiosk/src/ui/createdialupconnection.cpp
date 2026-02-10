#include "createdialupconnection.h"

#include <QtWidgets/QMessageBox>

#include "ui_createdialupconnection.h"

CreateDialupConnection::CreateDialupConnection(QWidget *parent)
    : QDialog(parent), ui(new Ui::CreateDialupConnection) {
    ui->setupUi(this);

    connect(ui->btnDialupCreateNewCon, SIGNAL(clicked()), SLOT(btnCreateNewConClc()));
}

void CreateDialupConnection::btnCreateNewConClc() {
    QVariantMap data;
    data["device"] = ui->cbxDialupSelectDevice->currentText().trimmed();
    data["connection"] = ui->cbxDialupSelectConName->currentText().trimmed();
    data["phone"] = ui->cbxDialupSelectPhoneNum->currentText().trimmed();
    data["login"] = ui->cbxDialupSelectUserName->currentText().trimmed();
    data["password"] = ui->cbxDialupSelectUserPass->currentText().trimmed();

    if (data["device"] == "") {
        QMessageBox messageBox(this);
        messageBox.setWindowTitle("Параметры устройств.");
        messageBox.setText("В терминале нет установленных модемов\n");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        QPushButton *okButton = messageBox.addButton(QString("OK"), QMessageBox::AcceptRole);
        messageBox.setDefaultButton(okButton);
#else
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setButtonText(QMessageBox::Ok, QString("OK"));
#endif
        messageBox.setIcon(QMessageBox::Critical);

        messageBox.exec();
        return;
    }

    if (data["device"] == "" || data["connection"] == "" || data["phone"] == "" ||
        data["login"] == "" || data["password"] == "") {

        QMessageBox messageBox(this);
        messageBox.setWindowTitle("Сохранение параметров.");
        messageBox.setText("Проверьте правильность ввода данных\n");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        QPushButton *okButton = messageBox.addButton(QString("OK"), QMessageBox::AcceptRole);
        messageBox.setDefaultButton(okButton);
#else
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setButtonText(QMessageBox::Ok, QString("OK"));
#endif
        messageBox.setIcon(QMessageBox::Warning);

        messageBox.exec();
        return;
    }

    for (auto &con : conList) {
        if (data["connection"] == con) {
            QMessageBox messageBox(this);
            messageBox.setWindowTitle("Сохранение параметров.");
            messageBox.setText("Такое соединение уже существует\n");
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
            QPushButton *okButton = messageBox.addButton(QString("OK"), QMessageBox::AcceptRole);
            messageBox.setDefaultButton(okButton);
#else
            messageBox.setStandardButtons(QMessageBox::Ok);
            messageBox.setDefaultButton(QMessageBox::Ok);
            messageBox.setButtonText(QMessageBox::Ok, QString("OK"));
#endif
            messageBox.setIcon(QMessageBox::Critical);

            messageBox.exec();
            return;
        }
    }

    QMessageBox messageBox(this);
    messageBox.setWindowTitle(QString("Сохранение параметров."));
    messageBox.setText(QString("Вы действительно хотите создать<br/>"
                               "соединение с параметрами?<br/><br/>"
                               "Наименование устройства: <b>%1</b><br/>"
                               "Наименование соединения: <b>%2</b><br/>"
                               "Номер дозвона:           <b>%3</b><br/>"
                               "Имя пользователя:        <b>%4</b><br/>"
                               "Пароль пользователя:     <b>%5</b><br/>")
                           .arg(data["device"].toString(),
                                data["connection"].toString(),
                                data["phone"].toString(),
                                data["login"].toString(),
                                data["password"].toString()));

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QPushButton *yesButton = messageBox.addButton(QString("Да"), QMessageBox::AcceptRole);
    QPushButton *cancelButton = messageBox.addButton(QString("Отмена"), QMessageBox::RejectRole);
    messageBox.setDefaultButton(yesButton);
    messageBox.setEscapeButton(cancelButton);
#else
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Yes);
    messageBox.setEscapeButton(QMessageBox::Cancel);
    messageBox.setButtonText(QMessageBox::Yes, QString("Да"));
    messageBox.setButtonText(QMessageBox::Cancel, QString("Отмена"));
#endif
    messageBox.setIcon(QMessageBox::Question);

    int r = messageBox.exec();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    if (r == QMessageBox::Accepted) {
#else
    if (r == QMessageBox::Yes) {
#endif
        emit emitDialupParam(data);
        this->close();
    }
}

void CreateDialupConnection::openThis() {
    // Вставляем устройства
    ui->cbxDialupSelectDevice->clear();
    ui->cbxDialupSelectDevice->addItems(this->devList);

    this->show();
}

CreateDialupConnection::~CreateDialupConnection() {
    delete ui;
}
