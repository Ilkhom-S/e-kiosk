#pragma once

#include "SendRequest.h"

struct Bill {
    int face;
    int value = 0;
};

struct Coin {
    int face;
    int value = 0;
};

struct NominalData {
    QString billCurrency;
    QString coinCurrency;

    QList<Bill> bills;
    QList<Coin> coins;

    int billCount = 0;
    double billSum = 0;
    int coinCount = 0;
    double coinSum = 0;
    double coinDivider = 100;

    static NominalData from_Variant(const QVariantMap &v) {
        NominalData nominalData;
        nominalData.billCurrency = v.value("bill_currency").toString();
        nominalData.coinCurrency = v.value("coin_currency").toString();

        auto bills = v.value("bills").toList();
        for (auto &b : bills) {
            Bill bill;
            bill.face = b.toInt();
            bill.value = 0;

            nominalData.bills.append(bill);
        }

        auto coins = v.value("coins").toList();
        for (auto &c : coins) {
            Coin coin;
            coin.face = c.toInt();
            coin.value = 0;

            nominalData.coins.append(coin);
        }

        return nominalData;
    }

    void calculateTotal() {
        billCount = 0;
        billSum = 0.0;

        for (auto &b : bills) {
            billCount += b.value;
            billSum += b.face * b.value;
        }

        coinCount = 0;
        coinSum = 0.0;

        for (auto &b : coins) {
            coinCount += b.value;
            coinSum += b.face * b.value;
        }
    }
};

class SendRequest;

class CollectDaemons : public SendRequest {
    Q_OBJECT

public:
    CollectDaemons(QObject *parent = 0);
    void setNominalData(QVariantMap data);

    bool firstCollection = false;
    bool createNewCollection(QString id);
    int getCollectionCount(QString status = "new");

    QVariantMap getNominalInfo();

    QString getHtmlInfoBox(QString &nonCollectPay,
                           int &moneyOutNum,
                           double &moneyOutSum,
                           QString data,
                           QString &collectionId,
                           QString &stackId,
                           QString &trnFrom,
                           QString &trnTo);
    bool getCheckText(QString &text, bool preCheck, QString dateP, QString cid = QString());
    bool getDataCollectList(QStringList &lst, int &countI);

    QString getCollectionId(QString status = "new");

    QTimer *demonTimer;
    QTimer *sendNewTimer{};
    int countAllRep;
    int RealRepeat;

    QString SMSTEXT;

private:
    NominalData nominalData;

    int getNonCollectOperationCount(const QString cid);
    int getCollectCount(bool newCollect);
    bool sendCollection(QString &trnId,
                        const QString collectionId,
                        QString &collectionIdNext,
                        QString &dateCreate,
                        QString xmlDenom);
    bool confirm_Collection(QString collectionId);

    QString getDenominalXml();
    bool getDatePrevCollection(QString &date, QString collectionId);
    bool getTrnOperation(const QString collectId, const QString &cmd, QString &trn);
    bool getMoneyOut(const QString &collectionId, int &numOut, double &sumOut);
    bool getCollectionInfo(QString collectionId,
                           QString &collectionIdNext,
                           QString &idTrn,
                           QString &dateCreate,
                           QString &denomXml);
    bool parsDenomilSlot(const QString &xmlDenom);
    bool updateMoneyOut(QString collectionId, QString collectionStatus);
    void getXmlForSend(QString &xml,
                       QString cid,
                       QString cidNext,
                       QString sid,
                       QString dateCreate,
                       QString collectDenom,
                       int moneyOutCount,
                       double moneyOutSum,
                       QString trnFrom,
                       QString trnTo);
    void parseNode(const QDomNode &domElement);

    QString getCollectIdByDate(QString date);

    int repeatCount{};

    QString cIdUpdate;

    QString num_Trm;
    QString login;
    QString pass;
    QString requestXml;

    QString getCoinTxt(int coin) const;

private slots:
    bool parserCollectIntoOperation(const QString cid);

    void sendCollectRequest();
    void getRequestParam();
    void errResponse();
    void setDataNote(const QDomNode &domElement);

signals:
    void lockUnlockAuthorization(bool lock, int sts);
};
