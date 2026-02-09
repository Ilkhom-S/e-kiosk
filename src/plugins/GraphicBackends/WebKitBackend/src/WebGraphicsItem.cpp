/* @file Графический объект. */

#include "WebGraphicsItem.h"

#include <QtCore/QFile>
#include <QtCore/QMetaEnum>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtWebKit/QWebElementCollection>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebPage>
#include <QtWidgets/QAction>
#include <QtWidgets/QGraphicsRectItem>

#include "WebPageLogger.h"

namespace CWebGraphicsItem {
const char ContainerScriptObject[] = "Container";
const char StartPageKey[] = "start_page";
const char HeightKey[] = "height";
const char WidthKey[] = "width";
const char DebugKey[] = "debug";
const char HandlerScriptClass[] = "main";
const char InitializeHandlerSignature[] = "initialize()";
} // namespace CWebGraphicsItem

//---------------------------------------------------------------------------
WebGraphicsItem::WebGraphicsItem(const SDK::GUI::GraphicsItem_Info &aInfo,
                                 SDK::PaymentProcessor::Scripting::Core *aCore,
                                 ILog *aLog)
    : m_CoreProxy(aCore), m_Log(aLog), m_Item_Loaded(false), m_Context(aInfo.context) {
    m_WebView = QSharedPointer<QGraphicsWebView>(new QGraphicsWebView());

    m_WebView->setPage(new WebPageLogger(this, m_CoreProxy, m_Log));

    m_WebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    m_WebView->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    m_WebView->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    m_WebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    m_WebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    m_WebView->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
    m_WebView->settings()->setAttribute(QWebSettings::JavascriptCanCloseWindows, false);
    m_WebView->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,
                                        aInfo.parameters[CWebGraphicsItem::DebugKey] == "true");
    m_WebView->setWindowFlags(Qt::FramelessWindowHint);

    // Добавляем обновление страницы по F5
    QKeySequence keysRefresh(QKeySequence::Refresh);
    QAction *actionRefresh = new QAction(this);
    actionRefresh->setShortcut(keys_refresh);
    m_WebView->addAction(actionRefresh);
    connect(actionRefresh, SIGNAL(triggered()), SLOT(onRefresh()));

    // Скрываем контекстное меню  'Обновить'
    m_WebView->page()->action(QWebPage::Reload)->setVisible(false);

    connect(m_WebView->page()->mainFrame(),
            SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(onJavaScriptWindowObjectCleared()));

    // Получаем разрешение из конфига виджета (секция [web]).
    if (!aInfo.parameters.contains(CWebGraphicsItem::WidthKey) ||
        !aInfo.parameters.contains(CWebGraphicsItem::HeightKey)) {
        LOG(m_Log, LogLevel::Error, "Widget dimensions (width or height) missing.");
        return;
    }

    m_WebView->setGeometry(QRect(0,
                                 0,
                                 aInfo.parameters[CWebGraphicsItem::WidthKey].toInt(),
                                 aInfo.parameters[CWebGraphicsItem::HeightKey].toInt()));

    // Анализируем контент.
    QString path = aInfo.parameters[CWebGraphicsItem::StartPageKey];
    if (path.startsWith("http")) {
        // Загружаем удаленный адрес
        m_WebView->load(QUrl(path));
    } else {
        // Загружаем локальный контент
        path = aInfo.directory + "/" + path;
        QFile content(path);

        if (!content.open(QIODevice::ReadOnly | QIODevice::Text)) {
            LOG(m_Log,
                LogLevel::Error,
                QString("Failed to load html content file '%1'.").arg(path));
            return;
        }
        QTextStream stream(&content);
        m_WebView->setHtml(stream.readAll(), QUrl::fromLocalFile(aInfo.directory + "/"));
    }
    m_Url = path;

    // Импорт enum'a в виде свойств (QWebKit не понимает энумераторы внутри QObjectа, объявленные
    // через Q_ENUMS).
    const QMetaObject *metaObject = m_EventTypeMetaInfo.metaObject();
    QMetaEnum metaEnum = metaObject->enumerator(metaObject->indexOfEnumerator("Enum"));

    for (int keyIndex = 0; keyIndex < metaEnum.keyCount(); keyIndex++) {
        m_EventTypeMetaInfo.setProperty(metaEnum.key(keyIndex), metaEnum.value(keyIndex));
    }
}

//------------------------------------------------------------------------------
void WebGraphicsItem::onRefresh() {
    m_WebView->load(QUrl(m_Url));
}

//------------------------------------------------------------------------------
void WebGraphicsItem::onJavaScriptWindowObjectCleared() {
    QWebFrame *frame = qobject_cast<QWebFrame *>(sender());

    if (frame) {
        // Добавляем типы событий.
        frame->addToJavaScriptWindowObject(SDK::PaymentProcessor::Scripting::CProxyNames::EventType,
                                           &m_EventTypeMetaInfo);
        frame->addToJavaScriptWindowObject(SDK::PaymentProcessor::Scripting::CProxyNames::Core,
                                           m_CoreProxy,
                                           QWebFrame::QtOwnership); // TODO QJSEngine->QWebFrame ?
        frame->addToJavaScriptWindowObject(CWebGraphicsItem::ContainerScriptObject,
                                           this,
                                           QWebFrame::QtOwnership); // TODO QJSEngine->QWebFrame ?

        connect(frame, SIGNAL(loadFinished(bool)), SLOT(onFrameLoaded(bool)), Qt::UniqueConnection);
    }
}

//---------------------------------------------------------------------------
void WebGraphicsItem::onFrameLoaded(bool aOk) {
    QWebFrame *frame = qobject_cast<QWebFrame *>(sender());

    if (frame && aOk) {
        foreach (QWebElement tag, frame->findAllElements("script")) {
            if (tag.hasClass(CWebGraphicsItem::HandlerScriptClass)) {
                // Производим инициализацию скрипта в странице
                QVariant result = tag.evaluateJavaScript(
                    QString("%1; true").arg(CWebGraphicsItem::InitializeHandlerSignature));

                if (result.typeId() != QMetaType::Bool || result.toBool() != true) {
                    LOG(m_Log,
                        LogLevel::Error,
                        "Web frame has no initialize() method or error occured. Graphics events "
                        "are inaccessible.");
                }

                while (!m_SignalQueue.isEmpty()) {
                    QString signalName = m_SignalQueue.first().first;

                    switch (m_SignalQueue.first().second.count()) {
                    case 0:
                        QMetaObject::invokeMethod(this, signalName.toLatin1());
                        break;
                    case 1:
                        QMetaObject::invokeMethod(
                            this,
                            signalName.toLatin1(),
                            Q_ARG(QVariantMap, m_SignalQueue.first().second.first().toMap()));
                        break;
                    case 2:
                        QMetaObject::invokeMethod(
                            this,
                            signalName.toLatin1(),
                            Q_ARG(QString, m_SignalQueue.first().second.first().toString()),
                            Q_ARG(QVariantMap, m_SignalQueue.first().second.last().toMap()));
                        break;

                    default:
                        LOG(m_Log,
                            LogLevel::Warning,
                            QString("Signal with wrong arguments queued: %1. Failed to emit.")
                                .arg(signalName));
                    }

                    m_SignalQueue.takeFirst();
                }

                m_Item_Loaded = true;
            }
        }
    } else {
        LOG(m_Log, LogLevel::Warning, "Cannot load frame " + m_WebView->title());
    }
}

//---------------------------------------------------------------------------
void WebGraphicsItem::show() {
    m_Item_Loaded ? emit onShow()
                  : m_SignalQueue.push_back(qMakePair(QString("onShow"), QList<QVariant>()));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::hide() {
    m_Item_Loaded ? emit onHide()
                  : m_SignalQueue.push_back(qMakePair(QString("onHide"), QList<QVariant>()));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::reset(const QVariantMap &aParameters) {
    m_Item_Loaded
        ? emit onReset(aParameters)
        : m_SignalQueue.push_back(qMakePair(QString("onReset"), QList<QVariant>() << aParameters));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::notify(const QString &aReason, const QVariantMap &aParameters) {
    m_Item_Loaded ? emit onNotify(aReason, aParameters)
                  : m_SignalQueue.push_back(qMakePair(QString("onNotify"),
                                                      QList<QVariant>() << aReason << aParameters));
}

//---------------------------------------------------------------------------
QQuickItem *WebGraphicsItem::getWidget() const {
    // FIXME !!!
    return nullptr; // m_WebView.data();
}

//---------------------------------------------------------------------------
QVariantMap WebGraphicsItem::getContext() const {
    return m_Context;
}

//---------------------------------------------------------------------------
bool WebGraphicsItem::isValid() const {
    return !m_WebView.isNull();
}

//---------------------------------------------------------------------------
QString WebGraphicsItem::getError() const {
    return m_Error;
}

//---------------------------------------------------------------------------
