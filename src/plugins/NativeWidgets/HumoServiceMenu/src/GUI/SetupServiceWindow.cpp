/* @file Окно настроек. */

#include "SetupServiceWindow.h"

#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

#include "Backend/HumoServiceBackend.h"
#include "Backend/KeysManager.h"
#include "DispenserServiceWindow.h"
#include "HardwareServiceWindow.h"
#include "KeysServiceWindow.h"
#include "NetworkServiceWindow.h"
#include "TokenServiceWindow.h"

SetupServiceWindow::SetupServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend), m_CurrentPageIndex(-1) {
    setupUi(this);
    connect(twPages, SIGNAL(currentChanged(int)), SLOT(onCurrentPageChanged(int)));
}

//------------------------------------------------------------------------
void SetupServiceWindow::onCurrentPageChanged(int aIndex) {
    IServiceWindow *prev = dynamic_cast<IServiceWindow *>(twPages->widget(m_CurrentPageIndex));

    if (prev) {
        if (!prev->deactivate()) {
            // Окно не может быть сейчас закрыто.
            twPages->blockSignals(true);
            twPages->setCurrentIndex(m_CurrentPageIndex);
            twPages->blockSignals(false);

            return;
        }
    }

    IServiceWindow *next = dynamic_cast<IServiceWindow *>(twPages->widget(aIndex));

    if (next) {
        next->activate();
    }

    m_CurrentPageIndex = aIndex;

    QWidget *currentPage = twPages->widget(m_CurrentPageIndex);
    if (currentPage) {
        m_Backend->toLog(QString("Page activated: %1.").arg(currentPage->objectName()));
    }
}

//------------------------------------------------------------------------
bool SetupServiceWindow::activate() {
    IServiceWindow *page = 0;

    if (m_CurrentPageIndex == -1) {
        if (twPages->count()) {
            m_CurrentPageIndex = 0;
        }
    }

    page = dynamic_cast<IServiceWindow *>(twPages->widget(m_CurrentPageIndex));

    if (page) {
        page->activate();
    }

    return true;
}

//------------------------------------------------------------------------
bool SetupServiceWindow::deactivate() {
    IServiceWindow *page = dynamic_cast<IServiceWindow *>(twPages->widget(m_CurrentPageIndex));

    if (page) {
        return page->deactivate();
    }

    return true;
}

//------------------------------------------------------------------------
bool SetupServiceWindow::initialize() {
    twPages->blockSignals(true);
    twPages->clear();

    HumoServiceBackend::TAccessRights rights = m_Backend->getAccessRights();

    if (rights.contains(HumoServiceBackend::SetupHardware) || !m_Backend->hasAnyPassword()) {
        HardwareServiceWindow *hardwareWindow = new HardwareServiceWindow(m_Backend, this);
        hardwareWindow->initialize();
        twPages->addTab(hardwareWindow, tr("#hardware"));
    }

    if (rights.contains(HumoServiceBackend::SetupNetwork) || !m_Backend->hasAnyPassword()) {
        NetworkServiceWindow *networkWindow = new NetworkServiceWindow(m_Backend, this);
        networkWindow->initialize();
        twPages->addTab(networkWindow, tr("#network"));
    }

#ifdef TC_USE_TOKEN
    if (rights.contains(HumoServiceBackend::SetupKeys) || !m_Backend->hasAnyPassword()) {
        TokenServiceWindow *tokenWindow = new TokenServiceWindow(m_Backend, this);
        tokenWindow->initialize();
        twPages->addTab(tokenWindow, tr("#token"));
    }
#endif

    if (rights.contains(HumoServiceBackend::SetupKeys) || !m_Backend->hasAnyPassword()) {
        KeysServiceWindow *keysWindow = new KeysServiceWindow(m_Backend, this);
        keysWindow->initialize();
        twPages->addTab(keysWindow, tr("#keys"));
    }

    if (rights.contains(HumoServiceBackend::Encash)) {
        DispenserServiceWindow *dispenserWindow = new DispenserServiceWindow(m_Backend, this);
        dispenserWindow->initialize();
        twPages->addTab(dispenserWindow, tr("#dispenser"));
    }

    foreach (QWidget *widget, m_Backend->getExternalWidgets()) {
        twPages->addTab(widget, widget->property("widget_name").toString());
    }

    twPages->blockSignals(false);

    return true;
}

//------------------------------------------------------------------------
bool SetupServiceWindow::shutdown() {
    foreach (QWidget *widget, m_Backend->getExternalWidgets(false)) {
        twPages->removeTab(twPages->indexOf(widget));
        widget->setParent(nullptr);
    }

    return true;
}

//------------------------------------------------------------------------
