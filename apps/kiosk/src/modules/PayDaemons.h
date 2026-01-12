#ifndef PAYDAEMONS_H
#define PAYDAEMONS_H

// System
#include "../other/receipt.h"

// Project
#include "SendRequest.h"

class SendRequest;

class PayDaemons : public SendRequest {
  Q_OBJECT

public:
  PayDaemons(QObject *parent = 0);
  QString getReceiptInfo(QString tranz_id);

  QString oraganization;
  QString rma;
  QString tpl;

  QString kassir;
  QString phone;
  QString printerModel;
  QString numTrm;

  void startTimer(const int sec);

private:
  bool GetOperationCount(const QString &id_trn);

  bool getPayData(QString id_trn, QString &date_create, QString &prv_name,
                  QString &account, QString &sum_from, QString &sum_to,
                  QString &ratio_sum, QString &ratio_persent);
  void sendPaymentToServer(bool withNon);

  bool getPaymentMap(QString &payment, int &count_pay, double &all_sum);
  bool updateOperationStatus(const QString &id_trm, const QString &status,
                             const QString &dateConfirm);
  void parcerNote(const QDomNode &domElement);

  void confirmPayments();

  QTimer *payTimer;
  QTimer *restatTimer;

  double gbl_balance;
  double gbl_overdraft;
  bool status_get;
  bool lockState;
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
  void get_confirm_pay(QString tranz_id, bool print);
  void get_print_id(QString tranz_id);
  bool getCountPayment(int &count);
  void checkPayStatus54();

signals:
  void emit_to_print(QString print);
  void lockUnlockNonSend(bool lock);
  void lockUnlockAvtorization(bool lock, int sts);
  void emit_responseBalance(const double balance, const double overdraft,
                            const double threshold);
  void emit_statusUpdatetoIn();
  void emit_AnimateForSearch(bool start);
  void emit_RestartNet();
  void emit_RestartTerminal();
  void emit_errorDB();
};

#endif // PAYDAEMONS_H
