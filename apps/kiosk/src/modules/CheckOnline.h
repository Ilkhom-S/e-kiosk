#pragma once

// Project
#include "SendRequest.h"

class SendRequest;

class CheckOnline : public SendRequest {
    Q_OBJECT

  public:
    CheckOnline(QObject *parent = 0);

  private slots:
    void resendRequest();
    void setDataNote(const QDomNode &domElement);

  public slots:
    void sendCheckOnlineRequest(QString trn, QString prvId, QString account, double amount, QVariantMap param);

  private:
    void parcerNote(const QDomNode &domElement);

    bool getData;

    QString resultCode;
    QString status;
    QString message;

    QVariantList items;

  signals:
    void emit_CheckOnlineResult(QString resultCode, QString status, QString message, QVariantList items);
};
