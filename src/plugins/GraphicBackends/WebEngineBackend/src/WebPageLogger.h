/* @file Класс для перехвата сообщений javascript в WebEngine */

#pragma once

#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtWebEngineWidgets/QWebEnginePage>

#include <Common/ILog.h>

#include <SDK/PaymentProcessor/Scripting/Core.h>

class WebPageLogger : public QWebEnginePage {
    Q_OBJECT
public:
    WebPageLogger::WebPageLogger(QObject *aParent,
                                 SDK::PaymentProcessor::Scripting::Core *aCoreProxy,
                                 ILog *aLog)
        : QWebEnginePage(aParent), mCoreProxy(aCoreProxy), mLog(aLog) {}
    ~WebPageLogger() {}

protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                          const QString &message,
                                          int lineNumber,
                                          const QUrl &sourceID);
    virtual void javaScriptAlert(const QUrl &securityOrigin, const QString &msg);

private:
    SDK::PaymentProcessor::Scripting::Core *mCoreProxy;
    ILog *mLog;
};
