#include "JsonRequest.h"

#include "qjsonarray.h"

JsonRequest::JsonRequest(QObject *parent) : QObject(parent) {
    mgr = new QNetworkAccessManager(this);
}

void JsonRequest::setAuthData(const QString token, const QString uuid) {
    this->token = token;
    this->uuid = uuid;
}

void JsonRequest::setBaseUrl(QString url) {
    baseUrl = url.replace("aso/api/", "").replace("aso/v3/", "") /*.replace("https", "http")*/;
}

void JsonRequest::sendRequest(QJsonObject json,
                              QString url_,
                              QString requestName,
                              int method_,
                              const int timeout,
                              QVariantMap header) {
    QUrl url = QUrl(baseUrl + url_);
    qDebug() << url;

    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    QNetworkRequest request(url);

    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", token.toUtf8());
    }

    if (!header.value("token").toString().isEmpty()) {
        auto t = "Bearer " + header.value("token").toString();
        request.setRawHeader("token", t.toUtf8());
    }

    qDebug() << request.rawHeaderList();

    QNetworkReply *reply;

    if (method_ == Method::GET) {
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        reply = mgr->get(request);
    } else {
        request.setRawHeader("Content-Type", "application/json");

        reply = mgr->post(request, jsonData);
    }

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(timeout * 1000);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();

        QJsonParseError e;
        QJsonDocument doc;

        //        int code =
        //        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (reply->error() == QNetworkReply::NoError) {
            QString resp = reply->readAll();

            //            qDebug() << "readAll" << resp.toUtf8();

            // Success
            doc = QJsonDocument::from_Json(resp.toUtf8(), &e);

            if (!doc.isNull() && e.error == QJsonParseError::NoError) {
                QVariantMap data;

                if (doc.isArray()) {
                    data["data"] = doc.array().toVariantList();
                } else {
                    data = doc.object().toVariantMap();
                }

                emit emitResponseSuccess(data, requestName);
            } else {
                emit emitResponseError(e.errorString(), requestName);
            }
        } else {
            // handle error
            QString resp = reply->readAll();

            //            qDebug() << "resp " << resp;

            doc = QJsonDocument::from_Json(resp.toUtf8(), &e);
            auto response = doc.object().toVariantMap();
            auto message = response.value("message").toString();

            QString error = reply->errorString();

            emit emitResponseError(message.isEmpty() ? error : message, requestName);
        }

        //        qDebug() << "code " << code;
    } else {
        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();

        emit emitResponseError("timeout", requestName);
    }

    reply->deleteLater();
}
