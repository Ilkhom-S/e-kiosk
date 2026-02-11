#pragma once

#include <QtCore/QStringList>

#include "protocol/ATProtocol.h"

class ATProtocol;

class ClassModem : public QThread {

    Q_OBJECT

public:
    ClassModem(QObject *parent = 0);

    bool execCommand(Modem_ProtocolCommands::Enum aCommand, bool thread);
    void setPort(QString portName);
    bool isItYou(QString &modemComent);
    bool isItYou(QStringList &comList, QString &modemName, QString &comStr, QString &modemComent);

    QString nowModem_Name;
    QString nowModem_Comment;
    QString nowPortName;
    QString nowProviderSim;
    QString nowNumberSim;
    QString nowBalanceSim;
    bool nowSim_Present{};
    QString nowModem_Quality;

    QString ussdRequestBalanseSim;
    QString ussdRequestNumberSim;
    int indexBalanceParse{};

    QString SMSTEXT_TO;
    void setBalanceRequest(QString text1, QString text2, int position);
    void setSim_NumberRequest(QString text1, QString text2);
    void sendSmsToAgentFew(QStringList smsId);
    void close();

signals:
    void emit_balance_sim_parsed();
    void emit_number_sim_parsed();
    void emit_statusSmsSend(bool sts, QStringList lstId);

private:
    ATProtocol *ATDevice;
    QString command;
    QStringList smsIdIn;

    static void msleep(int ms) { QThread::msleep(ms); }
    virtual void run();
    bool e_Data_Execute();
};
