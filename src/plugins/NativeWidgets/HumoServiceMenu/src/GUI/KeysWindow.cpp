/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

// boost

#include "KeysWindow.h"

#include <QtConcurrent/QtConcurrentRun>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>

#include "Backend/HumoServiceBackend.h"
#include "Backend/KeysManager.h"
#include "Backend/MessageBox.h"
#include "SIPStyle.h"
#include "ServiceTags.h"

KeysWindow::KeysWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), m_Backend(aBackend) {
    setupUi(this);

    foreach (QLineEdit *le, findChildren<QLineEdit *>()) {
        le->setStyle(new SIPStyle);
    }

    connect(btnCreate, SIGNAL(clicked()), SLOT(onCreateButtonClicked()));
    connect(btnRepeat, SIGNAL(clicked()), SLOT(onRepeatButtonClicked()));
    connect(cbKeypairChange, SIGNAL(stateChanged(int)), this, SLOT(onCheckedKeyPairChanged(int)));
    connect(&m_GenerateTaskWatcher, SIGNAL(finished()), SLOT(onGenerateTaskFinished()));

    cbKeypairChange->setEnabled(m_Backend->getKeysManager()->allowAnyKeyPair());
}

//------------------------------------------------------------------------
KeysWindow::~KeysWindow() {
    if (m_GenerateTaskWatcher.isRunning()) {
        m_GenerateTaskWatcher.waitForFinished();
    }
}

//------------------------------------------------------------------------
void KeysWindow::initialize(bool aHasRuToken, bool aRutokenOK) {
    swPages->setCurrentWidget(wGeneratePage);

    foreach (QLineEdit *le, findChildren<QLineEdit *>()) {
        le->setEnabled(!aHasRuToken || (aHasRuToken && aRutokenOK));
    }

    foreach (QPushButton *pb, findChildren<QPushButton *>()) {
        pb->setEnabled(!aHasRuToken || (aHasRuToken && aRutokenOK));
    }

    cbKeypairChange->setChecked(Qt::Unchecked);
    frameKeyPair->setEnabled(false);
}

//------------------------------------------------------------------------
bool KeysWindow::save() {
    if (m_GenerateTaskWatcher.isRunning()) {
        return false;
    }

    return m_Backend->getKeysManager()->saveKey();
}

//------------------------------------------------------------------------
void KeysWindow::doGenerate() {
    m_GenerateTaskWatcher.setFuture(QtConcurrent::run(boost::bind(
        &KeysManager::generateKey, m_Backend->getKeysManager(), boost::ref(m_TaskParameters))));
}

//------------------------------------------------------------------------
void KeysWindow::onCreateButtonClicked() {
    SetStyleSheet(login,
                  login->text().isEmpty() ? CKeysWindow::WarningStyleSheet
                                          : CKeysWindow::DefaultStyleSheet);
    SetStyleSheet(password,
                  password->text().isEmpty() ? CKeysWindow::WarningStyleSheet
                                             : CKeysWindow::DefaultStyleSheet);

    if (login->text().isEmpty() || password->text().isEmpty()) {
        return;
    }

    QString keyPair = sbKeypairNumber->text();
    bool rewriteExistNumber = true;

    if (cbKeypairChange->checkState() == Qt::Checked && !keyPair.isEmpty()) {
        int num = keyPair.toInt();

        if (m_Backend->getKeysManager()->getLoadedKeys().contains(num)) {
            rewriteExistNumber = MessageBox::question(tr("#keypair_already_exist"));
        }
    } else {
        keyPair = "0";
    }

    if (!rewriteExistNumber) {
        return;
    }

    m_TaskParameters.clear();

    m_TaskParameters[CServiceTags::Login] = login->text();
    m_TaskParameters[CServiceTags::Password] = password->text();
    m_TaskParameters[CServiceTags::KeyPairNumber] = keyPair;
    m_TaskParameters[CServiceTags::KeyPairDescription] = description->text();

    emit beginGenerating();
}

//------------------------------------------------------------------------
void KeysWindow::onRepeatButtonClicked() {
    login->clear();
    password->clear();

    swPages->setCurrentWidget(wGeneratePage);
}

//------------------------------------------------------------------------
void KeysWindow::onCheckedKeyPairChanged(int aState) {
    frameKeyPair->setEnabled(aState > 0);
}

//------------------------------------------------------------------------
void KeysWindow::onGenerateTaskFinished() {
    if (m_GenerateTaskWatcher.result()) {
        lbAp->setText(m_Backend->getKeysManager()->getAP());
        lbSd->setText(m_Backend->getKeysManager()->getSD());
        lbOp->setText(m_Backend->getKeysManager()->getOP());

        swPages->setCurrentWidget(wResultsPage);

        emit endGenerating();
    } else {
        emit error(m_TaskParameters[CServiceTags::Error].toString());
    }
}

//------------------------------------------------------------------------
