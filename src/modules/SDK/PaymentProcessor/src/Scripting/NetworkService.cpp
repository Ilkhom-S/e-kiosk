/* @file Прокси класс для работы с сетью. */

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRandom_Generator>
#include <QtCore/QUrl>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/IDatabaseService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Humo/RequestSender.h>
#include <SDK/PaymentProcessor/Scripting/NetworkService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <Crypt/ICryptEngine.h>
#include <DatabaseProxy/IDatabaseQuery.h>
#include <NetworkTaskManager/MemoryDataStream.h>
#include <NetworkTaskManager/NetworkTask.h>
#include <NetworkTaskManager/NetworkTaskManager.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CRequest {
const QString SD = "SD";
const QString AP = "AP";
const QString OP = "OP";
const QString SESSION = "SESSION";
} // namespace CRequest

//------------------------------------------------------------------------------
Request::Request(const QVariantMap &aRequestParameters) {
    foreach (QString key, aRequestParameters.keys()) {
        addParameter(key, aRequestParameters.value(key).toString());
    }
}

//------------------------------------------------------------------------------
Response::Response(const SDK::PaymentProcessor::Humo::Request &aRequest,
                   const QString &aResponseString)
    : SDK::PaymentProcessor::Humo::Response(aRequest, aResponseString) {}

//------------------------------------------------------------------------------
NetworkService::NetworkService(ICore *aCore)
    : m_Core(aCore), m_CurrentTask(0), m_NetworkService(aCore->getNetworkService()) {
    if (m_NetworkService) {
        m_TaskManager = m_NetworkService->getNetworkTaskManager();
        m_RequestSender = QSharedPointer<PPSDK::Humo::RequestSender>(new PPSDK::Humo::RequestSender(
            m_TaskManager, m_Core->getCryptService()->getCryptEngine()));
        m_RequestSender->setResponseCreator(std::bind(
            &NetworkService::createResponse, this, std::placeholders::_1, std::placeholders::_2));

        connect(&m_ResponseWatcher, SIGNAL(finished()), SLOT(onResponseFinished()));
        connect(&m_ResponseSendReceiptWatcher,
                SIGNAL(finished()),
                SLOT(onResponseSendReceiptFinished()));
    }

    m_Core->getEventService()->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));

    m_SD = static_cast<PPSDK::TerminalSettings *>(
               m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))
               ->getKeys()[0]
               .sd;

    m_AP = static_cast<PPSDK::TerminalSettings *>(
               m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))
               ->getKeys()[0]
               .ap;

    m_OP = static_cast<PPSDK::TerminalSettings *>(
               m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))
               ->getKeys()[0]
               .op;
}

//------------------------------------------------------------------------------
NetworkService::~NetworkService() {
    if (m_TaskManager && m_CurrentTask) {
        disconnect(m_CurrentTask, SIGNAL(onComplete()), this, SLOT(taskComplete()));

        m_TaskManager->removeTask(m_CurrentTask);
        m_CurrentTask->deleteLater();

        m_CurrentTask = 0;
    }
}

//------------------------------------------------------------------------------
void NetworkService::onEvent(const SDK::PaymentProcessor::Event &aEvent) {
    switch (aEvent.getType()) {
    case SDK::PaymentProcessor::EEventType::ConnectionEstablished:
    case SDK::PaymentProcessor::EEventType::ConnectionLost:
        emit connectionStatus();
        break;
    }
}

//------------------------------------------------------------------------------
bool NetworkService::isConnected() {
    return m_NetworkService->isConnected(true);
}

//------------------------------------------------------------------------------
int NetworkService::get(const QString & /*aUrl*/, const QString & /*aData*/) {
    // TODO:

    return 0;
}

//------------------------------------------------------------------------------
bool NetworkService::post(const QString &aUrl, const QString &aData) {
    if (m_TaskManager) {
        if (m_CurrentTask) {
            disconnect(m_CurrentTask, SIGNAL(onComplete()), this, SLOT(taskComplete()));

            m_TaskManager->removeTask(m_CurrentTask);
            m_CurrentTask->deleteLater();

            m_CurrentTask = 0;
        }

        std::unique_ptr<NetworkTask> task(new NetworkTask());

        task->setUrl(aUrl);
        task->setType(NetworkTask::Post);
        task->setDataStream(new MemoryDataStream());
        task->getDataStream()->write(aData.toUtf8());

        connect(task.get(), SIGNAL(onComplete()), SLOT(taskComplete()));

        m_TaskManager->addTask(m_CurrentTask = task.release());

        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
void NetworkService::taskComplete() {
    if (m_CurrentTask && (m_CurrentTask->getError() == NetworkTask::NoError)) {
        emit complete(false, QString::from_Utf8(m_CurrentTask->getDataStream()->takeAll()));

        m_CurrentTask->deleteLater();
    } else {
        emit complete(true, QString());
    }

    m_CurrentTask = 0;
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::Humo::Response *
NetworkService::createResponse(const SDK::PaymentProcessor::Humo::Request &aRequest,
                               const QString &aData) {
    return new Response(aRequest, aData);
}

//------------------------------------------------------------------------------
void NetworkService::sendRequest(const QString &aUrl, QVariantMap aParameters) {
    if (m_RequestSender.isNull()) {
        // TODO
        return;
    }

    m_ResponseWatcher.setFuture(QtConcurrent::run(
        [this, aUrl, aParameters]() mutable { return postRequest(aUrl, aParameters); }));
}

//------------------------------------------------------------------------------
Response *NetworkService::sendRequestInternal(Request *aRequest, const QString &aUrl) {
    PPSDK::Humo::RequestSender::ESendError error;

    std::unique_ptr<SDK::PaymentProcessor::Humo::Response> response(
        m_RequestSender->post(aUrl, *aRequest, PPSDK::Humo::RequestSender::Solid, error));

    delete aRequest;
    aRequest = nullptr;

    if (response == nullptr) {
        return nullptr;
    }

    if (!dynamic_cast<Response *>(response.get())) {
        return nullptr;
    }

    return dynamic_cast<Response *>(response.release());
}

//------------------------------------------------------------------------------
void NetworkService::sendReceipt(const QString &aEmail, const QString &aContact) {
    QVariantMap params;
    params.insert("PAYER_EMAIL", aEmail);
    params.insert("CONTACT", aContact);

    QString queryStr = QString("SELECT `response` FROM `fiscal_client` WHERE `payment` = %1")
                           .arg(m_Core->getPaymentService()->getActivePayment());
    QSharedPointer<IDatabaseQuery> query(
        m_Core->getDatabaseService()->createAndExecQuery(queryStr));

    QString fiscalResponse;
    QVariantMap fiscalResult;
    if (query && query->first()) {
        fiscalResponse = query->value(0).toString();
    }

    if (!fiscalResponse.isEmpty()) {
        QJsonDocument doc = QJsonDocument::from_Json(fiscalResponse.toUtf8());
        QVariantMap payment;
        if (doc.isObject()) {
            QVariantMap root = doc.object().toVariantMap();
            QVariant v = root.value("payment");
            if (v.metaType().id() == QMetaType::QVariantMap) {
                payment = v.toMap();
            }
        }
        QVariantMap fiscalFieldData = payment.value("fiscal_field_data").toMap();
        QVariantMap fiscalPaymentData = payment.value("fiscal_payment_data").toMap();

        if (!fiscalPaymentData.isEmpty() && !fiscalPaymentData.isEmpty()) {
            foreach (auto paymentData, fiscalPaymentData.keys()) {
                QString val = fiscalPaymentData.value(paymentData).toString();
                if (val.isEmpty()) {
                    continue;
                }

                QString tr =
                    fiscalFieldData.value(paymentData).toMap().value("translation").toString();

                if (tr.isEmpty()) {
                    continue;
                }

                fiscalResult.insert(tr, val);
            }
        }
    }

    QString message =
        m_Core->getPrinterService()->loadReceipt(m_Core->getPaymentService()->getActivePayment());
    message.append("-----------------------------------");

    foreach (auto key, fiscalResult.keys()) {
        message += QString("\n%1\t%2").arg(key, fiscalResult.value(key).toString());
    }

    params.insert("PAYER_EMAIL_MESSAGE", message.toLocal8Bit().toPercentEncoding());

    PPSDK::TerminalSettings *terminalSettings = static_cast<PPSDK::TerminalSettings *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

    Request *request = new Request(params);

    QString receiptUrl = terminalSettings->getReceiptMailURL();
    m_ResponseSendReceiptWatcher.setFuture(QtConcurrent::run(
        [this, request, receiptUrl]() { return sendRequestInternal(request, receiptUrl); }));
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::Humo::Response *
NetworkService::postRequest(const QString &aUrl, QVariantMap &aRequestParameters) {
    // Предохранимся
    foreach (QString key, aRequestParameters.keys()) {
        if (key.toLower() == "amount" || key.toLower() == "amount_all") {
            aRequestParameters.remove(key);
        }
    }

    aRequestParameters.insert(CRequest::SD, m_SD);
    aRequestParameters.insert(CRequest::AP, m_AP);
    aRequestParameters.insert(CRequest::OP, m_OP);
    aRequestParameters.insert(
        CRequest::SESSION,
        QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") +
            QString("%1").arg(QRandom_Generator::global()->bounded(1000), 3, 10, QChar('0')));

    Request request(aRequestParameters);

    PPSDK::Humo::RequestSender::ESendError e;

    return m_RequestSender->post(QUrl(aUrl), request, PPSDK::Humo::RequestSender::Solid, e);
}

//------------------------------------------------------------------------------
void NetworkService::onResponseFinished() {
    std::unique_ptr<SDK::PaymentProcessor::Humo::Response> response(m_ResponseWatcher.result());
    emit requestCompleted(response == nullptr
                              ? QVariantMap()
                              : dynamic_cast<Response *>(response.release())->getParameters());
}

//------------------------------------------------------------------------------
void NetworkService::onResponseSendReceiptFinished() {
    std::unique_ptr<SDK::PaymentProcessor::Humo::Response> response(
        m_ResponseSendReceiptWatcher.result());
    emit sendReceiptComplete(response != nullptr && response->isOk());
}

//------------------------------------------------------------------------------

} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
