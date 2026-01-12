#ifndef AUTHREQUEST_H
#define AUTHREQUEST_H

#include "SendRequest.h"
class SendRequest;

class AuthRequest : public SendRequest {
  Q_OBJECT

public:
  AuthRequest(QObject *parent = 0);

private:
  QString resultCode;
  QString login;
  QString token;
  QString message;

  void parcerNote(const QDomNode &domElement);

private slots:
  void errorResponse();
  void setDataNote(const QDomNode &domElement);

public slots:
  void sendAuthRequest(QString login, QString otp, QString hash, QString cid);

signals:
  void emitResult(QString resultCode, QString login, QString token,
                  QString message);
};

#endif // AUTHREQUEST_H
