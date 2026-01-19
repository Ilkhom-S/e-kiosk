#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// System
#include "protocol/ATProtocol.h"

class ATProtocol;

class ClassModem : public QThread {

    Q_OBJECT

  public:
    ClassModem(QObject *parent = 0);

    bool execCommand(ModemProtocolCommands::Enum aCommand, bool thread);
    void setPort(QString port_name);
    bool isItYou(QString &modem_coment);
    bool isItYou(QStringList &comList, QString &modem_name, QString &com_str, QString &modem_coment);

    QString nowModemName;
    QString nowModemComment;
    QString nowPortName;
    QString nowProviderSim;
    QString nowNumberSim;
    QString nowBalanceSim;
    bool nowSimPresent;
    QString nowModemQuality;

    QString ussdRequestBalanseSim;
    QString ussdRequestNumberSim;
    int indexBalanceParse;

    QString SMSTEXT_TO;
    void setBalanceRequest(QString text1, QString text2, int position);
    void setSimNumberRequest(QString text1, QString text2);
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

    static void msleep(int ms) {
        QThread::msleep(ms);
    }
    virtual void run();
    bool e_Data_Execute();
};
