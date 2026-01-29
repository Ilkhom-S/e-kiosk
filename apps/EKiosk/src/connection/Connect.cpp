// Project
#include "Connect.h"

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

ConnectionPart::ConnectionPart(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_WIN
    rasConn = new RasConnection(this);
    connect(rasConn, SIGNAL(emit_ConnectionError()), this, SIGNAL(emit_ConnectionError()));
    connect(rasConn, SIGNAL(emit_ConnectionUp()), this, SIGNAL(emit_ConnectionUp()));
    connect(rasConn, SIGNAL(emit_connState(QString, QString)), this, SIGNAL(emit_connState(QString, QString)));
    connect(rasConn, SIGNAL(emit_errorState(QString, QString)), this, SIGNAL(emit_errorState(QString, QString)));
    connect(rasConn, SIGNAL(emit_dialupState(int)), SLOT(nowStateDialuping(int)));
    connect(rasConn, SIGNAL(emit_toLoging(int, QString, QString)), SIGNAL(emit_toLoging(int, QString, QString)));
#endif

    checkConn = new CheckConnection(this);
    connect(checkConn, SIGNAL(emit_Ping(bool)), this, SIGNAL(emit_Ping(bool)));
    connect(checkConn, SIGNAL(emit_toLoging(int, QString, QString)), SIGNAL(emit_toLoging(int, QString, QString)));

    daemonTimer = new QTimer(this);
    connect(daemonTimer, SIGNAL(timeout()), SIGNAL(emit_checkConState()));

    conState = Connection::conStateDown;
}

void ConnectionPart::nowStateDialuping(int state)
{
    //    qDebug() << state;
    this->conState = state;
}

void ConnectionPart::setDateTimeIn(QString dt)
{
#ifdef Q_OS_WIN32
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        emit emit_toLoging(2, "SYNCHRONIZATION", QString("Error synchronization DateTime"));
        return;
    }

    QString privilege = "SeShutdownPrivilege";
    LookupPrivilegeValueW(NULL, (LPCWSTR)privilege.utf16(), &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
    {
        emit emit_toLoging(2, "SYNCHRONIZATION", QString("Error synchronization DateTime"));
        return;
    }

    emit emit_toLoging(0, "SYNCHRONIZATION", QString("Server DateTime - ") + dt);

    if (dt.length() != 19)
    {
        emit emit_toLoging(2, "SYNCHRONIZATION", QString("Error DateTime FORMAT"));
        return;
    }

    SYSTEMTIME *systime = new SYSTEMTIME();

    GetLocalTime(systime);

    systime->wYear = dt.mid(0, 4).toInt();
    systime->wMonth = dt.mid(5, 2).toInt();
    systime->wDay = dt.mid(8, 2).toInt();
    systime->wHour = dt.mid(11, 2).toInt();
    systime->wMinute = dt.mid(14, 2).toInt();
    systime->wSecond = dt.mid(17, 2).toInt();

    bool test_time = SetLocalTime(systime);

    delete systime;

    emit emit_toLoging(0, "SYNCHRONIZATION", QString("%1").arg(test_time));

    CloseHandle(hToken);
#else
    // Date/time synchronization not implemented for this platform
    emit emit_toLoging(2, "SYNCHRONIZATION", QString("Date/time synchronization not supported on this platform"));
#endif
}

void ConnectionPart::startCheckConnection()
{
    if (!this->daemonTimer->isActive())
    {
        this->daemonTimer->start(300000);
    }
}

QStringList ConnectionPart::getLocalConnectionList()
{
    QStringList interfaces;

    foreach (QNetworkInterface intf, QNetworkInterface::allInterfaces())
    {
        interfaces.append(intf.humanReadableName());
    }

    return interfaces;
}

QStringList ConnectionPart::getRasConnectionList()
{
#ifdef Q_OS_WIN
    QStringList list;
    rasConn->getConnection(list);

    for (int i = 0; i < list.count(); i++)
    {
        qDebug() << QString("connection -(%1) ").arg(i) << list.at(i);
    }

    return list;
#else
    return QStringList();
#endif
}

bool ConnectionPart::getNowConnectionState(QStringList &lstCon)
{
#ifdef Q_OS_WIN
    return rasConn->getConName(lstCon);
#else
    lstCon.clear();
    return false;
#endif
}

QString ConnectionPart::getActiveConnection()
{
    return "";
}

bool ConnectionPart::checkConnection(int type)
{
    bool checConn = false;

    checkConn->checkConnection(type);

    return checConn;
}

void ConnectionPart::closeThis()
{
    checkConn->terminate();
    checkConn->wait(100);
}

int ConnectionPart::createNewDialupConnection(QString conName, QString devName, QString phone, QString login,
                                              QString pass)
{
#ifdef Q_OS_WIN
    int status = rasConn->createNewDialupConnection(conName, devName, phone, login, pass);
    return status;
#else
    Q_UNUSED(conName);
    Q_UNUSED(devName);
    Q_UNUSED(phone);
    Q_UNUSED(login);
    Q_UNUSED(pass);
    return -1; // Not supported
#endif
}

bool ConnectionPart::hasInstalledModems(QStringList &lstModemList)
{
#ifdef Q_OS_WIN
    bool result = rasConn->HasInstalledModems(lstModemList);
    return result;
#else
    Q_UNUSED(lstModemList);
    return false;
#endif
}

void ConnectionPart::connectNet()
{
#ifdef Q_OS_WIN
    if (connectionName.toUpper() != "LOCAL CONNECTION")
    {
        rasConn->execCommand(DialupParam::StartDial);
    }
#endif
}

void ConnectionPart::setConnectionConfig(QString pointName)
{
    connectionName = pointName;
#ifdef Q_OS_WIN
    rasConn->setConnectionName(this->connectionName);
#endif
}

bool ConnectionPart::disconnectNet()
{
#ifdef Q_OS_WIN
    if (connectionName.toUpper() != "LOCAL CONNECTION")
    {
        rasConn->HangUp();
    }
#endif

    return true;
}

bool ConnectionPart::restartNet()
{
    return true;
}

void ConnectionPart::setEndpoint(int respTime, QString serverAddress)
{
    checkConn->setEndpoint(respTime, serverAddress);
}

bool ConnectionPart::restartWindows(bool restart)
{
#ifdef Q_OS_WIN
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        return false;
    }

    QString privilege = "SeShutdownPrivilege";
    LookupPrivilegeValueW(NULL, (LPCWSTR)privilege.utf16(), &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
    {
        return false;
    }

    int COMMAND_INIT = EWX_SHUTDOWN;
    if (restart)
        COMMAND_INIT = EWX_REBOOT;

    if (!ExitWindowsEx(COMMAND_INIT | EWX_FORCE, 0))
    {
        return false;
    }

    return true;
#else
    Q_UNUSED(restart);
    return false; // Not supported on this platform
#endif
}

void ConnectionPart::stopReconnect()
{
#ifdef Q_OS_WIN
    rasConn->stopReconnect();
#endif
}
