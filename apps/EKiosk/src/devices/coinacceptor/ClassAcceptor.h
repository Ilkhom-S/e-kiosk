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
    static void setPortListInfo(QStringList portList);
    void setPartNumber(QString partNumber);
    bool openPort();
    bool isItYou(QStringList &aComList,
                 QString &aValidatorName,
                 QString &aComStr,
                 QString &aValidatorComent);

    void closeThis();
    bool pollState();

    QString nowValidatorName;
    QString nowPortName;
    QString nowComent;

    QString v_PartNumber;
    QString v_SerialNumber;

    QString phone_number;

    int status{};

public slots:
    void execCommand(int cmd);
    void getStatusFrom_Acceptor(int sts, QString comment);

private:
    CCTalk *CCTalkAcceptor{};

    QString com_Port;
    QStringList portList;
    QString validatorName;

    bool CIsItYou(QString &aValidatorName);

    int cmdExec{};

    virtual void run();

private slots:
    void terminatedThread();

signals:
    void eNominal(int nominal);
    void eNominalDuplicate(int nominal);
    void showHideDialogAnimate(bool status);
    void emitStatusCoinAcceptor(int sts, QString comment);
    void emitLoging(int status, QString title, QString text);
    void emitBillTable(QString bill_table);
};
