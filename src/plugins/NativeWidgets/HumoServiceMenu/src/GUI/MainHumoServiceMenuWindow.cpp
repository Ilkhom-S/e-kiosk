/* @file Главное окно HumoService */

#include "MainHumoServiceMenuWindow.h"

#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMessageBox>

#include "Backend/HumoServiceBackend.h"
#include "DiagnosticsServiceWindow.h"
#include "EncashmentServiceWindow.h"
#include "LogsServiceWindow.h"
#include "PaymentServiceWindow.h"
#include "SetupServiceWindow.h"

MainHumoServiceMenuWindow::MainHumoServiceMenuWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QWidget(aParent), mBackend(aBackend), mCurrentPageIndex(0) {
    setupUi(this);

    // Setup timers
    connect(&mIdleTimer, &QTimer::timeout, this, &MainHumoServiceMenuWindow::onIdleTimeout);
    connect(&mDateTimeTimer, &QTimer::timeout, this, &MainHumoServiceMenuWindow::onDateTimeRefresh);

    // Setup connections
    connect(btnCloseServiceMenu,
            &QPushButton::clicked,
            this,
            &MainHumoServiceMenuWindow::onCloseHumoServiceMenu);
    connect(btnRebootApplication,
            &QPushButton::clicked,
            this,
            &MainHumoServiceMenuWindow::onRebootApplication);
    connect(btnStopApplication,
            &QPushButton::clicked,
            this,
            &MainHumoServiceMenuWindow::onStopApplication);
    connect(btnRebootTerminal,
            &QPushButton::clicked,
            this,
            &MainHumoServiceMenuWindow::onRebootTerminal);
    connect(btnToggleLock, &QPushButton::clicked, this, &MainHumoServiceMenuWindow::onToggleLock);

    // Connect tab widget
    connect(twServiceScreens,
            &QTabWidget::currentChanged,
            this,
            &MainHumoServiceMenuWindow::onCurrentPageChanged);

    // Connect password page
    connect(
        btnProceedAuth, &QPushButton::clicked, this, &MainHumoServiceMenuWindow::onProceedLogin);
    connect(btnCancelAuth,
            &QPushButton::clicked,
            this,
            &MainHumoServiceMenuWindow::onCancelAuthorization);

    // Connect digital keyboard
    connect(
        btnBackspace, &QPushButton::clicked, this, &MainHumoServiceMenuWindow::onBackspaceClicked);
    connect(btnClear, &QPushButton::clicked, this, &MainHumoServiceMenuWindow::onClearClicked);

    // Connect digit buttons
    digitGroup = new QButtonGroup(this);
    digitGroup->addButton(btnDigit0, 0);
    digitGroup->addButton(btnDigit1, 1);
    digitGroup->addButton(btnDigit2, 2);
    digitGroup->addButton(btnDigit3, 3);
    digitGroup->addButton(btnDigit4, 4);
    digitGroup->addButton(btnDigit5, 5);
    digitGroup->addButton(btnDigit6, 6);
    digitGroup->addButton(btnDigit7, 7);
    digitGroup->addButton(btnDigit8, 8);
    digitGroup->addButton(btnDigit9, 9);
    connect(
        digitGroup, &QButtonGroup::buttonClicked, this, &MainHumoServiceMenuWindow::onDigitClicked);
}

//--------------------------------------------------------------------------
bool MainHumoServiceMenuWindow::initialize() {
    // Get terminal info
    mBackend->getTerminalInfo(mTerminalInfo);

    // Apply configuration
    applyConfiguration();

    // Start timers
    mDateTimeTimer.start(1000);      // Update every second
    mIdleTimer.start(5 * 60 * 1000); // 5 minutes idle timeout

    return true;
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::shutdown() {
    mIdleTimer.stop();
    mDateTimeTimer.stop();
}

//--------------------------------------------------------------------------
WizardWindow *MainHumoServiceMenuWindow::getScreenByName(const QString &aScreenName) {
    // TODO: Implement screen retrieval
    return nullptr;
}

//--------------------------------------------------------------------------
bool MainHumoServiceMenuWindow::closeHumoServiceMenu(bool aExitByNotify,
                                                     const QString &aMessage,
                                                     bool aStartIdle) {
    if (!aExitByNotify && !aMessage.isEmpty()) {
        QMessageBox::information(this, tr("HumoService"), aMessage);
    }

    closeMenu(aStartIdle);
    return true;
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onCurrentPageChanged(int aIndex) {
    mCurrentPageIndex = aIndex;
    applyAccessRights();
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onProceedLogin() {
    QString password = lePassword->text();
    if (mBackend->authorize(password)) {
        // Switch to main page
        twServiceScreens->setCurrentIndex(1);
        lePassword->clear();
    } else {
        QMessageBox::warning(this, tr("Authorization Failed"), tr("Invalid password"));
    }
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onCancelAuthorization() {
    lePassword->clear();
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onCloseHumoServiceMenu() {
    closeHumoServiceMenu(false, QString(), true);
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onRebootApplication() {
    if (QMessageBox::question(this,
                              tr("Reboot Application"),
                              tr("Are you sure you want to reboot the application?")) ==
        QMessageBox::Yes) {
        mBackend->sendEvent(SDK::PaymentProcessor::EEventType::Restart);
    }
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onStopApplication() {
    if (QMessageBox::question(
            this, tr("Stop Application"), tr("Are you sure you want to stop the application?")) ==
        QMessageBox::Yes) {
        mBackend->sendEvent(SDK::PaymentProcessor::EEventType::CloseApplication);
    }
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onRebootTerminal() {
    if (QMessageBox::question(
            this, tr("Reboot Terminal"), tr("Are you sure you want to reboot the terminal?")) ==
        QMessageBox::Yes) {
        mBackend->sendEvent(SDK::PaymentProcessor::EEventType::Reboot);
    }
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onToggleLock() {
    // TODO: Implement terminal lock/unlock
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onBackspaceClicked() {
    QString text = lePassword->text();
    if (!text.isEmpty()) {
        text.chop(1);
        lePassword->setText(text);
    }
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onClearClicked() {
    lePassword->clear();
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onDigitClicked(QAbstractButton *aButton) {
    QPushButton *button = qobject_cast<QPushButton *>(aButton);
    if (button) {
        QString digit = button->text();
        lePassword->setText(lePassword->text() + digit);
    }
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onIdleTimeout() {
    closeHumoServiceMenu(false, tr("Session timeout"), true);
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onDateTimeRefresh() {
    // TODO: Update date/time display
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::applyConfiguration() {
    // Clear existing tabs
    while (twServiceScreens->count() > 1) {
        twServiceScreens->removeTab(1);
    }

    // Clear service window list
    qDeleteAll(mServiceWindowList);
    mServiceWindowList.clear();

    // Add main service tabs
    HumoServiceBackend::TAccessRights rights = mBackend->getAccessRights();

    // Setup tab
    if (rights.contains(HumoServiceBackend::SetupHardware) ||
        rights.contains(HumoServiceBackend::SetupNetwork) ||
        rights.contains(HumoServiceBackend::SetupKeys) || !mBackend->hasAnyPassword()) {
        SetupServiceWindow *setupWindow = new SetupServiceWindow(mBackend, this);
        setupWindow->initialize();
        twServiceScreens->addTab(setupWindow, tr("#setup"));
        mServiceWindowList.append(setupWindow);
    }

    // Diagnostics tab
    if (rights.contains(HumoServiceBackend::Diagnostic) || !mBackend->hasAnyPassword()) {
        DiagnosticsServiceWindow *diagnosticsWindow = new DiagnosticsServiceWindow(mBackend, this);
        diagnosticsWindow->initialize();
        twServiceScreens->addTab(diagnosticsWindow, tr("#diagnostics"));
        mServiceWindowList.append(diagnosticsWindow);
    }

    // Payments tab
    if (rights.contains(HumoServiceBackend::ViewPayments) || !mBackend->hasAnyPassword()) {
        PaymentServiceWindow *paymentWindow = new PaymentServiceWindow(mBackend, this);
        paymentWindow->initialize();
        twServiceScreens->addTab(paymentWindow, tr("#payments"));
        mServiceWindowList.append(paymentWindow);
    }

    // Logs tab
    if (rights.contains(HumoServiceBackend::ViewLogs) || !mBackend->hasAnyPassword()) {
        LogsServiceWindow *logsWindow = new LogsServiceWindow(mBackend, this);
        logsWindow->initialize();
        twServiceScreens->addTab(logsWindow, tr("#logs"));
        mServiceWindowList.append(logsWindow);
    }

    // Encashment tab
    if (rights.contains(HumoServiceBackend::Encash) || !mBackend->hasAnyPassword()) {
        EncashmentServiceWindow *encashmentWindow = new EncashmentServiceWindow(mBackend, this);
        encashmentWindow->initialize();
        twServiceScreens->addTab(encashmentWindow, tr("#encashment"));
        mServiceWindowList.append(encashmentWindow);
    }

    // Add external widgets from plugins
    foreach (QWidget *widget, mBackend->getExternalWidgets()) {
        twServiceScreens->addTab(widget, widget->property("widget_name").toString());
    }
}

//--------------------------------------------------------------------------
bool MainHumoServiceMenuWindow::applyAccessRights() {
    // TODO: Apply access rights based on user permissions
    return true;
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::closeMenu(bool aStartIdle) {
    hide();
    if (aStartIdle) {
        // TODO: Start idle scenario
    }
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::onAbstractButtonClicked() {
    QAbstractButton *button = qobject_cast<QAbstractButton *>(sender());
    if (!button)
        return;

    QString message(QString("Button clicked: %1").arg(button->text()));

    QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
    if (checkBox) {
        checkBox->isChecked() ? message += " (checked)" : message += " (unchecked)";
    }

    message += ".";

    // TODO: Add logging if needed
    // mBackend->toLog(message);
}

//--------------------------------------------------------------------------
void MainHumoServiceMenuWindow::connectAllAbstractButtons(QWidget *aParentWidget) {
    // TODO: Connect abstract buttons
}

//--------------------------------------------------------------------------