#ifndef JSONREQUEST_H
#define JSONREQUEST_H

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageAuthenticationCode>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

enum Method {
    GET = 0,
    POST = 1,
};

class JsonRequest : public QObject {
    Q_OBJECT

  public:
    JsonRequest(QObject *parent = 0);

    void setAuthData(const QString token, const QString uuid);
    void setBaseUrl(QString url);

  private:
    QNetworkAccessManager *mgr;

    QString baseUrl;
    QString token;
    QString uuid;

  public slots:
    void sendRequest(QJsonObject json, QString url, QString requestName, int method,
                     const int timeout = 10, QVariantMap header = QVariantMap());

  signals:
    void emitResponseSuccess(QVariantMap response, QString requestName);
    void emitResponseError(QString error, QString requestName);
};

#endif  // JSONREQUEST_H
