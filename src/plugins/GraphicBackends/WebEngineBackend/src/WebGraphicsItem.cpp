/* @file Реализация графического объекта WebEngine для отображения веб-контента. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QMetaEnum>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWidgets/QAction>
#include <QtWidgets/QGraphicsRectItem>
#include <Common/QtHeadersEnd.h>

// Project
#include "WebGraphicsItem.h"
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
WebGraphicsItem::WebGraphicsItem(const SDK::GUI::GraphicsItemInfo &aInfo, SDK::PaymentProcessor::Scripting::Core *aCore,
                                 ILog *aLog)
    : mCoreProxy(aCore), mLog(aLog), mItemLoaded(false), mContext(aInfo.context) {
    // Создаем WebEngine виджет для отображения веб-контента
    mWebView = QSharedPointer<QWebEngineView>(new QWebEngineView());

    mWebView->setPage(new WebPageLogger(this, mCoreProxy, mLog));

    // Настраиваем параметры безопасности и функциональности WebEngine
    auto *settings = mWebView->settings();
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    // Note: JavascriptCanCloseWindows and DeveloperExtrasEnabled may not be available in Qt 5.15
    // settings->setAttribute(QWebEngineSettings::JavascriptCanCloseWindows, false);
    // settings->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled,
    //                      aInfo.parameters.value(CWebGraphicsItem::DebugKey).toString() == "true");
    mWebView->setWindowFlags(Qt::FramelessWindowHint);

    // Добавляем горячую клавишу F5 для обновления страницы
    QKeySequence keysRefresh(QKeySequence::Refresh);
    QAction *actionRefresh = new QAction(this);
    actionRefresh->setShortcut(keysRefresh);
    mWebView->addAction(actionRefresh);
    connect(actionRefresh, &QAction::triggered, this, &WebGraphicsItem::onLoadFinished);

    // Скрываем контекстное меню 'Обновить'
    mWebView->page()->action(QWebEnginePage::Reload)->setVisible(false);

    // Подключаем сигналы WebEngine для обработки загрузки и JavaScript взаимодействия
    connect(mWebView.get(), &QWebEngineView::loadFinished, this, &WebGraphicsItem::onLoadFinished);
    connect(mWebView.get(), &QWebEngineView::urlChanged, this, &WebGraphicsItem::onUrlChanged);

    // Получаем разрешение из конфига виджета (секция [web]).
    if (!aInfo.parameters.contains(CWebGraphicsItem::WidthKey) ||
        !aInfo.parameters.contains(CWebGraphicsItem::HeightKey)) {
        LOG(mLog, LogLevel::Error, "Widget dimensions (width or height) missing.");
        return;
    }

    mWebView->setGeometry(QRect(0, 0, aInfo.parameters[CWebGraphicsItem::WidthKey].toInt(),
                                aInfo.parameters[CWebGraphicsItem::HeightKey].toInt()));

    // Анализируем и загружаем контент
    QString path = aInfo.parameters.value(CWebGraphicsItem::StartPageKey, "");
    if (path.startsWith("http")) {
        // Загружаем удаленный адрес
        mWebView->load(QUrl(path));
    } else {
        // Загружаем локальный контент
        path = aInfo.directory + "/" + path;
        QFile content(path);

        if (!content.open(QIODevice::ReadOnly | QIODevice::Text)) {
            LOG(mLog, LogLevel::Error, QString("Failed to load html content file '%1'.").arg(path));
            mError = QString("Failed to load content file: %1").arg(path);
            return;
        }

        QTextStream stream(&content);
        QString htmlContent = stream.readAll();
        QUrl baseUrl = QUrl::fromLocalFile(aInfo.directory + "/");
        mWebView->setHtml(htmlContent, baseUrl);
    }
    mUrl = path;
}

//---------------------------------------------------------------------------
// Деструктор. Освобождает ресурсы WebEngine виджета.
WebGraphicsItem::~WebGraphicsItem() {
    // WebEngine resources are automatically cleaned up by QSharedPointer
}

//------------------------------------------------------------------------------
void WebGraphicsItem::onRefresh() {
    mWebView->load(QUrl(mUrl));
}

//------------------------------------------------------------------------------

/// Обработчик завершения загрузки страницы WebEngine.
/// Настраивает JavaScript взаимодействие и инициализирует обработчики.
/// @param aOk true если страница загружена успешно
void WebGraphicsItem::onLoadFinished(bool aOk) {
    if (aOk) {
        mItemLoaded = true;

        // Настраиваем JavaScript объекты для взаимодействия с платежной системой
        // В WebEngine JavaScript объекты добавляются через runJavaScript
        QString jsSetup = QString("if (typeof %1 === 'undefined') { %1 = {}; }"
                                  "console.log('WebEngine container initialized');")
                              .arg(CWebGraphicsItem::ContainerScriptObject);

        mWebView->page()->runJavaScript(jsSetup);

        // Инициализируем JavaScript обработчики если они существуют
        mWebView->page()->runJavaScript(
            QString("if (typeof initialize === 'function') { "
                    "    try { initialize(); console.log('JavaScript initialized successfully'); } "
                    "    catch(e) { console.error('Initialize error:', e); } "
                    "} else { "
                    "    console.warn('No initialize function found in page'); "
                    "}"));

        // Обрабатываем отложенные сигналы
        while (!mSignalQueue.isEmpty()) {
            auto signalData = mSignalQueue.takeFirst();
            QString signalName = signalData.first;
            QList<QVariant> args = signalData.second;

            // Вызываем JavaScript обработчики для сигналов
            if (signalName == "onShow") {
                mWebView->page()->runJavaScript("if (typeof onShow === 'function') { onShow(); }");
            } else if (signalName == "onHide") {
                mWebView->page()->runJavaScript("if (typeof onHide === 'function') { onHide(); }");
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
                    mWebView->page()->runJavaScript(
                        QString("if (typeof onReset === 'function') { onReset(%1); }").arg(paramsJson));
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
                    mWebView->page()->runJavaScript(
                        QString("if (typeof onNotify === 'function') { onNotify('%1', %2); }").arg(reason, paramsJson));
                }
            }
        }

        LOG(mLog, LogLevel::Normal, "WebEngine page loaded and JavaScript initialized successfully");
    } else {
        mError = "Failed to load web page";
        LOG(mLog, LogLevel::Error, mError);
    }
}

//------------------------------------------------------------------------------
/// Обработчик изменения URL страницы.
/// @param aUrl Новый URL страницы
void WebGraphicsItem::onUrlChanged(const QUrl &aUrl) {
    mUrl = aUrl.toString();
    LOG(mLog, LogLevel::Normal, QString("WebEngine URL changed to: %1").arg(mUrl));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::show() {
    mItemLoaded ? emit onShow() : mSignalQueue.push_back(qMakePair(QString("onShow"), QList<QVariant>()));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::hide() {
    mItemLoaded ? emit onHide() : mSignalQueue.push_back(qMakePair(QString("onHide"), QList<QVariant>()));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::reset(const QVariantMap &aParameters) {
    mItemLoaded ? emit onReset(aParameters)
                : mSignalQueue.push_back(qMakePair(QString("onReset"), QList<QVariant>() << aParameters));
}

//---------------------------------------------------------------------------
void WebGraphicsItem::notify(const QString &aReason, const QVariantMap &aParameters) {
    mItemLoaded ? emit onNotify(aReason, aParameters)
                : mSignalQueue.push_back(qMakePair(QString("onNotify"), QList<QVariant>() << aReason << aParameters));
}

//---------------------------------------------------------------------------
QQuickItem *WebGraphicsItem::getWidget() const {
    // FIXME !!!
    return nullptr; // mWebView.data();
}

//---------------------------------------------------------------------------
QWidget *WebGraphicsItem::getNativeWidget() const {
    return mWebView.data();
}

//---------------------------------------------------------------------------
bool WebGraphicsItem::isValid() const {
    return !mWebView.isNull();
}

//---------------------------------------------------------------------------
QString WebGraphicsItem::getError() const {
    return mError;
}

//---------------------------------------------------------------------------
QVariantMap WebGraphicsItem::getContext() const {
    return mContext;
}