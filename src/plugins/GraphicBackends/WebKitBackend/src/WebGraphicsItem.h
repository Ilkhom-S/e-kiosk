/* @file Графический объект. */

#pragma once

#include <QtCore/QSharedPointer>
#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QGraphicsWebView>

#include <Common/ILog.h>

// TODO Убрать зависимость
// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>

// GUI SDK
#include <SDK/GUI/GraphicsItem_Info.h>
#include <SDK/GUI/IGraphicsItem.h>

//---------------------------------------------------------------------------
/// Интерфейс для созданного движком графического объекта.
class WebGraphicsItem : public QObject, public SDK::GUI::IGraphicsItem, protected ILogable {
    Q_OBJECT

public:
    WebGraphicsItem(const SDK::GUI::GraphicsItem_Info &aInfo,
                    SDK::PaymentProcessor::Scripting::Core *aCore,
                    ILog *m_Log);

    /// Вызывается перед отображением виджета.
    virtual void show();

    /// Вызывается для сброса/настройки виджета.
    virtual void reset(const QVariantMap &aParameters);

    /// Вызывается перед сокрытием виджета.
    virtual void hide();

    /// Посылает уведомление виджету.
    virtual void notify(const QString &aReason, const QVariantMap &aParameters);

    /// Проверяет готов ли виджет.
    virtual bool isValid() const;

    /// Возвращает описание ошибки.
    virtual QString getError() const;

    /// Возвращает виджет.
    virtual QQuickItem *getWidget() const;

    virtual QWidget *getNativeWidget() const { return nullptr; }

    /// Возвращает контекст виджета.
    virtual QVariantMap getContext() const;

signals:
    /// Сигналы для проброса в JavaScript.
    void onShow();
    void onReset(const QVariantMap &aParameters);
    void onHide();
    void onNotify(const QString &aReason, const QVariantMap &aParameters);

private slots:
    void onJavaScriptWindowObjectCleared();
    void onFrameLoaded(bool aOk);
    void onRefresh();

private:
    ILog *m_Log;
    QString m_Url;
    QString m_Error;
    QSharedPointer<QGraphicsWebView> m_WebView;
    SDK::PaymentProcessor::Scripting::Core *m_CoreProxy;
    SDK::PaymentProcessor::EEventType m_EventTypeMetaInfo;
    bool m_Item_Loaded;
    QVariantMap m_Context;
    QList<QPair<QString, QList<QVariant>>> m_SignalQueue;
};

//---------------------------------------------------------------------------
