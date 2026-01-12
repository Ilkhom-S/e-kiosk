#ifndef USERDAEMONS_H
#define USERDAEMONS_H

#include "SendRequest.h"
class SendRequest;

class UserDaemons : public SendRequest {
  Q_OBJECT

public:
  UserDaemons(QObject *parent = 0);

private slots:
  void resendRequest();
  void setDataNote(const QDomNode &domElement);

public slots:
  void sendUserDataRequest(QString account, QString prvId);

private:
  void parcerNote(const QDomNode &domElement);

  bool resultCode;
  bool getData;
  QString balance;
  //        QString login;

  QString numTrm;
  QString login;
  QString pass;

signals:
  void emit_UserData(QString data);
};

#endif // USERDAEMONS_H
