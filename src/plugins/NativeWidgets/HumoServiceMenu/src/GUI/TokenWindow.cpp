/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

// boost

#include "TokenWindow.h"

#include <QtConcurrent/QtConcurrentRun>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <boost/bind/bind.hpp>

#include "Backend/HumoServiceBackend.h"
#include "Backend/KeysManager.h"

TokenWindow::TokenWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), m_Backend(aBackend) {
    setupUi(this);

    connect(btnFormat, SIGNAL(clicked()), SLOT(onFormatButtonClicked()));

    connect(&m_FormatTaskWatcher, SIGNAL(finished()), SLOT(onFormatTaskFinished()));
}

//------------------------------------------------------------------------
TokenWindow::~TokenWindow() {
    if (m_FormatTaskWatcher.isRunning()) {
        m_FormatTaskWatcher.waitForFinished();
    }
}

//------------------------------------------------------------------------
void TokenWindow::initialize(const CCrypt::TokenStatus &aStatus) {
    updateUI(aStatus);
}

//------------------------------------------------------------------------
void TokenWindow::doFormat() {
    m_FormatTaskWatcher.setFuture(
        QtConcurrent::run([this]() { return m_Backend->getKeysManager()->formatToken(); }));
}

//------------------------------------------------------------------------
void TokenWindow::onFormatButtonClicked() {
    m_TaskParameters.clear();

    emit beginFormat();
}

//------------------------------------------------------------------------
void TokenWindow::onFormatTaskFinished() {
    if (m_FormatTaskWatcher.result()) {
        updateUI(m_Backend->getKeysManager()->tokenStatus());

        emit endFormat();
    } else {
        emit error(tr("#error_format_token"));
    }
}

//------------------------------------------------------------------------
void TokenWindow::updateUI(const CCrypt::TokenStatus &aStatus) {
    labelName->setText(aStatus.name.isEmpty() ? tr("#empty") : aStatus.name);

    QString status;

    if (aStatus.available && aStatus.initialized) {
        status = tr("#ok");
        btnFormat->setEnabled(false);
    } else if (aStatus.available) {
        status = tr("#not_initialised");
        btnFormat->setEnabled(true);
    } else {
        status = tr("#none");
        btnFormat->setEnabled(false);
    }

    labelStatus->setText(status);
}

//------------------------------------------------------------------------
