#pragma once

#include "SendRequest.h"

class SendRequest;

class CommandConfirm : public SendRequest {
    Q_OBJECT

public:
    CommandConfirm(QObject *parent = 0);

private slots:
    void resendRequest();
    void setDataNote(const QDomNode &domElement);
    void sendRequestRepeat();

public slots:
    void sendCommandConfirm(QString trn, int cmd);

private:
    void parseNode(const QDomNode &domElement);

    bool resultCode;
    int countAllRep;
    QString requestXml;

    QString trnCmd;

signals:
    void emit_cmdConfirmed(QString trn);
};
