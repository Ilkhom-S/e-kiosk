#ifndef COMMANDCONFIRM_H
#define COMMANDCONFIRM_H

#include "SendRequest.h"
class SendRequest;

class CommandConfirm : public SendRequest {
    Q_OBJECT

  public:
    CommandConfirm(QObject* parent = 0);

  private slots:
    void resendRequest();
    void setDataNote(const QDomNode& domElement);
    void sendRequestRepeet();

  public slots:
    void sendCommandConfirm(QString trn, int cmd);

  private:
    void parcerNote(const QDomNode& domElement);

    bool resultCode;
    int countAllRep;
    QString requestXml;

    QString trnCmd;

  signals:
    void emit_cmdConfirmed(QString trn);
};

#endif  // COMMANDCONFIRM_H
