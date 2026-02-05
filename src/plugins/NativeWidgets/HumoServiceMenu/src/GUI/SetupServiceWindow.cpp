/* @file Окно настроек. */

// SDK
#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

// System
#include "Backend/HumoServiceBackend.h"
#include "Backend/KeysManager.h"

// Project
#include "DispenserServiceWindow.h"
#include "HardwareServiceWindow.h"
#include "KeysServiceWindow.h"
#include "NetworkServiceWindow.h"
#include "SetupServiceWindow.h"
#include "TokenServiceWindow.h"

SetupServiceWindow::SetupServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend), mCurrentPageIndex(-1)
{
    setupUi(this);
    connect(twPages, SIGNAL(currentChanged(int)), SLOT(onCurrentPageChanged(int)));
}

//------------------------------------------------------------------------
void SetupServiceWindow::onCurrentPageChanged(int aIndex)
{
    IServiceWindow *prev = dynamic_cast<IServiceWindow *>(twPages->widget(mCurrentPageIndex));

    if (prev)
    {
        if (!prev->deactivate())
        {
            // Окно не может быть сейчас закрыто.
            twPages->blockSignals(true);
            twPages->setCurrentIndex(mCurrentPageIndex);
            twPages->blockSignals(false);

            return;
        }
    }

    IServiceWindow *next = dynamic_cast<IServiceWindow *>(twPages->widget(aIndex));

    if (next)
    {
        next->activate();
    }

    mCurrentPageIndex = aIndex;

    QWidget *currentPage = twPages->widget(mCurrentPageIndex);
    if (currentPage)
    {
        mBackend->toLog(QString("Page activated: %1.").arg(currentPage->objectName()));
    }
}

//------------------------------------------------------------------------
bool SetupServiceWindow::activate()
{
    IServiceWindow *page = 0;

    if (mCurrentPageIndex == -1)
    {
        if (twPages->count())
        {
            mCurrentPageIndex = 0;
        }
    }

    page = dynamic_cast<IServiceWindow *>(twPages->widget(mCurrentPageIndex));

    if (page)
    {
        page->activate();
    }

    return true;
}

//------------------------------------------------------------------------
bool SetupServiceWindow::deactivate()
{
    IServiceWindow *page = dynamic_cast<IServiceWindow *>(twPages->widget(mCurrentPageIndex));

    if (page)
    {
        return page->deactivate();
    }

    return true;
}

//------------------------------------------------------------------------
bool SetupServiceWindow::initialize()
{
    twPages->blockSignals(true);
    twPages->clear();

    HumoServiceBackend::TAccessRights rights = mBackend->getAccessRights();

    if (rights.contains(HumoServiceBackend::SetupHardware) || !mBackend->hasAnyPassword())
    {
        HardwareServiceWindow *hardwareWindow = new HardwareServiceWindow(mBackend, this);
        hardwareWindow->initialize();
        twPages->addTab(hardwareWindow, tr("#hardware"));
    }

    if (rights.contains(HumoServiceBackend::SetupNetwork) || !mBackend->hasAnyPassword())
    {
        NetworkServiceWindow *networkWindow = new NetworkServiceWindow(mBackend, this);
        networkWindow->initialize();
        twPages->addTab(networkWindow, tr("#network"));
    }

#ifdef TC_USE_TOKEN
    if (rights.contains(HumoServiceBackend::SetupKeys) || !mBackend->hasAnyPassword())
    {
        TokenServiceWindow *tokenWindow = new TokenServiceWindow(mBackend, this);
        tokenWindow->initialize();
        twPages->addTab(tokenWindow, tr("#token"));
    }
#endif

    if (rights.contains(HumoServiceBackend::SetupKeys) || !mBackend->hasAnyPassword())
    {
        KeysServiceWindow *keysWindow = new KeysServiceWindow(mBackend, this);
        keysWindow->initialize();
        twPages->addTab(keysWindow, tr("#keys"));
    }

    if (rights.contains(HumoServiceBackend::Encash))
    {
        DispenserServiceWindow *dispenserWindow = new DispenserServiceWindow(mBackend, this);
        dispenserWindow->initialize();
        twPages->addTab(dispenserWindow, tr("#dispenser"));
    }

    foreach (QWidget *widget, mBackend->getExternalWidgets())
    {
        twPages->addTab(widget, widget->property("widget_name").toString());
    }

    twPages->blockSignals(false);

    return true;
}

//------------------------------------------------------------------------
bool SetupServiceWindow::shutdown()
{
    foreach (QWidget *widget, mBackend->getExternalWidgets(false))
    {
        twPages->removeTab(twPages->indexOf(widget));
        widget->setParent(nullptr);
    }

    return true;
}

//------------------------------------------------------------------------
