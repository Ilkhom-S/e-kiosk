#pragma once

#include "SendRequest.h"

class SendRequest;

class CommandConfirm : public SendRequest {
    Q_OBJECT

public:
    CommandConfirm(QObject *parent = 0);

private slots:
    void resendRequest();
    void setDataNote(const QDom_Node &dom_Element);
    void sendRequestRepeet();

public slots:
    void sendCommandConfirm(QString trn, int cmd);

private:
    void parcerNote(const QDom_Node &dom_Element);

    bool resultCode;
    int countAllRep;
    QString requestXml;

    QString trnCmd;

signals:
    void emit_cmdConfirmed(QString trn);
};
