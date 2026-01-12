#ifndef RASCONNECTION_H
#define RASCONNECTION_H

#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include <iostream>

using namespace std;

// Windows
// Минимальная версия необходимая для работы с RAS API - WindowsXP
#define OLD_WINVER WINVER
#if defined(WINVER)
#undef WINVER
#endif
#define WINVER 0x501
// Ensure Winsock2 is included before windows.h to avoid conflicts with
// winsock.h
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define EK_RAS_DEFINED_WIN32_LEAN
#endif
#include <windows.h>
#include <wininet.h>
// Include RAS headers after windows.h
#include <ras.h>
#include <raserror.h>
#ifdef EK_RAS_DEFINED_WIN32_LEAN
#undef WIN32_LEAN_AND_MEAN
#undef EK_RAS_DEFINED_WIN32_LEAN
#endif
#undef WINVER
#define WINVER OLD_WINVER
#undef OLD_WINVER

namespace DialupParam {

enum Cmd {

  StartDial = 1,

  StopDial = 2,

  Restart = 3,

  GetConnectionList = 4,

  Ping = 5
};
} // namespace DialupParam

namespace ErrorDialup {
enum err {
  rNoError = 0,

  rErrorCreateDialupCon = 1,

  rErrorSetDialupParam = 2
};
} // namespace ErrorDialup

namespace Connection {

namespace Type {
enum i_type {

  Dialup = 1,

  Ethernet = 2,

  WiFi = 3
};

} // namespace Type

enum conSate {
  conStateUp = 0,
  conStateDoun = 1,
  conStateUpping = 2,
  conStateError = 3,
  SendingSMS = 4,
  GetSimData = 5
};
namespace TypePing {

enum i_typeping {

  Ping = 1,

  Socket = 2,

  Request = 3
};
} // namespace TypePing
} // namespace Connection

class RasConnection : public QThread {
  Q_OBJECT

public:
  static QString G_State;
  static QString G_Comment;
  static QString G_Error_Num;
  static QString G_Error_Comment;

  static void WINAPI RasCallback(HRASCONN hrasconn, UINT unMsg,
                                 RASCONNSTATE rascs, DWORD dwError,
                                 DWORD dwExtendedError);

  int createNewDialupConnection(QString conName, QString devName, QString phone,
                                QString login, QString pass);
  bool HasInstalledModems(QStringList &lstModemList);

  RasConnection(QObject *parent = 0);

  void setConnectionName(QString name);
  void execCommand(int cmd);
  bool getConName(QStringList &lstCon);
  void HangUp();
  void getConnection(QStringList &connections);
  void stopReconnect();

private:
  QString conName;
  int Debuger;
  int nowCmd;
  QTimer *stateDialTimer;
  QTimer *reCallTimer;

  DWORD m_rasConnSize;

  virtual void run();

  static void msleep(int ms) { QThread::msleep(ms); }

  QString nowStateDial;

  HRASCONN getConnection();
  RASCONNSTATE getConnectionState(HRASCONN hRasConn);
  bool hangUpThis(HRASCONN hRasConn);

  bool isConnected();
  void checkConnection();

private slots:
  void setStateDial();

  bool Dial();

  void reCall();

signals:
  void emit_connState(QString state, QString comment);
  void emit_errorState(QString errNum, QString errComment);
  void emit_ConnectionUp();
  void emit_ConnectionError();
  void emit_TimerStateDialStart();
  void emit_ReCallTimer(int time);
  void emit_Ping(bool sts);
  void emit_toLoging(int sts, QString title, QString text);
  void emit_dialupState(int state);
};

#endif // RASCONNECTION_H
