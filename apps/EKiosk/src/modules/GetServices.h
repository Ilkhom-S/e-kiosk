#pragma once

#include "SendRequest.h"

class SendRequest;

class GetServices : public SendRequest {
    Q_OBJECT

public:
    GetServices(QObject *parent = 0);

public slots:
    void parcerHash(const QDom_Node &dom_Element);
    void parcerNote(const QDom_Node &dom_Element);
    void setDataNote(const QDom_Node &dom_Element);
    void sendGetServicesQuery();
    //        void errorResponse();

private:
    QString toLogData;
    int count_providers;
    int index_prv;

    int gbl_id_services;

    QVariantMap infoMap;
    QMap<int, QString> curMap;
    QMap<int, QMap<QString, QString>> providerList;
    QVariantList servicesInputs;

    QVariantList banners;

    bool saveServicesDB();
    bool saveCategoriesDB(const QVariantMap category);
    bool saveCommissionDB(const int id,
                          const double sum_From,
                          const double sum_To,
                          const int type,
                          const double value,
                          const int serviceId,
                          const int index);
    bool saveInputsDB(const QVariantMap input);
    bool saveServicesInputsDB();
    bool deleteOldData();

    QString getHashConfig();

    bool insertTerminalExtra(QVariantMap map);
    bool getTerminalExtra(QVariantMap &map);
    int getProviderCount();
    bool getReqStatus;

signals:
    void emit_responseBalance(const double balance, const double overdraft, const double threshold);
    void emit_responseIsActive(const bool active);
    void emit_getServices(bool status);
    void emit_infoData(QVariantMap data);
    //        void emit_errorResponse();
    void lockUnlockAvtorization(bool lock, int sts);
    void emit_timeServer(QString dateTime);
    void emit_banners(QVariantList banners);
};
