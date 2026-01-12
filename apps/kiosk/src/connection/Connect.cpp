// Project
#include "Connect.h"

ConnectionPart::ConnectionPart(QObject *parent) : QObject(parent) {
  rasConn = new RasConnection(this);
  connect(rasConn, SIGNAL(emit_ConnectionError()), this,
          SIGNAL(emit_ConnectionError()));
  connect(rasConn, SIGNAL(emit_ConnectionUp()), this,
          SIGNAL(emit_ConnectionUp()));
  connect(rasConn, SIGNAL(emit_connState(QString, QString)), this,
          SIGNAL(emit_connState(QString, QString)));
  connect(rasConn, SIGNAL(emit_errorState(QString, QString)), this,
          SIGNAL(emit_errorState(QString, QString)));
  connect(rasConn, SIGNAL(emit_dialupState(int)), SLOT(nowStateDialuping(int)));
  connect(rasConn, SIGNAL(emit_toLoging(int, QString, QString)),
          SIGNAL(emit_toLoging(int, QString, QString)));

  checkConn = new CheckConnection(this);
  connect(checkConn, SIGNAL(emit_Ping(bool)), this, SIGNAL(emit_Ping(bool)));
  connect(checkConn, SIGNAL(emit_toLoging(int, QString, QString)),
          SIGNAL(emit_toLoging(int, QString, QString)));

  daemonTimer = new QTimer(this);
  connect(daemonTimer, SIGNAL(timeout()), SIGNAL(emit_checkConState()));

  conState = Connection::conStateDoun;
}

void ConnectionPart::nowStateDialuping(int state) {
  //    qDebug() << state;
  this->conState = state;
}

void ConnectionPart::setDateTimeIn(QString dt) {
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;

  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    emit emit_toLoging(2, "SYNCHRONIZATION",
                       QString("Error synchronization DateTime"));
    return;
  }

  QString privilege = "SeShutdownPrivilege";
  LookupPrivilegeValueW(NULL, (LPCWSTR)privilege.utf16(),
                        &tkp.Privileges[0].Luid);

  tkp.PrivilegeCount = 1;
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

  if (GetLastError() != ERROR_SUCCESS) {
    emit emit_toLoging(2, "SYNCHRONIZATION",
                       QString("Error synchronization DateTime"));
    return;
  }

  emit emit_toLoging(0, "SYNCHRONIZATION", QString("Server DateTime - ") + dt);

  if (dt.length() != 19) {
    emit emit_toLoging(2, "SYNCHRONIZATION", QString("Error DateTime FORMAT"));
    return;
  }

  SYSTEMTIME *systime = new SYSTEMTIME();

  GetLocalTime(systime);

  systime->wYear = dt.midRef(0, 4).toInt();
  systime->wMonth = dt.midRef(5, 2).toInt();
  systime->wDay = dt.midRef(8, 2).toInt();
  systime->wHour = dt.midRef(11, 2).toInt();
  systime->wMinute = dt.midRef(14, 2).toInt();
  systime->wSecond = dt.midRef(17, 2).toInt();

  bool test_time = SetLocalTime(systime);

  delete systime;

  emit emit_toLoging(0, "SYNCHRONIZATION", QString("%1").arg(test_time));

  CloseHandle(hToken);
}

void ConnectionPart::startCheckConnection() {
  if (!this->daemonTimer->isActive()) {
    this->daemonTimer->start(300000);
  }
}

QStringList ConnectionPart::getLocalConnectionList() {
  QStringList interfaces;

  foreach (QNetworkInterface intf, QNetworkInterface::allInterfaces()) {
    interfaces.append(intf.humanReadableName());
  }

  return interfaces;
}

QStringList ConnectionPart::getRasConnectionList() {
  QStringList list;
  rasConn->getConnection(list);

  for (int i = 0; i < list.count(); i++) {
    qDebug() << QString("connection -(%1) ").arg(i) << list.at(i);
  }

  return list;
}

bool ConnectionPart::getNowConnectionState(QStringList &lstCon) {
  return rasConn->getConName(lstCon);
}

QString ConnectionPart::getActiveConnection() { return ""; }

bool ConnectionPart::checkConnection(int type) {
  bool checConn = false;

  checkConn->checkConnection(type);

  return checConn;
}

void ConnectionPart::closeThis() {
  checkConn->terminate();
  checkConn->wait(100);
}

int ConnectionPart::createNewDialupConnection(QString conName, QString devName,
                                              QString phone, QString login,
                                              QString pass) {
  int status =
      rasConn->createNewDialupConnection(conName, devName, phone, login, pass);
  return status;
}

bool ConnectionPart::hasInstalledModems(QStringList &lstModemList) {
  bool result = rasConn->HasInstalledModems(lstModemList);
  return result;
}

void ConnectionPart::connectNet() {
  if (connectionName.toUpper() != "LOCAL CONNECTION") {
    rasConn->execCommand(DialupParam::StartDial);
  }
}

void ConnectionPart::setConnectionConfig(QString pointName) {
  connectionName = pointName;
  rasConn->setConnectionName(this->connectionName);
}

bool ConnectionPart::disconnectNet() {
  if (connectionName.toUpper() != "LOCAL CONNECTION") {
    rasConn->HangUp();
  }

  return true;
}

bool ConnectionPart::restartNet() { return true; }

void ConnectionPart::setEndpoint(int respTime, QString serverAddress) {
  checkConn->setEndpoint(respTime, serverAddress);
}

bool ConnectionPart::restartWindows(bool restart) {
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;

  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    return false;
  }

  QString privilege = "SeShutdownPrivilege";
  LookupPrivilegeValueW(NULL, (LPCWSTR)privilege.utf16(),
                        &tkp.Privileges[0].Luid);

  tkp.PrivilegeCount = 1;
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

  if (GetLastError() != ERROR_SUCCESS) {
    return false;
  }

  int COMMAND_INIT = EWX_SHUTDOWN;
  if (restart)
    COMMAND_INIT = EWX_REBOOT;

  if (!ExitWindowsEx(COMMAND_INIT | EWX_FORCE, 0)) {
    return false;
  }

  return true;
}

void ConnectionPart::stopReconnect() { rasConn->stopReconnect(); }
