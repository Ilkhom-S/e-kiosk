/* @file Окно настроек. */

#include "KeysServiceWindow.h"

#include <QtCore/QTime>

#include <Common/BasicApplication.h>

#include <SDK/PaymentProcessor/Core/ICore.h>

#include "Backend/HumoServiceBackend.h"
#include "Backend/KeysManager.h"
#include "MessageBox/MessageBox.h"

KeysServiceWindow::KeysServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend), m_Window(new KeysWindow(aBackend, this)) {
    setupUi(this);

    connect(m_Window, SIGNAL(beginGenerating()), SLOT(onBeginGenerating()));
    connect(m_Window, SIGNAL(endGenerating()), SLOT(onEndGenerating()));
    connect(m_Window, SIGNAL(error(QString)), SLOT(onError(QString)));

    m_Window->setParent(this);
    wContainer->setLayout(new QHBoxLayout);
    wContainer->layout()->setSpacing(0);
    wContainer->layout()->setContentsMargins(0, 0, 0, 0);
    wContainer->layout()->addWidget(m_Window);
}

//------------------------------------------------------------------------
bool KeysServiceWindow::activate() {
    auto tokenStatus = m_Backend->getKeysManager()->tokenStatus();

    m_Window->initialize(tokenStatus.available, tokenStatus.initialized);

    return true;
}

//------------------------------------------------------------------------
bool KeysServiceWindow::deactivate() {
    return true;
}

//------------------------------------------------------------------------
bool KeysServiceWindow::initialize() {
    auto tokenStatus = m_Backend->getKeysManager()->tokenStatus();

    m_Window->initialize(tokenStatus.available, tokenStatus.initialized);

    return true;
}

//------------------------------------------------------------------------
bool KeysServiceWindow::shutdown() {
    return true;
}

//------------------------------------------------------------------------
void KeysServiceWindow::onBeginGenerating() {
    if (GUI::MessageBox::question(tr("#question_generate_keys_warning")) != 0) {
        GUI::MessageBox::hide();
        GUI::MessageBox::wait(tr("#creating_keys"));

        m_Window->doGenerate();
    }
}

//------------------------------------------------------------------------
void KeysServiceWindow::onEndGenerating() {
    GUI::MessageBox::hide();

    QString generateResult;
    generateResult = "\n" + tr("#sd") + " " + m_Backend->getKeysManager()->getSD() + "\n";
    generateResult += tr("#ap") + " " + m_Backend->getKeysManager()->getAP() + "\n";
    generateResult += tr("#op") + " " + m_Backend->getKeysManager()->getOP();

    if (GUI::MessageBox::question(tr("#question_save_and_register_keys") + generateResult) != 0) {
        if (m_Window->save()) {
            m_Backend->saveConfiguration();

            if (m_Backend->getKeysManager()->isDefaultKeyOP(m_Backend->getKeysManager()->getOP())) {
                if (GUI::MessageBox::question(tr("#question_need_new_config")) != 0) {
                    m_Backend->needUpdateConfigs();
                }
            }

            QVariantMap params;
            params["signal"] = "close";
            m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, params);
            m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::CloseApplication);
        }
    } else {
        auto tokenStatus = m_Backend->getKeysManager()->tokenStatus();

        m_Window->initialize(tokenStatus.available, tokenStatus.initialized);
    }
}

//------------------------------------------------------------------------
void KeysServiceWindow::onError(QString aError) {
    GUI::MessageBox::hide();
    GUI::MessageBox::critical(aError);
}

//------------------------------------------------------------------------
