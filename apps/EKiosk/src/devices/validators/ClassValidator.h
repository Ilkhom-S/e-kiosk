#pragma once

#include "dev/CCNetSM.h"
#include "dev/EBDS.h"

namespace ValidatorModel {
const QString CashCodeCCNET = "CashCode_CCNET";
const QString MeiEBDS = "Mei";
} // namespace ValidatorModel

class CCNetSm;

class ClassValidator : public QThread {
    Q_OBJECT

public:
    ClassValidator(QObject *parent = 0);

    void setValidator(QString name);
    void setPortName(QString portName);
    static void setPortListInfo(QStringList portList);
    void setPartNumber(QString partNumber);
    void setDBError(bool error);

    bool openPort();
    bool isItYou(QStringList &comList,
                 QString &validatorName,
                 QString &comStr,
                 QString &validatorComent);

    void closeThis();
    bool pollState();

    QString nowValidatorName;
    QString nowPortName;
    QString nowComent;

    QString vPartNumber;
    QString vSerialNumber;

    QVariantMap maxSum;
    int status{};

    QString firmwareVersion;

public slots:
    void execCommand(int cmd);
    void getStatusFrom_Validator(int sts, QString comment);

private:
    CCNetSm *CCNetValidator{};
    EBDS *EBDSValidator{};

    QString com_Port;
    QStringList portList;
    QString validatorName;

    bool CIsItYou(QString &validatName);

    int cmdExec{};

    virtual void run() override;

signals:
    void eNominal(int nominal);
    void eNominalDuplicate(int nominal);
    void showHideDialogAnimate(bool status);
    void showHideDialogReturnNominal(bool status);
    void emitStatusValidator(int sts, QString comment);
    void emitLog(int status, QString title, QString text);
    void emitValidatorLog(int status, QByteArray data, QString text);
    void emitFirmwareUpdate(QString state);
};
