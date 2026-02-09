#pragma once

#include "SendRequest.h"

class SendRequest;

class UserDaemons : public SendRequest {
    Q_OBJECT

public:
    UserDaemons(QObject *parent = 0);

private slots:
    void resendRequest();
    void setDataNote(const QDomNode &dom_Element);

public slots:
    void sendUserDataRequest(QString account, QString prvId);

private:
    void parcerNote(const QDomNode &dom_Element);

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
