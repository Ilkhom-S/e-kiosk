#pragma once

#include "SendRequest.h"

class SendRequest;

class SendOtp : public SendRequest {
    Q_OBJECT

public:
    SendOtp(QObject *parent = 0);

private slots:
    void resendRequest();
    void setDataNote(const QDom_Node &dom_Element);

public slots:
    void sendOtpRequest(QString account);

private:
    void parcerNote(const QDom_Node &dom_Element);

    QString resultCode;
    QString otpId;

signals:
    void emit_SendOtpResult(QString resultCode, QString otpId);
};
