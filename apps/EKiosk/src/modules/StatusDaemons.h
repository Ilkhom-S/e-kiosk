#pragma once

#include "SendRequest.h"

class SendRequest;

namespace Sender {
struct Validator {
    int billCount;
    double billSum;
    QString billInfo;

    int moneyOutCount;
    double moneyOutSum;

    QString port;
    QString serial;
    QString name;
    QString state;

    Validator()
        : billCount(0), billSum(0), moneyOutCount(0), moneyOutSum(0), port(""), serial(""),
          name(""), state("") {}
};

struct CoinAcceptor {
    int coinCount;
    double coinSum;
    QString coinInfo;

    QString port;
    QString serial;
    QString name;
    QString state;

    CoinAcceptor()
        : coinCount(0), coinSum(0), coinInfo(""), port(""), serial(""), name(""), state("") {}
};

struct Printer {
    QString name;
    QString port;
    int state;
    QString allState;

    Printer() : name(""), port(""), state(0), allState("") {}
};

struct Modem {
    QString name;
    QString port;
    QString state;
    QString signal;
    QString balance;
    QString provider;
    QString number;
    QString comment;

    Modem()
        : name(""), port(""), state(""), signal(""), balance(""), provider(""), number(""),
          comment("") {}
};

struct Data {
    bool firstSend;
    QString version;
    QString fullVersion;
    int lockStatus;
    QStringList action;
    bool actionState;
    QString connection;
    QVariantMap system_Info;

    Validator validator;
    CoinAcceptor coinAcceptor;
    Printer printer;
    Modem modem;

    Data()
        : firstSend(false), version(""), lockStatus(0),

          actionState(false), connection("0") {}
};
} // namespace Sender

class StatusDaemons : public SendRequest {
    Q_OBJECT

public:
    StatusDaemons(QObject *parent = 0);
    void startTimer(const int sec);

    void sendStatusToServer(Sender::Data &a_Data);

    bool firstSend;

private:
    void parcerNote(const QDom_Node &dom_Element);

    double gbl_overdraft;
    double gbl_balance;
    QString gbl_active;
    int count_resend;

    QTimer *demonTimer;
    QString requestString;

    QVariantList cmdList;

private slots:
    void resendRequest();
    void r_RequestRepeet();
    void setDataNote(const QDom_Node &dom_Element);

signals:
    void getRequestParam();
    void emit_responseBalance(const double balance, const double overdraft, const double threshold);
    void lockUnlockAvtorization(bool lock, int sts);
    void emit_responseIsActive(const bool is_active);
    void emit_cmdToMain(QVariantList cmdList);
    void emit_hashToCheck(QString hash);
    void emit_hashUpdateToCheck(QString hashUpdate, QString path);
};
