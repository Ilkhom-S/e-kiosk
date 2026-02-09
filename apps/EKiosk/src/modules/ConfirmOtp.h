#pragma once

#include "SendRequest.h"

class SendRequest;

class Confirm_Otp : public SendRequest {
    Q_OBJECT

public:
    Confirm_Otp(QObject *parent = 0);

private slots:
    void resendRequest();
    void setDataNote(const QDomNode &dom_Element);

public slots:
    void confirm_OtpRequest(QString otpId, QString otpValue);

private:
    void parcerNote(const QDomNode &dom_Element);
    QString resultCode;

signals:
    void emit_Confirm_OtpResult(QString resultCode);
};
