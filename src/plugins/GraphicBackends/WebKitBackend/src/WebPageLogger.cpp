/* @file Класс для перехвата сообщений javascript */

#include "WebPageLogger.h"

#include <SDK/PaymentProcessor/Core/IGUIService.h>

void WebPageLogger::javaScriptConsoleMessage(const QString &message,
                                             int lineNumber,
                                             const QString &sourceID) {
    LOG(m_Log, LogLevel::Normal, QString("%1, [%2]: %3").arg(sourceID).arg(lineNumber).arg(message));
}

void WebPageLogger::javaScriptAlert(QWebFrame *frame, const QString &msg) {
    Q_UNUSED(frame);
    QVariantMap popupParameters;
    popupParameters.insert("type", "popup");
    popupParameters.insert("cancelable", "true");
    popupParameters.insert("message", "Alert: " + msg);
    popupParameters.insert("result", "");
    m_CoreProxy->getCore()->getGUIService()->showPopup("InfoPopup", popupParameters);
}