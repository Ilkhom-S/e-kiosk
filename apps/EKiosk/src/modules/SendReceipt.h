#pragma once

#include "SendRequest.h"

class SendRequest;

class SendReceipt : public SendRequest {
    Q_OBJECT

public:
    SendReceipt(QObject *parent = 0);

private slots:
    void resendRequest();
    void setDataNote(const QDomNode &dom_Element);
    void sendRequestRepeet();

public slots:
    void sendReceiptRequest(QString trn, QString notify);

private:
    void parcerNote(const QDomNode &dom_Element);

    bool getData;

    int countAllRep;
    QString requestXml;

    QString resultCode;
    QString trn;
    QString status;

signals:
    void emitSendReceiptResult(QString resultCode, QString trn, QString status);
};
