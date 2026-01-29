#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QDialog>
#include <Common/QtHeadersEnd.h>

// Project
#include "keypud.h"

namespace Ui
{
    class ChangePassword;
}
class CoddingRes;
class ChangePassword : public QDialog
{
    Q_OBJECT

  public:
    explicit ChangePassword(QWidget *parent = 0);
    ~ChangePassword();

  private:
    Ui::ChangePassword *ui;
    keyPud *KeyPud;

  private slots:
    void btnok_clc();
    void btncancel_clc();
    void sendCharacter(QChar character);

  signals:
    void emit_changepass(QString login, QString password);
};
