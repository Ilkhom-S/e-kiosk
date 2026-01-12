#ifndef GETSERVICES_H
#define GETSERVICES_H

#include "SendRequest.h"

class SendRequest;

class GetServices : public SendRequest {
  Q_OBJECT

public:
  GetServices(QObject *parent = 0);

public slots:
  void parcerHash(const QDomNode &domElement);
  void parcerNote(const QDomNode &domElement);
  void setDataNote(const QDomNode &domElement);
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
  bool saveCommissionDB(const int id, const double sumFrom, const double sumTo,
                        const int type, const double value, const int serviceId,
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
  void emit_responseBalance(const double balance, const double overdraft,
                            const double threshold);
  void emit_responseIsActive(const bool active);
  void emit_getServices(bool status);
  void emit_infoData(QVariantMap data);
  //        void emit_errorResponse();
  void lockUnlockAvtorization(bool lock, int sts);
  void emit_timeServer(QString dateTime);
  void emit_banners(QVariantList banners);
};

#endif // GETSERVICES_H
