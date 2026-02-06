#pragma once

#include <QtCore/QPointer>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

#include "keypud.h"

namespace Ui {
class AvtorizationToAdminIn;
}

class AvtorizationToAdminIn : public QDialog {
    Q_OBJECT

public:
    explicit AvtorizationToAdminIn(QWidget *parent = 0);
    ~AvtorizationToAdminIn();

    void setAuthParam(QString login, QString pass);
signals:
    void emit_openAdminDialog();

private:
    Ui::AvtorizationToAdminIn *ui;

    keyPud *KeyPud;

    QString loginIn;
    QString passIn;
    int countCheckIn;

private slots:
    void sendCharacter(QChar character);
    void checkInputData();
};
