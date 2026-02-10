#pragma once

#include "../other/receipt.h"
#include "SendRequest.h"

class SendRequest;

class PayDaemons : public SendRequest {
    Q_OBJECT

public:
    PayDaemons(QObject *parent = 0);
    QString getReceiptInfo(QString tranzId);

    QString oraganization;
    QString rma;
    QString tpl;

    QString kassir;
    QString phone;
    QString printerModel;
    QString num_Trm;

    void startTimer(const int sec);

private:
    bool GetOperationCount(const QString &idTrn);

    bool getPayData(QString idTrn,
                    QString &dateCreate,
                    QString &prvName,
                    QString &account,
                    QString &sumFrom,
                    QString &sumTo,
                    QString &ratioSum,
                    QString &ratioPersent);
    void sendPaymentToServer(bool withNon);

    bool getPaymentMap(QString &payment, int &countPay, double &allSum);
    bool
    updateOperationStatus(const QString &idTrm, const QString &status, const QString &dateConfirm);
    void parseNode(const QDomNode &domElement);

    void confirm_Payments();

    QTimer *payTimer;
    QTimer *restatTimer;

    double gbl_balance{};
    double gbl_overdraft{};
    bool status_get{};
    bool lockState{};
    bool firstSend;

    bool abortPresent;
    int count_non_send;

    QString getCollectionId();

private slots:
    void setDataNote(const QDomNode &domElement);
    void getErrResponse();

public slots:
    void sendPayRequest();
    void get_new_pay(QVariantMap payment);
    void get_update_pay(QVariantMap payment);
    void get_confirm_pay(QString tranzId, bool print);
    void get_print_id(QString tranzId);
    bool getCountPayment(int &count);
    void checkPayStatus54();

signals:
    void emit_to_print(QString print);
    void lockUnlockNonSend(bool lock);
    void lockUnlockAuthorization(bool lock, int sts);
    void emit_responseBalance(const double balance, const double overdraft, const double threshold);
    void emit_statusUpdatetoIn();
    void emit_AnimateForSearch(bool start);
    void emit_RestartNet();
    void emit_RestartTerminal();
    void emit_errorDB();
};
