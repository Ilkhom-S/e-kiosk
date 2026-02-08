#pragma once

#include "dev/CCTalk.h"

namespace AcceptorModel {
const QString CCTalk = "CCTalk";
}

class CCTalk;

class ClassAcceptor : public QThread {
    Q_OBJECT

public:
    ClassAcceptor(QObject *parent = 0);

    void setValidator(QString name);
    void setPortName(QString portName);
    void setPortListInfo(QStringList port_list);
    void setPartNumber(QString partNumber);
    bool openPort();
    bool isItYou(QStringList &com_List,
                 QString &validator_name,
                 QString &com_str,
                 QString &validator_coment);

    void closeThis();
    bool pollState();

    QString nowValidatorName;
    QString nowPortName;
    QString nowComent;

    QString v_PartNumber;
    QString v_SerialNumber;

    QString phone_number;

    int status;

public slots:
    void execCommand(int cmd);
    void getStatusFrom_Acceptor(int sts, QString comment);

private:
    CCTalk *CCTalkAcceptor;

    QString com_Port;
    QStringList portList;
    QString validatorName;

    bool CIsItYou(QString &validat_name);

    int cmdExec;

    virtual void run();

private slots:
    void termanatedThread();

signals:
    void eNominal(int nominal);
    void eNominalDuplicate(int nominal);
    void showHideDialogAnimate(bool status);
    void emitStatusCoinAcceptor(int sts, QString comment);
    void emitLoging(int status, QString title, QString text);
    void emitBillTable(QString bill_table);
};
