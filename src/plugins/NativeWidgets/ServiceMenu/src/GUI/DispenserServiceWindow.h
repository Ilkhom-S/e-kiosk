/* @file Окно редактирования купюр в диспенсере. */

#pragma once

#include <QtCore/QFutureWatcher>

#include <SDK/PaymentProcessor/Core/ICashDispenserManager.h>

#include "IServiceWindow.h"
#include "ui_DispenserServiceWindow.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
class DispenserServiceWindow : public QFrame,
                               public ServiceWindowBase,
                               protected Ui::DispenserServiceWindow {
    Q_OBJECT

public:
    DispenserServiceWindow(ServiceMenuBackend *aBackend, QWidget *aParent = 0);

public:
    virtual bool initialize();
    virtual bool shutdown();
    virtual bool activate();
    virtual bool deactivate();
};

//------------------------------------------------------------------------
