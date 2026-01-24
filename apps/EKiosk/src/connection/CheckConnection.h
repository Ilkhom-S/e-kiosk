#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QProcess>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <Common/QtHeadersEnd.h>

class QTcpSocket;
class QAbstractSocket;

namespace TypePing {
    enum i_typeping { Ping = 1, Socket = 2, Request = 3 };
} // namespace TypePing

class CheckConnection : public QThread {
    Q_OBJECT

  public:
    CheckConnection(QObject *parent = 0);
    void setEndpoint(int timeOut, QString serverAddress);
    void checkConnection(int type);

  private:
    QString serverAddress;
    QTcpSocket *m_pTcpSocket;
    QNetworkAccessManager *manager;

    QTimer *stsTimer;
    QTimer *abortTimer;

    int timePing;
    int cmd;

    virtual void run();
    bool ping(int timeOut, QString ipAddress);
    void connectToHost(QString ipAddress);

    void sendRequest(QString address);

  private slots:
    void slotConnectedOk();
    void slotErrorConnected(QAbstractSocket::SocketError err);
    void timeOutDisconnect();
    void replyFinished(QNetworkReply *reply);
    void handleSslErrors(QNetworkReply *reply, QList<QSslError> list);

  signals:
    void emit_Ping(bool sts);
    void emit_toLoging(int sts, QString title, QString text);
};