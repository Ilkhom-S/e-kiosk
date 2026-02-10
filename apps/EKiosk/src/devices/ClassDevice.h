#pragma once

#include <QtSql/QSqlDatabase>

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

    void setComListInfo(QStringList comList);
    void setDbName(QSqlDatabase &dbName);
    bool takeBalanceSim;
    bool takeSimNumber;
    ;

    QString signalQuality;
    QString sim_Number;
    QString sim_Balance;
    QString operatorName;
    QString modem_Comment;
    QString prtWinName;
    bool nowSim_Present{};
    ;
    QString receiptTest;

    QString s_ussdRequestBalanceSim;
    QString s_ussdRequestNumberSim;
    int s_indexBalanceParse;
    ;

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

    bool modem_Found;
    bool modem_ConUp;

    bool testMode;

private:
    ClassModem *modem;
    ClassPrinter *printer;
    ClassValidator *validator;
    ClassAcceptor *coinAccepter;
    WatchDogs *watchDogs;

    QSqlDatabase db;
    QStringList com_List;
    QString vrm_Modem_Port;

    bool debugger;

    static void msleep(int ms) { QThread::msleep(ms); }
    virtual void run();
    bool getDeviseInBase(int idDevice, QString &nameDevice, QString &port);
    bool setDeviseInBase(int idDevice,
                         const QString &nameDevice,
                         const QString &port,
                         const QString &comment,
                         int state);
    bool insertDeviseInBase(int idDevice, QString &nameDevice, QString &port);

public slots:
    bool searchDeviceMethod(int device,
                            QString &devName,
                            QString &comStr,
                            QString &devComent,
                            const bool withTest = false);
    void getModem_Info();

signals:
    void emit_getModem_Info();

public slots:

signals:
    void emitDeviceSearch(
        int device, int result, QString dev_name, QString dev_comment, QString dev_port);
    void emitDeviceSearchFinished();
};
