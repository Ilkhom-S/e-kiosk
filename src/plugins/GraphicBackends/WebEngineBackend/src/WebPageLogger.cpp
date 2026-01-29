/* @file Класс для перехвата сообщений javascript в WebEngine */

// SDK
#include <SDK/PaymentProcessor/Core/IGUIService.h>

// Project
#include "WebPageLogger.h"

void WebPageLogger::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
                                             int lineNumber, const QUrl &sourceID)
{
    Q_UNUSED(level);
    LOG(mLog, LogLevel::Normal, QString("%1, [%2]: %3").arg(sourceID.toString()).arg(lineNumber).arg(message));
}

//------------------------------------------------------------------------------
void WebPageLogger::javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin);
    Q_UNUSED(msg);
    // TODO: Implement GUI service integration
    // QVariantMap popupParameters;
    // popupParameters.insert("type", "popup");
    // popupParameters.insert("cancelable", "true");
    // popupParameters.insert("message", "Alert: " + msg);
    // popupParameters.insert("result", "");
    // mCoreProxy->getCore()->getGUIService()->showPopup("InfoPopup", popupParameters);
}