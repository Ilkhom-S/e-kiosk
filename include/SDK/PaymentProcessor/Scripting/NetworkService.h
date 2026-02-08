/* @file Прокси класс для работы с сетью. */

#pragma once

#include <QtCore/QDebug>
#include <QtCore/QFutureWatcher>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Humo/Request.h>
#include <SDK/PaymentProcessor/Humo/RequestSender.h>
#include <SDK/PaymentProcessor/Humo/Response.h>

class NetworkTaskManager;
class NetworkTask;

namespace SDK {
namespace PaymentProcessor {

class ICore;
class INetworkService;

namespace Scripting {

//---------------------------------------------------------------------------
/// Класс запроса для скриптов.
class Request : public SDK::PaymentProcessor::Humo::Request {
public:
    /// Конструктор.
    Request(const QVariantMap &aRequestParameters);
};

//---------------------------------------------------------------------------
/// Класс ответа для скриптов.
class Response : public SDK::PaymentProcessor::Humo::Response {
public:
    /// Конструктор.
    Response(const SDK::PaymentProcessor::Humo::Request &aRequest, const QString &aResponseString);
};

//------------------------------------------------------------------------------
/// Прокси класс для работы с сетью.
class NetworkService : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionStatus)

public:
    /// Конструктор.
    NetworkService(ICore *aCore);
    /// Деструктор.
    ~NetworkService();

public slots:
    /// Выполнение запроса GET на адрес aUrl с данными aData.
    int get(const QString &aUrl, const QString &aData);

    /// Выполнение запроса POST на адрес aUrl с данными aData.
    bool post(const QString &aUrl, const QString &aData);

    /// Получение статуса соединения.
    bool isConnected();

public slots:
    void sendRequest(const QString &aUrl, QVariantMap aParameters);
    void sendReceipt(const QString &aEmail, const QString &aContact);

private slots:
    /// Сетевой запрос завершился.
    void taskComplete();

    /// Получения событий ядра
    void onEvent(const SDK::PaymentProcessor::Event &aEvent);

    void onResponseFinished();

    void onResponseSendReceiptFinished();

signals:
    /// Срабатывает при завершении запроса aRequest.
    void complete(bool aError, QString aResult);

    /// Срабатывает при получении сигнала о состоянии соединения.
    void connectionStatus();

    void requestCompleted(const QVariantMap &aResult);

    void sendReceiptComplete(bool aResult);

private:
    /// Создаёт ответ по классу запроса и данным.
    SDK::PaymentProcessor::Humo::Response *
    createResponse(const SDK::PaymentProcessor::Humo::Request &aRequest, const QString &aData);

    /// Отправляет POST запрос.
    SDK::PaymentProcessor::Humo::Response *postRequest(const QString &aUrl,
                                                       QVariantMap &aRequestParameters);

    /// Отправляет запрос внутренне.
    Response *sendRequestInternal(Request *aRequest, const QString &aUrl);

private:
    /// Указатель на ядро.
    ICore *m_Core;
    /// Указатель на сетевой сервис.
    INetworkService *m_NetworkService;
    /// Менеджер задач.
    NetworkTaskManager *m_TaskManager;
    /// Текущая задача.
    NetworkTask *m_CurrentTask;

private:
    /// SD.
    QString m_SD;
    /// AP.
    QString m_AP;
    /// OP.
    QString m_OP;
    /// Отправитель запросов.
    QSharedPointer<SDK::PaymentProcessor::Humo::RequestSender> m_RequestSender;
    /// Наблюдатель за ответами.
    QFutureWatcher<SDK::PaymentProcessor::Humo::Response *> m_ResponseWatcher;
    /// Наблюдатель за отправкой чеков.
    QFutureWatcher<Response *> m_ResponseSendReceiptWatcher;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
