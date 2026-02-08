/* @file Окно редактирования купюр в диспенсере. */

#include "DispenserServiceWindow.h"

#include <QtCore/QDir>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include "Backend/HardwareManager.h"
#include "Backend/ServiceMenuBackend.h"

DispenserServiceWindow::DispenserServiceWindow(ServiceMenuBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend) {
    setupUi(this);
}

//------------------------------------------------------------------------
bool DispenserServiceWindow::activate() {
    lwCashUnits->clear();

    PPSDK::TCashUnitsState cashUnitState =
        m_Backend->getCore()->getFundsService()->getDispenser()->getCashUnitsState();

    if (cashUnitState.isEmpty()) {
        return false;
    }

    foreach (QString device, cashUnitState.keys()) {
        QVariantMap config = m_Backend->getHardwareManager()->getDeviceConfiguration(device);

        for (const PPSDK::SCashUnit &cashUnit : cashUnitState.value(device)) {
            QListWidgetItem *item =
                new QListWidgetItem(QString("%1:%2 -> %3 x %4 %5 = %6%7")
                                        .arg(config.value("system_name").toString())
                                        .arg(config.value("model_name").toString())
                                        .arg(cashUnit.nominal)
                                        .arg(cashUnit.count)
                                        .arg(tr("#pts"))
                                        .arg(cashUnit.nominal * cashUnit.count)
                                        .arg(cashUnit.currencyName),
                                    lwCashUnits);

            lwCashUnits->addItem(item);
        }
    }

    return true;
}

//------------------------------------------------------------------------
bool DispenserServiceWindow::deactivate() {
    return true;
}

//------------------------------------------------------------------------
bool DispenserServiceWindow::initialize() {
    return true;
}

//------------------------------------------------------------------------
bool DispenserServiceWindow::shutdown() {
    return true;
}

//------------------------------------------------------------------------
