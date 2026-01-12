#ifndef SENDOTP_H
#define SENDOTP_H

// Project
#include "SendRequest.h"

class SendRequest;

class SendOtp : public SendRequest {
  Q_OBJECT

public:
  SendOtp(QObject *parent = 0);

private slots:
  void resendRequest();
  void setDataNote(const QDomNode &domElement);

public slots:
  void sendOtpRequest(QString account);

private:
  void parcerNote(const QDomNode &domElement);

  QString resultCode;
  QString otpId;

signals:
  void emit_SendOtpResult(QString resultCode, QString otpId);
};

#endif // SENDOTP_H
