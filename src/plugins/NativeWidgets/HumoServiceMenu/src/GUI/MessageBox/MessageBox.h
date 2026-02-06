/* @file Всплывающие окна (модальные и не модальные) */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>

#include <SDK/GUI/MessageBoxParams.h>

#include "MessageWindow.h"

//------------------------------------------------------------------------
namespace CMessageBox {
const int WaitWindowTimeout = 1000 * 3; // 3 секунды
} // namespace CMessageBox

namespace GUI {

//------------------------------------------------------------------------
class MessageBox : public QObject {
    Q_OBJECT

public:
    static void initialize();
    static void shutdown();

public:
    static void info(const QString &aText);
    static void critical(const QString &aText);
    static void warning(const QString &aText);
    static void wait(const QString &aText, bool aCancelable = false);

    /// Окошко без кнопок. Показывается aTimeout миллисекунд
    static void notify(const QString &aText, int aTimeout = 1000);

    /// Возвращает 1, если нажали OK/Yes
    static int question(const QString &aText);

    static void hide(bool aWaiting = false);

    /// Обновить состояние виджета
    static void update(const QVariantMap &aParameters);

    /// Сохраняем подписчика на сигнал clicked()
    static void subscribe(QObject *aReceiver);

    /// Обновляем параметры сигнала/слота и испускаем сигнал
    static void emitSignal(const QVariantMap &aParameters);

    static void setParentWidget(QWidget *aParent);

public:
    int showPopup(const QString &aText,
                  SDK::GUI::MessageBoxParams::Enum aIcon,
                  SDK::GUI::MessageBoxParams::Enum aButton);
    void showNotify(const QString &aText, int aTimeout);
    void updatePopup(const QVariantMap &aParameters);
    void setReceiver(QObject *aReceiver);
    void emitPopupSignal(const QVariantMap &aParameters);
    void startWaitTimer() { mWaitTimer.start(CMessageBox::WaitWindowTimeout); }
    void stopWaitTimer() { mWaitTimer.stop(); }

    void updateParentWidget(QWidget *aParent);

public slots:
    void hideWindow();

signals:
    void clicked(const QVariantMap &aParameters);

private:
    MessageBox();
    ~MessageBox() {}

    static MessageBox *getInstance() { return mInstance; }

private:
    static MessageBox *mInstance;

private:
    QPointer<QObject> mSignalReceiver;
    QTimer mWaitTimer;
    QPointer<MessageWindow> mWindow;
    QWidget *mParentWidget;
};

} // namespace GUI

//------------------------------------------------------------------------------
