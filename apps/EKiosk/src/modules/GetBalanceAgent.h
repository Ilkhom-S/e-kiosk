#pragma once

#include "SendRequest.h"

class SendRequest;

class GetBalanceAgent : public SendRequest {
    Q_OBJECT

public:
    GetBalanceAgent(QObject *parent = 0);

private:
    void parcerNote(const QDomNode &dom_Element);

    bool resultCode;
    bool getData;

    QString balance;
    QString overdraft;

private slots:
    void resendRequest();
    void setDataNote(const QDomNode &dom_Element);

public slots:
    void sendDataRequest();

signals:
    void emit_BalanceAgent(QString dataBalance, QString dataOverdraft);
};
