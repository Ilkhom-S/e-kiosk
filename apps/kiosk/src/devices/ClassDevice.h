#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtSql/QSqlDatabase>
#include <Common/QtHeadersEnd.h>

// System
#include "coinacceptor/ClassAcceptor.h"
#include "modems/ClassModem.h"
#include "printers/ClassPrinter.h"
#include "validators/ClassValidator.h"
#include "watchdogs/watchdogs.h"

namespace SearchDev {
enum ssTools {
  search_validator = 0,
  search_printer = 1,
  search_modem = 2,
  search_watchdog = 3,
  search_coin_acceptor = 4,

  start_search = 0,
  device_found = 1,
  device_notfound = 2
};
} // namespace SearchDev

class SearchDevices : public QThread {

  Q_OBJECT

public:
  SearchDevices(QObject *parent = 0);

  void setComListInfo(QStringList com_list);
  void setDbName(QSqlDatabase &dbName);
  bool takeBalanceSim;
  bool takeSimNumber;

  QString signalQuality;
  QString simNumber;
  QString simBalance;
  QString operatorName;
  QString modemComment;
  QString prtWinName;
  bool nowSimPresent;
  QString receiptTest;

  QString s_ussdRequestBalanseSim;
  QString s_ussdRequestNumberSim;
  int s_indexBalanceParse;

  QString validatorSerialNum;
  QString validatorPartNum;

  QString coinAcceptorSerialNum;
  QString coinAcceptorPartNum;

  // Поиск валидатора
  bool searchValidator;
  // Поиск монетоприемника
  bool searchCoinAcceptor;
  // Поиск принтера
  bool searchPrinter;
  // Поиск модема
  bool searchModem;
  // Поиск Сторожа
  bool searchWD;

  bool modemFound;
  bool modemConUp;

  bool testMode;

private:
  ClassModem *modem;
  ClassPrinter *printer;
  ClassValidator *validator;
  ClassAcceptor *coinAccepter;
  WatchDogs *watchDogs;

  QSqlDatabase db;
  QStringList comList;
  QString vrmModemPort;

  bool debugger;

  static void msleep(int ms) { QThread::msleep(ms); }
  virtual void run();
  bool getDeviseInBase(int id_device, QString &name_device, QString &port);
  bool setDeviseInBase(int id_device, const QString &name_device,
                       const QString &port, const QString &comment, int state);
  bool insertDeviseInBase(int id_device, QString &name_device, QString &port);

public slots:
  bool searchDeviceMethod(int device, QString &dev_name, QString &com_str,
                          QString &dev_coment, const bool with_test = false);
  void getModemInfo();

signals:
  void emit_getModemInfo();

public slots:

signals:
  void emitDeviceSearch(int device, int result, QString dev_name,
                        QString dev_comment, QString dev_port);
  void emitDeviceSearchFinished();
};
