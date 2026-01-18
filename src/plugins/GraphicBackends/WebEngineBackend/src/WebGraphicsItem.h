/* @file Графический объект WebEngine для отображения веб-контента. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <QtWebEngineWidgets/QWebEngineView>
#include <Common/QtHeadersEnd.h>

#include <Common/ILog.h>

// TODO Убрать зависимость
// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

// GUI SDK
#include <SDK/GUI/GraphicsItemInfo.h>
#include <SDK/GUI/IGraphicsItem.h>

//---------------------------------------------------------------------------
/// Графический элемент для отображения веб-контента с использованием Qt WebEngine.
/// Предоставляет интерфейс для создания и управления веб-виджетами в графической системе EKiosk.
/// Поддерживает JavaScript взаимодействие, локальное хранилище и интеграцию с платежной системой.
class WebGraphicsItem : public QObject, public SDK::GUI::IGraphicsItem, protected ILogable {
    Q_OBJECT

  public:
    /// Конструктор графического элемента WebEngine.
    /// @param aInfo Информация о графическом элементе (размеры, параметры, контекст)
    /// @param aCore Указатель на ядро скриптинга платежной системы для JavaScript взаимодействия
    /// @param aLog Указатель на логгер для записи диагностической информации
    WebGraphicsItem(const SDK::GUI::GraphicsItemInfo &aInfo, SDK::PaymentProcessor::Scripting::Core *aCore, ILog *aLog);

    /// Деструктор. Освобождает ресурсы WebEngine виджета.
    ~WebGraphicsItem();

    /// Вызывается перед отображением виджета.
    /// Инициализирует WebEngine виджет и загружает начальную страницу.
    virtual void show();

    /// Вызывается для сброса/настройки виджета.
    /// @param aParameters Параметры конфигурации (URL, размеры, настройки отладки)
    virtual void reset(const QVariantMap &aParameters);

    /// Вызывается перед сокрытием виджета.
    /// Останавливает загрузку и скрывает WebEngine виджет.
    virtual void hide();

    /// Посылает уведомление виджету.
    /// @param aReason Тип уведомления (например, "payment_success")
    /// @param aParameters Дополнительные параметры уведомления
    virtual void notify(const QString &aReason, const QVariantMap &aParameters);

    /// Проверяет готов ли виджет к использованию.
    /// @return true если виджет инициализирован и готов к работе
    virtual bool isValid() const;

    /// Возвращает описание ошибки если виджет не готов.
    /// @return Строка с описанием ошибки или пустая строка
    virtual QString getError() const;

    /// Возвращает QML виджет для интеграции с Qt Quick.
    /// @return Указатель на QQuickItem или nullptr для виджет-based реализации
    virtual QQuickItem *getWidget() const;

    /// Возвращает нативный QWidget для интеграции с Qt Widgets.
    /// @return Указатель на QWidget с WebEngine представлением
    virtual QWidget *getNativeWidget() const;

    /// Возвращает контекст виджета для сохранения состояния.
    /// @return QVariantMap с текущими параметрами и состоянием
    virtual QVariantMap getContext() const;

  signals:
    /// Сигналы для проброса в JavaScript обработчики.
    void onShow();
    void onReset(const QVariantMap &aParameters);
    void onHide();
    void onNotify(const QString &aReason, const QVariantMap &aParameters);

  private slots:
    /// Обработчик события завершения загрузки страницы.
    /// @param aOk true если страница загружена успешно
    void onLoadFinished(bool aOk);

    /// Обработчик события изменения URL.
    /// @param aUrl Новый URL страницы
    void onUrlChanged(const QUrl &aUrl);

    /// Обработчик обновления страницы (F5).
    void onRefresh();

  private:
    ILog *mLog;                                          ///< Логгер для диагностики
    QString mUrl;                                        ///< Текущий URL страницы
    QString mError;                                      ///< Описание последней ошибки
    QSharedPointer<QWebEngineView> mWebView;             ///< WebEngine виджет для отображения веб-контента
    SDK::PaymentProcessor::Scripting::Core *mCoreProxy;  ///< Прокси для взаимодействия с платежной системой
    bool mItemLoaded;                                    ///< Флаг готовности виджета
    QVariantMap mContext;                                ///< Контекст виджета для сохранения состояния
    QList<QPair<QString, QList<QVariant>>> mSignalQueue; ///< Очередь сигналов для отложенной обработки
};
