/* @file Реализация графического объекта WebEngine для отображения веб-контента. */

#include "WebGraphicsItem.h"

#include <QtCore/QFile>
#include <QtCore/QMetaEnum>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineSettings>
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
    // Создаем WebEngine виджет для отображения веб-контента
    m_WebView = QSharedPointer<QWebEngineView>(new QWebEngineView());

    m_WebView->setPage(new WebPageLogger(this, m_CoreProxy, m_Log));

    // Настраиваем параметры безопасности и функциональности WebEngine
    auto *settings = m_WebView->settings();
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    // Note: JavascriptCanCloseWindows and DeveloperExtrasEnabled may not be available in Qt 5.15
    // settings->setAttribute(QWebEngineSettings::JavascriptCanCloseWindows, false);
    // settings->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled,
    //                      aInfo.parameters.value(CWebGraphicsItem::DebugKey).toString() ==
    //                      "true");
    m_WebView->setWindowFlags(Qt::FramelessWindowHint);

    // Добавляем горячую клавишу F5 для обновления страницы
    QKeySequence keysRefresh(QKeySequence::Refresh);
    QAction *actionRefresh = new QAction(this);
    actionRefresh->setShortcut(keysRefresh);
    m_WebView->addAction(actionRefresh);
    connect(actionRefresh, &QAction::triggered, this, &WebGraphicsItem::onLoadFinished);

    // Скрываем контекстное меню 'Обновить'
    m_WebView->page()->action(QWebEnginePage::Reload)->setVisible(false);

    // Подключаем сигналы WebEngine для обработки загрузки и JavaScript взаимодействия
    connect(m_WebView.get(), &QWebEngineView::loadFinished, this, &WebGraphicsItem::onLoadFinished);
    connect(m_WebView.get(), &QWebEngineView::urlChanged, this, &WebGraphicsItem::onUrlChanged);

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

    // Анализируем и загружаем контент
    QString path = aInfo.parameters.value(CWebGraphicsItem::StartPageKey, "");
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
            m_Error = QString("Failed to load content file: %1").arg(path);
            return;
        }

        QTextStream stream(&content);
        QString htmlContent = stream.readAll();
        QUrl baseUrl = QUrl::from_LocalFile(aInfo.directory + "/");
        m_WebView->setHtml(htmlContent, baseUrl);
    }
    m_Url = path;
}

//---------------------------------------------------------------------------
// Деструктор. Освобождает ресурсы WebEngine виджета.
WebGraphicsItem::~WebGraphicsItem() {
    // WebEngine resources are automatically cleaned up by QSharedPointer
}

//------------------------------------------------------------------------------
void WebGraphicsItem::onRefresh() {
    m_WebView->load(QUrl(m_Url));
}

//------------------------------------------------------------------------------

/// Обработчик завершения загрузки страницы WebEngine.
/// Настраивает JavaScript взаимодействие и инициализирует обработчики.
/// @param aOk true если страница загружена успешно
void WebGraphicsItem::onLoadFinished(bool aOk) {
    if (aOk) {
        m_Item_Loaded = true;

        // Настраиваем JavaScript объекты для взаимодействия с платежной системой
        // В WebEngine JavaScript объекты добавляются через runJavaScript
        QString jsSetup = QString("if (typeof %1 === 'undefined') { %1 = {}; }"
                                  "console.log('WebEngine container initialized');")
                              .arg(CWebGraphicsItem::ContainerScriptObject);

        m_WebView->page()->runJavaScript(jsSetup);

        // Инициализируем JavaScript обработчики если они существуют
        m_WebView->page()->runJavaScript(
            QString("if (typeof initialize === 'function') { "
                    "    try { initialize(); console.log('JavaScript initialized successfully'); } "
                    "    catch(e) { console.error('Initialize error:', e); } "
                    "} else { "
                    "    console.warn('No initialize function found in page'); "
                    "}"));

        // Обрабатываем отложенные сигналы
        while (!m_SignalQueue.isEmpty()) {
            auto signalData = m_SignalQueue.takeFirst();
            QString signalName = signalData.first;
            QList<QVariant> args = signalData.second;

            // Вызываем JavaScript обработчики для сигналов
            if (signalName == "onShow") {
                m_WebView->page()->runJavaScript("if (typeof onShow === 'function') { onShow(); }");
            } else if (signalName == "onHide") {
                m_WebView->page()->runJavaScript("if (typeof onHide === 'function') { onHide(); }");
            } else if (signalName == "onReset") {
                // Для reset передаем параметры как JSON
                if (!args.isEmpty() && args.first().canConvert<QVariantMap>()) {
                    QVariantMap params = args.first().toMap();
                    QString paramsJson = "{";
                    for (auto it = params.begin(); it != params.end(); ++it) {
                        if (it != params.begin())
                            paramsJson += ",";
                        paramsJson += QString("\"%1\":\"%2\"").arg(it.key(), it.value().toString());
                    }
                    paramsJson += "}";
                    m_WebView->page()->runJavaScript(
                        QString("if (typeof onReset === 'function') { onReset(%1); }")
                            .arg(paramsJson));
                }
            } else if (signalName == "onNotify") {
                // Для notify передаем reason и параметры
                if (args.size() >= 2) {
                    QString reason = args.first().toString();
                    QVariantMap params = args.last().toMap();
                    QString paramsJson = "{";
                    for (auto it = params.begin(); it != params.end(); ++it) {
                        if (it != params.begin())
                            paramsJson += ",";
                        paramsJson += QString("\"%1\":\"%2\"").arg(it.key(), it.value().toString());
                    }
                    paramsJson += "}";
                    m_WebView->page()->runJavaScript(
                        QString("if (typeof onNotify === 'function') { onNotify('%1', %2); }")
                            .arg(reason, paramsJson));
                }
            }
        }

        LOG(m_Log,
            LogLevel::Normal,
            "WebEngine page loaded and JavaScript initialized successfully");
    } else {
        m_Error = "Failed to load web page";
        LOG(m_Log, LogLevel::Error, m_Error);
    }
}

//------------------------------------------------------------------------------
/// Обработчик изменения URL страницы.
/// @param aUrl Новый URL страницы
void WebGraphicsItem::onUrlChanged(const QUrl &aUrl) {
    m_Url = aUrl.toString();
    LOG(m_Log, LogLevel::Normal, QString("WebEngine URL changed to: %1").arg(m_Url));
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
QWidget *WebGraphicsItem::getNativeWidget() const {
    return m_WebView.data();
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
QVariantMap WebGraphicsItem::getContext() const {
    return m_Context;
}