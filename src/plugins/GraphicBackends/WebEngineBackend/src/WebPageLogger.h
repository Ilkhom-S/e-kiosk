/* @file Класс для перехвата сообщений javascript в WebEngine */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QtCore/QVariantMap>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// SDK
#include <Common/ILog.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>

class WebPageLogger : public QWebEnginePage {
    Q_OBJECT
  public:
    WebPageLogger::WebPageLogger(QObject *aParent, SDK::PaymentProcessor::Scripting::Core *aCoreProxy, ILog *aLog)
        : QWebEnginePage(aParent), mCoreProxy(aCoreProxy), mLog(aLog) {
    }
    ~WebPageLogger() {
    }

  protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber,
                                          const QUrl &sourceID);
    virtual void javaScriptAlert(const QUrl &securityOrigin, const QString &msg);

  private:
    SDK::PaymentProcessor::Scripting::Core *mCoreProxy;
    ILog *mLog;
};
