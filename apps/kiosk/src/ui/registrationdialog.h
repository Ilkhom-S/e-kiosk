#ifndef REGISTRATIONDIALOG_H
#define REGISTRATIONDIALOG_H

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPointer>
#include <QtGui/QKeyEvent>
#include <QtGui/QMovie>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>
#include <Common/QtHeadersEnd.h>

// Project
#include "createdialupconnection.h"
#include "keypud.h"

namespace Ui {
class RegistrationDialog;
}

class RegistrationDialog : public QDialog {
  Q_OBJECT

public:
  explicit RegistrationDialog(QWidget *parent = 0);
  void showMsgDialog(QString title, QString text);
  void setDataListConnection(QStringList list);
  ~RegistrationDialog();
  void setValidatorText(QString status);
  void setPrinterText(QString status);
  void setWatchdogText(QString status);
  void setModemText(QString status);
  void logForTest(int sts, QString log);
  void LoadingAnim(bool start);

  QStringList dialupDevice;

private:
  Ui::RegistrationDialog *ui;

  keyPud *KeyPud;
  CreateDialupConnection *createDialupConnection;

  QString gblNumTrm;
  QString gblLogin;
  QString gblPass;

  // QString gblSecretNum;
  QString gblSecretLogin;
  QString gblSecretPass;

  QStringList connListInfData;

  QMovie *movie;

private slots:
  void on_btnNumTrmRegClear_clicked();
  void tabToRegistration1();
  void tabToRegistration2();

  void save1BtnReg();
  void save2BtnReg();
  void save3BtnReg();

  void closeForm();

  void sendCharacter(QChar character);

  void createNewConnection();
  void btnTesTclc();
  void btnBackClc();
  void starttest();
  void tabChanged(int page);

signals:
  void emitRegistrationData(QVariantMap data);
  void emitStartTest(QString numTrm, QString pLogin, QString pPass,
                     QString secretLogin, QString secretPass, QString conName,
                     QString balanceReq, QString positionReq,
                     QString numberReq);
  void emitCloseForm();
  void emitCreateNewConnection(QVariantMap data);
};

#endif // REGISTRATIONDIALOG_H
