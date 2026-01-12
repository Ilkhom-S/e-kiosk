#ifndef JSINTERFACE_H
#define JSINTERFACE_H

#include <QtCore/QDebug>
#include <QtCore/QObject>

class JsInterface : public QObject {
  Q_OBJECT

public:
  JsInterface(QObject *parent = nullptr) : QObject(parent) {}

public:
  Q_INVOKABLE
  void adminOpen() { emit emitAdminOpen(); }

  Q_INVOKABLE
  void gotoPageMain() { emit emitGotoPageMain(); }

  Q_INVOKABLE
  void gotoPageMainFromInput() { emit emitGotoPageMainFromInput(); }

  Q_INVOKABLE
  void gotoPageInfo() { emit emitGotoPageInfo(); }

  Q_INVOKABLE
  void gotoPageInputAccount(QString serviceName) {
    emit emitGotoPageInputAccount(serviceName);
  }

  Q_INVOKABLE
  void gotoPageInsertNominal(QString account, QString accountDesign,
                             QString prvId, QString prvName) {
    emit emitGotoPageInsertNominal(account, accountDesign, prvId, prvName);
  }

  Q_INVOKABLE
  void gotoPageServices(QString group) { emit emitGotoPageServices(group); }

  Q_INVOKABLE
  void gotoPageInputFromPayment() { emit emitGotoPageInputFromPayment(); }

  Q_INVOKABLE
  void paymentConfirm() { emit emitPaymentConfirm(); }

  Q_INVOKABLE
  void userInfoCheck(QString account, QString prvId) {
    emit emitUserInfoCheck(account, prvId);
  }

  void userInfoResult(QString data) { emit emitUserInfoResult(data); }

  Q_INVOKABLE
  void commissionProfileGet() { emit emitCommissionProfileGet(); }

  void commissionProfileSet(QString profile) {
    emit emitCommissionProfile(profile);
  }

  Q_INVOKABLE
  void langSet(QString lang) { emit emitLangSet(lang); }

  Q_INVOKABLE
  void playNumpadSound(QString fileName) { emit emitPlayNumpadSound(fileName); }

  Q_INVOKABLE
  void receiptPrint(bool withSound) { emit emitReceiptPrint(withSound); }

  Q_INVOKABLE
  void receiptSend(QString phone) { emit emitReceiptSend(phone); }

  void receiptSendError(QString message) { emit emitReceiptSendError(message); }

  void receiptSendSuccess(QString message) {
    emit emitReceiptSendSuccess(message);
  }

  Q_INVOKABLE
  void printerStatusGet() { emit emitPrinterStatusGet(); }

  void printerStatusSet(bool enable) { emit emitPrinterStatus(enable); }

  Q_INVOKABLE
  void precheck(QString idPrv, QString account, QString param, QString param2) {
    emit emitPreCheck(idPrv, account, param, param2);
  };

  void precheckError(QString message) { emit emitPrecheckError(message); }

  void precheckSuccess(QString message, QVariantList items) {
    emit emitPrecheckSuccess(message, items);
  }

signals:
  void emitAdminOpen();
  void emitGotoPageMain();
  void emitGotoPageMainFromInput();
  void emitGotoPageInfo();
  void emitGotoPageInputAccount(QString serviceName);
  void emitGotoPageInsertNominal(QString account, QString accountDesign,
                                 QString prvId, QString prvName);
  void emitGotoPageServices(QString group);
  void emitGotoPageInputFromPayment();

  void emitPaymentConfirm();

  void emitCommissionProfileGet();
  void emitCommissionProfile(QString profile);

  void emitUserInfoCheck(QString account, QString prvId);
  void emitUserInfoResult(QString data);

  void emitPreCheck(QString idPrv, QString account, QString param,
                    QString param2);
  void emitPrecheckError(QString message);
  void emitPrecheckSuccess(QString message, QVariantList items);

  void emitLangSet(QString lang);
  void emitPlayNumpadSound(QString fileName);

  void emitReceiptPrint(bool withSound);
  void emitReceiptSend(QString phone);
  void emitReceiptSendError(QString message);
  void emitReceiptSendSuccess(QString message);

  void emitPrinterStatusGet();
  void emitPrinterStatus(bool enable);
};

#endif // JSINTERFACE_H
