#pragma once

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

    void parcerNote(const QDom_Node &dom_Element);

private slots:
    void errorResponse();
    void setDataNote(const QDom_Node &dom_Element);

public slots:
    void sendAuthRequest(QString login, QString otp, QString hash, QString cid);

signals:
    void emitResult(QString resultCode, QString login, QString token, QString message);
};
