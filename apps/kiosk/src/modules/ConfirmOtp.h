#pragma once

// Project
#include "SendRequest.h"

class SendRequest;

class ConfirmOtp : public SendRequest {
    Q_OBJECT

  public:
    ConfirmOtp(QObject *parent = 0);

  private slots:
    void resendRequest();
    void setDataNote(const QDomNode &domElement);

  public slots:
    void confirmOtpRequest(QString otpId, QString otpValue);

  private:
    void parcerNote(const QDomNode &domElement);
    QString resultCode;

  signals:
    void emit_ConfirmOtpResult(QString resultCode);
};

