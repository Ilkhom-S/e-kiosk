/* @file Всплывающие окна (модальные и не модальные) */

#include "MessageBox.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWidget>

namespace GUI {

//------------------------------------------------------------------------
MessageBox *MessageBox::m_Instance = nullptr;

//------------------------------------------------------------------------
MessageBox::MessageBox() : m_SignalReceiver(nullptr) {
    connect(&m_WaitTimer, SIGNAL(timeout()), this, SLOT(hideWindow()));
}

//------------------------------------------------------------------------
void MessageBox::initialize() {
    delete m_Instance;
    m_Instance = new MessageBox();
}

//------------------------------------------------------------------------
void MessageBox::shutdown() {
    m_Instance->hideWindow();
    delete m_Instance;
    m_Instance = nullptr;
}

//------------------------------------------------------------------------
void MessageBox::info(const QString &aText) {
    getInstance()->showPopup(
        aText, SDK::GUI::MessageBoxParams::Info, SDK::GUI::MessageBoxParams::OK);
}

//------------------------------------------------------------------------
void MessageBox::critical(const QString &aText) {
    getInstance()->showPopup(
        aText, SDK::GUI::MessageBoxParams::Critical, SDK::GUI::MessageBoxParams::OK);
}

//------------------------------------------------------------------------
void MessageBox::warning(const QString &aText) {
    getInstance()->showPopup(
        aText, SDK::GUI::MessageBoxParams::Warning, SDK::GUI::MessageBoxParams::OK);
}

//------------------------------------------------------------------------
void MessageBox::wait(const QString &aText, bool aCancelable) {
    SDK::GUI::MessageBoxParams::Enum button;
    button =
        aCancelable ? SDK::GUI::MessageBoxParams::Cancel : SDK::GUI::MessageBoxParams::NoButton;
    getInstance()->showPopup(aText, SDK::GUI::MessageBoxParams::Wait, button);
}

//------------------------------------------------------------------------
void MessageBox::notify(const QString &aText, int aTimeout) {
    getInstance()->showNotify(aText, aTimeout);
}

//------------------------------------------------------------------------
int MessageBox::question(const QString &aText) {
    return getInstance()->showPopup(
        aText, SDK::GUI::MessageBoxParams::Question, SDK::GUI::MessageBoxParams::OK);
}

//------------------------------------------------------------------------
void MessageBox::update(const QVariantMap &aParameters) {
    getInstance()->updatePopup(aParameters);
}

//------------------------------------------------------------------------
void MessageBox::hide(bool aWaiting) {
    if (aWaiting) {
        getInstance()->startWaitTimer();
    } else {
        getInstance()->hideWindow();
    }
}

//------------------------------------------------------------------------
void MessageBox::subscribe(QObject *aReceiver) {
    getInstance()->setReceiver(aReceiver);
}

//------------------------------------------------------------------------
void MessageBox::emitSignal(const QVariantMap &aParameters) {
    getInstance()->emitPopupSignal(aParameters);
}

//------------------------------------------------------------------------
void MessageBox::setParentWidget(QWidget *aParent) {
    getInstance()->updateParentWidget(aParent);
}

//------------------------------------------------------------------------
int MessageBox::showPopup(const QString &aText,
                          SDK::GUI::MessageBoxParams::Enum aIcon,
                          SDK::GUI::MessageBoxParams::Enum aButton) {
    // Показываем затемняющий overlay поверх родительского окна
    if (m_ParentWidget) {
        if (m_Overlay.isNull()) {
            m_Overlay = QPointer<QWidget>(new QWidget(m_ParentWidget));
            m_Overlay->setStyleSheet(QStringLiteral("background-color: rgba(0, 0, 0, 168);"));
            m_Overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
        m_Overlay->setGeometry(m_ParentWidget->rect());
        m_Overlay->show();
        m_Overlay->raise();
    }

    m_Window->setup(aText, aIcon, aButton);

    // Центрируем диалог над родительским окном
    if (m_ParentWidget) {
        m_Window->adjustSize();
        const QPoint center = m_ParentWidget->mapToGlobal(m_ParentWidget->rect().center());
        m_Window->move(center - QPoint(m_Window->width() / 2, m_Window->height() / 2));
    }

    if (aIcon == SDK::GUI::MessageBoxParams::Question) {
        const int result = m_Window->exec();
        if (m_Overlay) {
            m_Overlay->hide();
        }
        // Восстанавливаем родительское окно после закрытия модального диалога
        if (m_ParentWidget) {
            m_ParentWidget->raise();
            m_ParentWidget->activateWindow();
        }
        return result;
    }
    m_Window->show();

    return 0;
}

//------------------------------------------------------------------------
void MessageBox::showNotify(const QString &aText, int aTimeout) {
    showPopup(aText, SDK::GUI::MessageBoxParams::NoIcon, SDK::GUI::MessageBoxParams::NoButton);
    QTimer::singleShot(aTimeout, this, SLOT(hideWindow()));
}

//------------------------------------------------------------------------
void MessageBox::updatePopup(const QVariantMap &aParameters) {
    showPopup(aParameters[SDK::GUI::CMessageBox::TextMessage].toString(),
              aParameters[SDK::GUI::CMessageBox::Icon].value<SDK::GUI::MessageBoxParams::Enum>(),
              aParameters[SDK::GUI::CMessageBox::Button].value<SDK::GUI::MessageBoxParams::Enum>());
}

//------------------------------------------------------------------------
void MessageBox::setReceiver(QObject *aReceiver) {
    m_SignalReceiver = aReceiver;
}

//------------------------------------------------------------------------
void MessageBox::emitPopupSignal(const QVariantMap &aParameters) {
    if (m_SignalReceiver) {
        QObject::connect(this,
                         SIGNAL(clicked(const QVariantMap &)),
                         m_SignalReceiver,
                         SLOT(onClicked(const QVariantMap &)),
                         Qt::UniqueConnection);
        emit clicked(aParameters);
    }
}

//------------------------------------------------------------------------
void MessageBox::updateParentWidget(QWidget *aParent) {
    m_ParentWidget = aParent;
    if (m_Window.isNull()) {
        m_Window = QPointer<MessageWindow>(new MessageWindow(aParent));
    }
}

//------------------------------------------------------------------------
void MessageBox::hideWindow() {
    m_WaitTimer.stop();

    if (m_SignalReceiver) {
        QObject::disconnect(this,
                            SIGNAL(clicked(const QVariantMap &)),
                            m_SignalReceiver,
                            SLOT(onClicked(const QVariantMap &)));
        m_SignalReceiver = nullptr;
    }

    if (!m_WaitTimer.isActive()) {
        m_Window->hide();
        if (m_Overlay) {
            m_Overlay->hide();
        }
    }
}
} // namespace GUI
