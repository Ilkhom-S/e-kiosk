#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QEventLoop>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMessageAuthenticationCode>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <Common/QtHeadersEnd.h>

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
    void sendRequest(QJsonObject json, QString url, QString requestName, int method, const int timeout = 10,
                     QVariantMap header = QVariantMap());

  signals:
    void emitResponseSuccess(QVariantMap response, QString requestName);
    void emitResponseError(QString error, QString requestName);
};

