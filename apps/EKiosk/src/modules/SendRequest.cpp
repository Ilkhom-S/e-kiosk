#include "SendRequest.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QMessageAuthenticationCode>

SendRequest::SendRequest(QObject *parent) : QThread(parent) {
    debugger = true;
    m_abort = true;

    abortTimer = new QTimer();
    abortTimer->setSingleShot(true);

    connect(abortTimer, SIGNAL(timeout()), this, SLOT(slotQHttpAbort()));

    connect(this, SIGNAL(emit_abortTimer(int)), abortTimer, SLOT(start(int)));

    connect(this, SIGNAL(send_req_data()), SLOT(sendRequestData()));
    connect(this, SIGNAL(check_to_abortTimer()), SLOT(timerAbortStarted()));
}

void SendRequest::setDbConnect(QSqlDatabase &dbName) {
    db = dbName;
}

void SendRequest::setAuthData(const QString token, const QString uuid, const QString version) {
    this->token = token;
    this->uuid = uuid;
    this->version = version;
}

void SendRequest::setUrl(QString url) {
    serverUrl = url;
}

void SendRequest::timerAbortStarted() {
    // Устанавливаем таймер разрыва
    if (abortTimer->isActive()) {
        abortTimer->stop();
    }
}

void SendRequest::run() {
    toDebug("SendRequest::run()");

    mutex.lock();
    m_abort = false;
    mutex.unlock();

    emit emit_abortTimer(this->reTimerStart);
    emit send_req_data();

    exec();

    toDebug("SendRequest::run() end");
}

void SendRequest::sendRequestData() {
    QByteArray timeOutRec = QString::number(reTimerStart).toUtf8();
    QByteArray headerPic = headerParam_Init.toUtf8();
    QUrl url(serverUrl);

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_3);
    qDebug() << url;
    QNetworkRequest aRequest;
    aRequest.setUrl(url);
    aRequest.setRawHeader("Host", QUrl::toAce(host()));
    aRequest.setRawHeader("User-Agent", headerPic);
    aRequest.setRawHeader("Accept",
                          "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    aRequest.setRawHeader("Accept-Language", "ru,en-us;q=0.7,en;q=0.3");
    aRequest.setRawHeader("Accept-Charset", "utf-8;q=0.7,*;q=0.7");
    aRequest.setRawHeader("Keep-Alive", timeOutRec);
    aRequest.setRawHeader("Connection", "keep-alive");
    aRequest.setRawHeader("Referer", "");
    aRequest.setRawHeader("Cookie", "auth=NO");
    aRequest.setRawHeader("Content-Type", "text/xml");

    if (!token.isEmpty()) {
        QMessageAuthenticationCode code(QCryptographicHash::Sha256);
        code.setKey(sendReqRes.toUtf8());
        code.addData(uuid.toUtf8());

        aRequest.setRawHeader("Authorization", token.toUtf8());
        aRequest.setRawHeader("X-Sign", code.result().toHex());
    }

    aRequest.setSslConfiguration(config);

    QByteArray postData = sendReqRes.toUtf8();

    reply = pManager.post(aRequest, postData);
    connect(reply,
            SIGNAL(error(QNetworkReply::NetworkError)),
            SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(slotReadyRead()));
}

QString SendRequest::host() {
    auto host = serverUrl;

    if (serverUrl.startsWith("https://")) {
        host = host.mid(8);
    } else if (host.startsWith("http://")) {
        host = host.mid(7);
    }

    return host;
}

void SendRequest::stopProcess() {
    toDebug("SendRequest::stopProcess()");

    mutex.lock();
    m_abort = true;
    mutex.unlock();

    exit(0);
    wait();

    toDebug("PROCESS STOPPED");
}

void SendRequest::slotError(QNetworkReply::NetworkError error) {
    toDebug(QString("SendRequest::slotError(%1)").arg(error));
}

bool SendRequest::sendRequest(QString request, int timeOut) {
    toDebug("SendRequest::sendRequest(QString request, int timeOut)");

    // Проверим если остановлена ветка
    if (m_abort) {
        reTimerStart = timeOut;
        sendReqRes = request;
        toDebug(request);

        start();
        return true;
    }

    emit emit_Loging(1, senderName, "Подождите идет отправка запроса...");
    return false;
}

void SendRequest::slotQHttpAbort() {
    toDebug("SendRequest::slotQHttpAbort()");
    if (reply != NULL) {
        toDebug("if(reply != NULL){");
        if (!reply->isFinished()) {
            toDebug("if(!reply->isFinished()){");
            reply->abort();

            // Сигнал о том что нет ответа
            emit emit_QHttpAbort();
        }
    }
}

void SendRequest::slotReadyRead() {
    toDebug("SendRequest::slotReadyRead()");

    if (abortTimer->isActive()) {
        abortTimer->stop();
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    int errorId = reply->error();
    reply->deleteLater();

    toDebug("after reply->deleteLater()");

    // Какая ошибка
    int statusError = 0;
    // Коментарий к ошибке
    QString statusErrorComment;

    switch (errorId) {
    case QNetworkReply::NoError: {
        if (statusCode == 200) {
            if (reply->bytesAvailable() > 0) {
                QByteArray bytes = reply->readAll();
                QString str(QString::fromUtf8(bytes));

                toDebug("\n===============RESPONSE===============\n");
                toDebug(str);

                QDomDocument ddomDoc;

                if (ddomDoc.setContent(str.toUtf8())) {
                    QDomElement ddomElement = ddomDoc.documentElement();
                    emit emit_Dom_Element(ddomElement);
                } else {
                    emit emit_Loging(
                        2, senderName, "Пришел ответ от сервера Не верный формат XML.");
                    statusError = 400;
                }
            } else {
                emit emit_Loging(2, senderName, "Пришел пустой ответ от сервера.");
                statusError = 400;
            }
        }
    } break;
    case QNetworkReply::ConnectionRefusedError: {
        statusError = 1;
        statusErrorComment = "the remote server refused the connection (the server "
                             "is not accepting requests)";
    } break;
    case QNetworkReply::RemoteHostClosedError: {
        statusError = 2;
        statusErrorComment = "the remote server closed the connection prematurely, "
                             "before the entire reply was received and processed";
    } break;
    case QNetworkReply::HostNotFoundError: {
        statusError = 3;
        statusErrorComment = "Имя удаленного хоста не найдено (неверное имя хоста)";
    } break;
    case QNetworkReply::TimeoutError: {
        statusError = 4;
        statusErrorComment = "the connection to the remote server timed out";
    } break;
    case QNetworkReply::OperationCanceledError: {
        statusError = 5;
        statusErrorComment = "Нет ответа от сервера( истёк таймаут ожидания запроса ).";
    } break;
    case QNetworkReply::SslHandshakeFailedError: {
        statusError = 6;
        statusErrorComment = "the SSL/TLS handshake failed and the encrypted "
                             "channel could not be established. The "
                             "sslErrors() signal should have been emitted.";
    } break;
    case QNetworkReply::ProxyConnectionRefusedError: {
        statusError = 101;
        statusErrorComment = "the connection to the proxy server was refused (the "
                             "proxy server is not accepting requests).";
    } break;
    case QNetworkReply::ProxyConnectionClosedError: {
        statusError = 102;
        statusErrorComment = "the proxy server closed the connection prematurely, "
                             "before the entire reply was received and processed";
    } break;
    case QNetworkReply::ProxyNotFoundError: {
        statusError = 103;
        statusErrorComment = "the proxy host name was not found (invalid proxy hostname).";
    } break;
    case QNetworkReply::ProxyTimeoutError: {
        statusError = 104;
        statusErrorComment = "the connection to the proxy timed out or the proxy "
                             "did not reply in time to the request sent.";
    } break;
    case QNetworkReply::ProxyAuthenticationRequiredError: {
        statusError = 105;
        statusErrorComment = "the proxy requires authentication in order to honour "
                             "the request but did not accept any "
                             "credentials offered (if any).";
    } break;
    case QNetworkReply::ContentAccessDenied: {
        statusError = 201;
        statusErrorComment = "the access to the remote content was denied (similar "
                             "to HTTP error 401).";
    } break;
    case QNetworkReply::ContentOperationNotPermittedError: {
        statusError = 202;
        statusErrorComment = "the operation requested on the remote content is not permitted.";
    } break;
    case QNetworkReply::ContentNotFoundError: {
        statusError = 203;
        statusErrorComment = "the remote content was not found at the server "
                             "(similar to HTTP error 404).";
    } break;
    case QNetworkReply::AuthenticationRequiredError: {
        statusError = 204;
        statusErrorComment = "the remote server requires authentication to serve "
                             "the content but the credentials "
                             "provided were not accepted (if any).";
    } break;
    case QNetworkReply::ContentReSendError: {
        statusError = 205;
        statusErrorComment = "the request needed to be sent again, but this failed "
                             "for example because the upload data "
                             "could not be read a second time.";
    } break;
    case QNetworkReply::ProtocolUnknownError: {
        statusError = 301;
        statusErrorComment = "the Network Access API cannot honor the request "
                             "because the protocol is not known.";
    } break;
    case QNetworkReply::ProtocolInvalidOperationError: {
        statusError = 302;
        statusErrorComment = "the requested operation is invalid for this protocol.";
    } break;
    case QNetworkReply::UnknownNetworkError: {
        statusError = 99;
        statusErrorComment = "Неизвестная ошибка при передачи данных( нет зоны покрытия сети ).";
    } break;
    case QNetworkReply::UnknownProxyError: {
        statusError = 199;
        statusErrorComment = "an unknown proxy-related error was detected.";
    } break;
    case QNetworkReply::UnknownContentError: {
        statusError = 299;
        statusErrorComment = "an unknown error related to the remote content was detected.";
    } break;
    case QNetworkReply::ProtocolFailure: {
        statusError = 399;
        statusErrorComment = "a breakdown in protocol was detected (parsing error, "
                             "invalid or unexpected responses, etc.).";
    } break;
        //        case QNetworkReply::InternalServerError:
        //            statusError = 401;
        //        break;
    }

    // Есть ошибка
    if (statusError) {
        if (statusError != 400) {
            emit emit_Loging(2,
                             senderName,
                             QString("Error - %1, %2\n").arg(statusError).arg(statusErrorComment));
            qDebug() << QString("Error - %1, %2\n").arg(statusError).arg(statusErrorComment);
        }

        emit emit_ErrResponse();
    }

    // Останавливаем нить
    stopProcess();
}

QString SendRequest::getHeaderRequest(int type) {
    QString title = "";

    switch (type) {

    case Request::Type::GetServices: {
        title = Request::CommentType::GetServices;
    } break;
    case Request::Type::SendMonitor: {
        title = Request::CommentType::SendMonitor;
    } break;
    case Request::Type::PayAuth: {
        title = Request::CommentType::PayAuth;
    } break;
    case Request::Type::SendEncashment: {
        title = Request::CommentType::SendEncashment;
    } break;
    case Request::Type::GetAbonentInfo: {
        title = Request::CommentType::GetAbonentInfo;
    } break;
    case Request::Type::SendCommandConfirm: {
        title = Request::CommentType::SendCommandConfirm;
    } break;
    case Request::Type::SendLogInfo: {
        title = Request::CommentType::SendLogInfo;
    } break;
    case Request::Type::GetBalance: {
        title = Request::CommentType::GetBalance;
    } break;
    case Request::Type::CheckOnline: {
        title = Request::CommentType::CheckOnline;
    } break;
    case Request::Type::SendReceipt: {
        title = Request::CommentType::SendReceipt;
    } break;
    case Request::Type::SendOtp: {
        title = Request::CommentType::SendOtp;
    } break;
    case Request::Type::Confirm_Otp: {
        title = Request::CommentType::Confirm_Otp;
    } break;
    }

    QString header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<request>\n"
                     "<requestNum>" +
                     title + "</requestNum>\n";

    return header;
}

QString SendRequest::getFooterRequest() {
    return QString("<version>" + version +
                   "</version>\n"
                   "<clientType>ASO</clientType>\n"
                   "</request>\n");
}

void SendRequest::toDebug(QString data) {
    if (debugger) {
        qDebug() << data;
    }
}
