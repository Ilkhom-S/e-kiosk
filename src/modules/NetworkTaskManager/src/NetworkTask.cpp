/* @file Реализация базового класса для сетевого запроса. */

#include "NetworkTask.h"

#include <QtCore/QMutexLocker>
#include <QtCore/QTimeZone>

#include "DataStream.h"
#include "IVerifier.h"
#include "NetworkTaskManager.h"

NetworkTask::NetworkTask()
    : m_Type(Get), m_Timeout(0), m_Error(NotReady), m_HttpError(0), m_Processing(false),
      m_ParentThread(QThread::currentThread()), m_Flags(None), m_Size(0), m_CurrentSize(0) {
    m_Timer.setParent(this);
    m_Timer.setSingleShot(true);

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onTimeout()));
}

//------------------------------------------------------------------------
NetworkTask::~NetworkTask() = default;

//------------------------------------------------------------------------
NetworkTaskManager *NetworkTask::getManager() const {
    return m_Manager;
}

//------------------------------------------------------------------------
QString qtNetworkError(QNetworkReply::NetworkError aError) {
    switch (aError) {
    case QNetworkReply::ConnectionRefusedError:
        return "ConnectionRefusedError";
    case QNetworkReply::RemoteHostClosedError:
        return "RemoteHostClosedError";
    case QNetworkReply::HostNotFoundError:
        return "HostNotFoundError";
    case QNetworkReply::TimeoutError:
        return "TimeoutError";
    case QNetworkReply::OperationCanceledError:
        return "OperationCanceledError";
    case QNetworkReply::SslHandshakeFailedError:
        return "SslHandshakeFailedError";
    case QNetworkReply::TemporaryNetworkFailureError:
        return "TemporaryNetworkFailureError";
    case QNetworkReply::UnknownNetworkError:
        return "UnknownNetworkError";

    // proxy errors (101-199):
    case QNetworkReply::ProxyConnectionRefusedError:
        return "ProxyConnectionRefusedError";
    case QNetworkReply::ProxyConnectionClosedError:
        return "ProxyConnectionClosedError";
    case QNetworkReply::ProxyNotFoundError:
        return "ProxyNotFoundError";
    case QNetworkReply::ProxyTimeoutError:
        return "ProxyTimeoutError";
    case QNetworkReply::ProxyAuthenticationRequiredError:
        return "ProxyAuthenticationRequiredError";
    case QNetworkReply::UnknownProxyError:
        return "UnknownProxyError";

    // content errors (201-299):
    case QNetworkReply::ContentAccessDenied:
        return "ContentAccessDenied";
    case QNetworkReply::ContentOperationNotPermittedError:
        return "ContentOperationNotPermittedError";
    case QNetworkReply::ContentNotFoundError:
        return "ContentNotFoundError";
    case QNetworkReply::AuthenticationRequiredError:
        return "AuthenticationRequiredError";
    case QNetworkReply::ContentReSendError:
        return "ContentReSendError";
    case QNetworkReply::UnknownContentError:
        return "UnknownContentError";

    // protocol errors
    case QNetworkReply::ProtocolUnknownError:
        return "ProtocolUnknownError";
    case QNetworkReply::ProtocolInvalidOperationError:
        return "ProtocolInvalidOperationError";
    case QNetworkReply::ProtocolFailure:
        return "ProtocolFailure";

    default:
        return "";
    }
}

//------------------------------------------------------------------------
QString NetworkTask::errorString() {
    switch (getError()) {
    case NoError:
        return "no error";
    case Stream_WriteError:
        return "stream write error";
    case UnknownOperation:
        return "unknown operation specified";
    case Timeout:
        return "request timeout exceeded";
    case BadTask:
        return "bad task";
    case VerifyFailed:
        return "verify failed";
    case TaskFailedButVerified:
        return "network error but content is verified";
    case NotReady:
        return "task is in progress";

    default:
        if (m_NetworkReplyError.isEmpty()) {
            return QString("(%1) %2")
                .arg(getError())
                .arg(qtNetworkError((QNetworkReply::NetworkError)getError()));
        } else {
            return QString("(%1) %2").arg(getError()).arg(m_NetworkReplyError);
        }
    }
}

//------------------------------------------------------------------------
void NetworkTask::setProcessing(NetworkTaskManager *aManager, bool aProcessing) {
    m_Manager = aManager;
    m_Processing = aProcessing;

    if (aProcessing) {
        if (m_Timer.interval() != 0) {
            m_Timer.start();
        }

        clearErrors();

        this->moveToThread(getManager()->thread());

        m_ProcessingMutex.lock();
    } else {
        m_Timer.stop();

        if (m_ParentThread != nullptr && m_ParentThread->isRunning()) {
            this->moveToThread(m_ParentThread);
        }

        if (m_Verifier) {
            if (!m_Verifier->verify(this, getDataStream()->readAll())) {
                if (getError() == NoError) {
                    setError(VerifyFailed);
                }
            } else {
                if (getError() != NoError) {
                    setError(TaskFailedButVerified);
                }
            }
        }

        emit onComplete();

        m_ProcessingCondition.wakeAll();
    }
}

//------------------------------------------------------------------------
void NetworkTask::resetTimer() {
    if (m_Processing) {
        if (m_Timer.interval() != 0) {
            m_Timer.start();
        }
    }
}

//------------------------------------------------------------------------
void NetworkTask::clearErrors() {
    m_Error = QNetworkReply::NoError;
    m_NetworkReplyError.clear();
    m_HttpError = 0;
}

//------------------------------------------------------------------------
void NetworkTask::setType(Type aType) {
    m_Type = aType;
}

//------------------------------------------------------------------------
NetworkTask::Type NetworkTask::getType() const {
    return m_Type;
}

//------------------------------------------------------------------------
void NetworkTask::setUrl(const QUrl &aUrl) {
    m_Url = aUrl;
}

//------------------------------------------------------------------------
const QUrl &NetworkTask::getUrl() const {
    return m_Url;
}

//------------------------------------------------------------------------
void NetworkTask::setTimeout(int aMsec) {
    m_Timer.setInterval(aMsec);
}

//------------------------------------------------------------------------
int NetworkTask::getTimeout() const {
    return m_Timeout;
}

//------------------------------------------------------------------------
void NetworkTask::setFlags(Flags aFlags) {
    m_Flags = aFlags;
}

//------------------------------------------------------------------------
NetworkTask::Flags NetworkTask::getFlags() const {
    return m_Flags;
}

//------------------------------------------------------------------------
void NetworkTask::setError(int aError, const QString &aMessage) {
    if ((getError() != QNetworkReply::NoError) &&
        (aError == QNetworkReply::OperationCanceledError)) {
        return;
    }

    m_Error = aError;
    m_NetworkReplyError = aMessage;
}

//------------------------------------------------------------------------
int NetworkTask::getError() const {
    return m_Error;
}

//------------------------------------------------------------------------
void NetworkTask::setHttpError(int aError) {
    m_HttpError = aError;
}

//------------------------------------------------------------------------
int NetworkTask::getHttpError() const {
    return m_HttpError;
}

//------------------------------------------------------------------------
void NetworkTask::setVerifier(IVerifier *aVerifier) {
    m_Verifier = QSharedPointer<IVerifier>(aVerifier);
}

//------------------------------------------------------------------------
IVerifier *NetworkTask::getVerifier() const {
    return m_Verifier.data();
}

//------------------------------------------------------------------------
void NetworkTask::setDataStream(DataStream *aDataStream) {
    m_DataStream = QSharedPointer<DataStream>(aDataStream);
}

//------------------------------------------------------------------------
DataStream *NetworkTask::getDataStream() const {
    return m_DataStream.data();
}

//------------------------------------------------------------------------
void NetworkTask::onTimeout() {
    setError(NetworkTask::Timeout);

    getManager()->removeTask(this);
}

//------------------------------------------------------------------------
NetworkTask::TByteMap &NetworkTask::getRequestHeader() {
    return m_RequestHeader;
}

//------------------------------------------------------------------------
NetworkTask::TByteMap &NetworkTask::getResponseHeader() {
    return m_ResponseHeader;
}

//------------------------------------------------------------------------
void NetworkTask::waitForFinished() {
    if (m_Processing) {
        m_ProcessingCondition.wait(&m_ProcessingMutex);
    }

    // Асинхронно разблокируем мьютекс, если задача не завершена
    m_ProcessingMutex.tryLock();
    m_ProcessingMutex.unlock();
}

//------------------------------------------------------------------------
void NetworkTask::setSize(qint64 aCurrent, qint64 aTotal) {
    m_CurrentSize = aCurrent;
    m_Size = aTotal;

    emit onProgress(aCurrent, aTotal);
}

//------------------------------------------------------------------------
qint64 NetworkTask::getSize() const {
    return m_Size;
}

//------------------------------------------------------------------------
qint64 NetworkTask::getCurrentSize() const {
    return m_CurrentSize;
}

//------------------------------------------------------------------------
void NetworkTask::setTag(const QVariant &aTag) {
    m_Tag = aTag;
}

//------------------------------------------------------------------------
const QVariant &NetworkTask::getTag() const {
    return m_Tag;
}

//------------------------------------------------------------------------
QDateTime NetworkTask::getServerDate() const {
    QDateTime date;

    // Проверим на наличие серверной даты в ответе
    if (m_ResponseHeader.contains("Date")) {
        // Пример: Mon, 05 Sep 2011 10:43:11 GMT
        QString dateString = QString::fromLatin1(m_ResponseHeader["Date"]);
        dateString.replace(0, dateString.indexOf(",") + 2, "");
        dateString.chop(4);

        dateString.replace("Jan", "01")
            .replace("Feb", "02")
            .replace("Mar", "03")
            .replace("Apr", "04")
            .replace("May", "05")
            .replace("Jun", "06")
            .replace("Jul", "07")
            .replace("Aug", "08")
            .replace("Sep", "09")
            .replace("Oct", "10")
            .replace("Nov", "11")
            .replace("Dec", "12");

        date = QDateTime::fromString(dateString, "dd MM yyyy hh:mm:ss");
        // QTimeZone::UTC is available in Qt 5.2+, which is the practical minimum
        date.setTimeZone(QTimeZone::UTC);
    }

    return date;
}

//------------------------------------------------------------------------
