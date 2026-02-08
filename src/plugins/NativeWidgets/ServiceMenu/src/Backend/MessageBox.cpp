/* @file Всплывающие окна (модальные и не модальные) */

#include "MessageBox.h"

MessageBox *MessageBox::m_Instance = 0;

//------------------------------------------------------------------------
MessageBox::MessageBox(SDK::PaymentProcessor::IGUIService *aGUIService)
    : m_GUIService(aGUIService), m_SignalReceiver(0) {
    connect(&m_WaitTimer, SIGNAL(timeout()), this, SLOT(hideWindow()));
}

//------------------------------------------------------------------------
void MessageBox::initialize(SDK::PaymentProcessor::IGUIService *aGUIService) {
    delete m_Instance;
    m_Instance = new MessageBox(aGUIService);
}

//------------------------------------------------------------------------
void MessageBox::shutdown() {
    m_Instance->hideWindow();
    delete m_Instance;
    m_Instance = 0;
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
    button = aCancelable ? SDK::GUI::MessageBoxParams::Cancel : SDK::GUI::MessageBoxParams::OK;
    getInstance()->showPopup(aText, SDK::GUI::MessageBoxParams::Wait, button);
}

//------------------------------------------------------------------------
void MessageBox::modal(const QString &aText, SDK::GUI::MessageBoxParams::Enum aIcon) {
    getInstance()->showModal(aText, aIcon);
}

//------------------------------------------------------------------------
void MessageBox::notify(const QString &aText, int aTimeout) {
    getInstance()->showNotify(aText, aTimeout);
}

//------------------------------------------------------------------------
bool MessageBox::question(const QString &aText) {
    return getInstance()->showModal(aText, SDK::GUI::MessageBoxParams::Question);
}

//------------------------------------------------------------------------
void MessageBox::update(const QVariantMap &aParameters) {
    return getInstance()->updatePopup(aParameters);
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
void MessageBox::showPopup(const QString &aText,
                           SDK::GUI::MessageBoxParams::Enum aIcon,
                           SDK::GUI::MessageBoxParams::Enum aButton) {
    hideWindow();

    QVariantMap params;
    params[SDK::GUI::CMessageBox::TextMessage] = aText;
    params[SDK::GUI::CMessageBox::Icon] = aIcon;
    params[SDK::GUI::CMessageBox::Button] = aButton;
    params[SDK::GUI::CMessageBox::Scaled] = true;

    m_GUIService->showPopup(SDK::GUI::CMessageBox::SceneName, params);
}

//------------------------------------------------------------------------
bool MessageBox::showModal(const QString &aText, SDK::GUI::MessageBoxParams::Enum aIcon) {
    hideWindow();

    QVariantMap params;
    params[SDK::GUI::CMessageBox::TextMessage] = aText;
    params[SDK::GUI::CMessageBox::Icon] = aIcon;
    params[SDK::GUI::CMessageBox::Button] = SDK::GUI::MessageBoxParams::OK;
    params[SDK::GUI::CMessageBox::Scaled] = true;

    QVariantMap returnParams = m_GUIService->showModal(SDK::GUI::CMessageBox::SceneName, params);

    bool result =
        returnParams[SDK::GUI::CMessageBox::Button].toInt() == SDK::GUI::MessageBoxParams::OK;

    return result;
}

//------------------------------------------------------------------------
void MessageBox::showNotify(const QString &aText, int aTimeout) {
    showPopup(aText, SDK::GUI::MessageBoxParams::NoIcon, SDK::GUI::MessageBoxParams::NoButton);
    QTimer::singleShot(aTimeout, this, SLOT(hideWindow()));
}

//------------------------------------------------------------------------
void MessageBox::updatePopup(const QVariantMap &aParameters) {
    m_GUIService->notify("", aParameters);
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
void MessageBox::hideWindow() {
    m_WaitTimer.stop();

    if (m_SignalReceiver) {
        QObject::disconnect(this,
                            SIGNAL(clicked(const QVariantMap &)),
                            m_SignalReceiver,
                            SLOT(onClicked(const QVariantMap &)));
        m_SignalReceiver = 0;
    }

    if (!m_WaitTimer.isActive()) {
        m_GUIService->hidePopup();
    }
}

//------------------------------------------------------------------------
