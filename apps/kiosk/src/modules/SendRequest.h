#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtXml/QDomNode>
#include <Common/QtHeadersEnd.h>

namespace Request {

    namespace Type {
        enum e_type {
            GetServices = 1,
            PayAuth = 2,
            PayStatus = 3,
            GetAbonentInfo = 4,
            GetBalance = 5,
            SendMoneyOut = 6,
            SendMonitor = 7,
            SendEncashment = 8,
            SendCommandConfirm = 9,
            SendLogInfo = 10,
            CheckOnline = 11,
            SendReceipt = 12,
            SendOtp = 13,
            ConfirmOtp = 14
        };
    } // namespace Type

    namespace CommentType {
        const QString GetServices = "ASO_GET_SERVICES_3";
        const QString PayAuth = "PAY";
        const QString GetAbonentInfo = "ABONENT_INFO";
        const QString GetBalance = "GET_BALANCE";
        const QString SendMoneyOut = "ASO_MONEY_OUT";
        const QString SendMonitor = "ASO_MONITOR_2";
        const QString SendEncashment = "ASO_ENCASHMENT";
        const QString SendCommandConfirm = "ASO_CMD_CONFIRM";
        const QString SendLogInfo = "ASO_LOGINFO";
        const QString CheckOnline = "ONLINE_CHECK";
        const QString SendReceipt = "SEND_RECEIPT";
        const QString SendOtp = "SEND_OTP";
        const QString ConfirmOtp = "CONFIRM_OTP";
    } // namespace CommentType
} // namespace Request

namespace SendReq {

    const QString Title = "SEND_REQUEST";

}

#include <QtCore/QMutex>
#include <QtCore/QThread>

class SendRequest : public QThread {

    Q_OBJECT

  public:
    SendRequest(QObject *parent = 0);
    //    ~SendRequest();
    bool sendRequest(QString request, int timeOut);
    void setUrl(QString url);
    QString getHeaderRequest(int type);
    QString getFooterRequest();
    void setDbConnect(QSqlDatabase &dbName);
    void setAuthData(const QString token, const QString uuid, const QString version);
    void stopProcess();

    bool m_abort;
    QMutex mutex;
    QNetworkReply *reply;
    QNetworkAccessManager pManager;

    QString headerParamInit;
    QTimer *abortTimer;

    QString serverUrl;
    QSqlDatabase db;
    QString senderName;

    bool debugger;

    QString sendReqRes;
    int reTimerStart;
    bool delPosStreem;

    QString version;

  private:
    QString token;
    QString uuid;

    QString host();

    void toDebug(QString data);

  protected:
    void run();

  public slots:
    void slotReadyRead();
    void slotQHttpAbort();
    void slotError(QNetworkReply::NetworkError error);
    void sendRequestData();
    void timerAbortStarted();

  signals:
    void emit_QHttpAbort();
    void emit_Loging(int status, QString title, QString text);
    void emit_DomElement(const QDomNode &domElement);
    void emit_ErrResponse();
    void emit_abortTimer(int msec);
    void send_req_data();
    void check_to_abortTimer();
};
