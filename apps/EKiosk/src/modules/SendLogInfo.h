#pragma once

#include "SendRequest.h"

class SendRequest;

class SendLogInfo : public SendRequest {
    Q_OBJECT

public:
    SendLogInfo(QObject *parent = 0);

    QString system_Log;

private:
    void parcerNote(const QDomNode &dom_Element);
    void getCompressLogData(QString date, bool &result, QString &strip);
    void getCompressValiatorLogData(QString date, QString account, bool &result, QString &strip);

    bool resultCode;
    int countAllRep;
    QString requestXml;
    int nowIdTrn;

    QTimer *timerPic;

private slots:
    void resendRequest();
    void setDataNote(const QDomNode &dom_Element);
    void sendRequestRepeet();

public slots:
    void sendLogInfoToServer(QString trn, QString date);
    void sendLogValidatorToServer(QString trn, QString date, QString account);

signals:
    void emit_cmdResponseCode(int sts);
};
