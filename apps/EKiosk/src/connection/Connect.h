#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtNetwork/QNetworkInterface>
#include <Common/QtHeadersEnd.h>

// Project
#include "CheckConnection.h"

#ifdef Q_OS_WIN
#include "RasConnection.h"
#endif

#ifdef Q_OS_WIN
class RasConnection;
#endif

namespace Connection
{
    enum conSate
    {
        conStateUp = 0,
        conStateDown = 1,
        conStateUpping = 2,
        conStateError = 3,
        SendingSMS = 4,
        GetSimData = 5
    };
} // namespace Connection

class ConnectionPart : public QObject
{
    Q_OBJECT

  public:
    ConnectionPart(QObject *parent = 0);

    QStringList getLocalConnectionList();
    void setConnectionConfig(QString pointName);
    QStringList getRasConnectionList();
    QString getActiveConnection();
    bool getNowConnectionState(QStringList &lstCon);
    void startCheckConnection();
    bool checkConnection(int type = 1);
    void connectNet();
    bool disconnectNet();
    bool restartNet();
    void setEndpoint(int respTime, QString serverAddress);
    void stopReconnect();
    void closeThis();
    bool restartWindows(bool restart);
    int createNewDialupConnection(QString conName, QString devName, QString phone, QString login, QString pass);
    bool hasInstalledModems(QStringList &lstModemList);
    int conState;
    QTimer *daemonTimer;

  private:
#ifdef Q_OS_WIN
    RasConnection *rasConn;
#endif
    CheckConnection *checkConn;

    QString connectionName;

  public slots:
    void nowStateDialuping(int state);
    void setDateTimeIn(QString dt);
  signals:
    void emit_connState(QString state, QString comment);
    void emit_errorState(QString errNum, QString errComment);
    void emit_ConnectionUp();
    void emit_ConnectionError();
    void emit_Ping(bool sts);
    void emit_checkConState();
    void emit_toLoging(int sts, QString title, QString text);
};
