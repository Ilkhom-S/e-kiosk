/* @file Класс для перехвата сообщений javascript в WebEngine */

#pragma once

#include <QWebEnginePage>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

#include <Common/ILog.h>

#include <SDK/PaymentProcessor/Scripting/Core.h>

class WebPageLogger : public QWebEnginePage {
    Q_OBJECT
public:
    WebPageLogger(QObject *aParent, SDK::PaymentProcessor::Scripting::Core *aCoreProxy, ILog *aLog)
        : QWebEnginePage(aParent), m_CoreProxy(aCoreProxy), m_Log(aLog) {}
    ~WebPageLogger() = default;

protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                          const QString &message,
                                          int lineNumber,
                                          const QUrl &sourceID);
    virtual void javaScriptAlert(const QUrl &securityOrigin, const QString &msg);

private:
    SDK::PaymentProcessor::Scripting::Core *m_CoreProxy;
    ILog *m_Log;
};
