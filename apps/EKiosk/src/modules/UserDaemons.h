#pragma once

#include "SendRequest.h"

class SendRequest;

class UserDaemons : public SendRequest {
    Q_OBJECT

public:
    UserDaemons(QObject *parent = 0);

private slots:
    void resendRequest();
    void setDataNote(const QDomNode &domElement);

public slots:
    void sendUserDataRequest(QString account, QString prvId);

private:
    void parseNode(const QDomNode &domElement);

    bool resultCode;
    bool getData;
    QString balance;
    //        QString login;

    QString num_Trm;
    QString login;
    QString pass;

signals:
    void emit_UserData(QString data);
};
