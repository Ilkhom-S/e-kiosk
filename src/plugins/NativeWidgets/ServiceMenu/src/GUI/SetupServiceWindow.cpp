/* @file Окно настроек. */

#include "SetupServiceWindow.h"

#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "DispenserServiceWindow.h"
#include "HardwareServiceWindow.h"
#include "KeysServiceWindow.h"
#include "NetworkServiceWindow.h"
#include "TokenServiceWindow.h"

SetupServiceWindow::SetupServiceWindow(ServiceMenuBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend), m_CurrentPageIndex(-1) {
    setupUi(this);
    connect(twPages, SIGNAL(currentChanged(int)), SLOT(onCurrentPageChanged(int)));
}

//------------------------------------------------------------------------
void SetupServiceWindow::onCurrentPageChanged(int aIndex) {
    auto *prev = dynamic_cast<IServiceWindow *>(twPages->widget(m_CurrentPageIndex));

    if (prev) {
        if (!prev->deactivate()) {
            // Окно не может быть сейчас закрыто.
            twPages->blockSignals(true);
            twPages->setCurrentIndex(m_CurrentPageIndex);
            twPages->blockSignals(false);

            return;
        }
    }

    auto *next = dynamic_cast<IServiceWindow *>(twPages->widget(aIndex));

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
    IServiceWindow *page = nullptr;

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
    auto *page = dynamic_cast<IServiceWindow *>(twPages->widget(m_CurrentPageIndex));

    if (page) {
        return page->deactivate();
    }

    return true;
}

//------------------------------------------------------------------------
bool SetupServiceWindow::initialize() {
    twPages->blockSignals(true);
    twPages->clear();

    ServiceMenuBackend::TAccessRights rights = m_Backend->getAccessRights();

    if (rights.contains(ServiceMenuBackend::SetupHardware) || !m_Backend->hasAnyPassword()) {
        auto *hardwareWindow = new HardwareServiceWindow(m_Backend, this);
        hardwareWindow->initialize();
        twPages->addTab(hardwareWindow, tr("#hardware"));
    }

    if (rights.contains(ServiceMenuBackend::SetupNetwork) || !m_Backend->hasAnyPassword()) {
        auto *networkWindow = new NetworkServiceWindow(m_Backend, this);
        networkWindow->initialize();
        twPages->addTab(networkWindow, tr("#network"));
    }

#ifdef TC_USE_TOKEN
    if (rights.contains(ServiceMenuBackend::SetupKeys) || !m_Backend->hasAnyPassword()) {
        TokenServiceWindow *tokenWindow = new TokenServiceWindow(m_Backend, this);
        tokenWindow->initialize();
        twPages->addTab(tokenWindow, tr("#token"));
    }
#endif

    if (rights.contains(ServiceMenuBackend::SetupKeys) || !m_Backend->hasAnyPassword()) {
        auto *keysWindow = new KeysServiceWindow(m_Backend, this);
        keysWindow->initialize();
        twPages->addTab(keysWindow, tr("#keys"));
    }

    if (rights.contains(ServiceMenuBackend::Encash)) {
        auto *dispenserWindow = new DispenserServiceWindow(m_Backend, this);
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
