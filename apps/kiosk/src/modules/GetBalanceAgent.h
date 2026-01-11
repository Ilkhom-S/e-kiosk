#ifndef GETBALANCEAGENT_H
#define GETBALANCEAGENT_H

#include "SendRequest.h"
class SendRequest;

class GetBalanceAgent : public SendRequest {
    Q_OBJECT

  public:
    GetBalanceAgent(QObject* parent = 0);

  private:
    void parcerNote(const QDomNode& domElement);

    bool resultCode;
    bool getData;

    QString balance;
    QString overdraft;

  private slots:
    void resendRequest();
    void setDataNote(const QDomNode& domElement);

  public slots:
    void sendDataRequest();

  signals:
    void emit_BalanceAgent(QString dataBalance, QString dataOverdraft);
};

#endif  // GETBALANCEAGENT_H
