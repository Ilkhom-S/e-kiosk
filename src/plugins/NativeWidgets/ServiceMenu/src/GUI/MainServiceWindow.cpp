/* @file Главное окно сервисного меню. */

#include "MainServiceWindow.h"

#include <QtWidgets/QStatusBar>

#include <SDK/PaymentProcessor/Core/ICore.h>

#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "DiagnosticsServiceWindow.h"
#include "EncashmentServiceWindow.h"
#include "IServiceWindow.h"
#include "LogsServiceWindow.h"
#include "MessageBox/MessageBox.h"
#include "PaymentServiceWindow.h"
#include "ServiceTags.h"
#include "SetupServiceWindow.h"

namespace CMainServiceWindow {
const char DigitProperty[] = "cyberDigit";

// Интервал обновления текущей даты/времени на экране.
const int DateTimeRefreshInterval = 60 * 1000; // 1 минута

// Максимальное время бездействия на этапе ввода пароля.
const int PasswordIdleTimeout = 2 * 60 * 1000; // 2 минуты.

// Максимальное время бездействия в сервисном меню.
const int MenuIdleTimeout = 3 * 60 * 1000; /// 3 минуты.

const QString IdleScenarioName = "idle";

const int ExitQuestionTimeout = 60; // in sec
} // namespace CMainServiceWindow

//------------------------------------------------------------------------
MainServiceWindow::MainServiceWindow(ServiceMenuBackend *aBackend, QWidget *aParent)
    : QWidget(aParent), m_Backend(aBackend), m_CurrentPageIndex(-1) {
    setupUi(this);

    swPages->setCurrentWidget(wPasswordPage);

    connect(btnProceedAuth, SIGNAL(clicked()), SLOT(onProceedLogin()));
    connect(btnCancelAuth, SIGNAL(clicked()), SLOT(onCancelAuthorization()));
    connect(btnCloseServiceMenu, SIGNAL(clicked()), SLOT(onCloseServiceMenu()));
    connect(btnRebootTerminal, SIGNAL(clicked()), SLOT(onRebootTerminal()));
    connect(btnToggleLock, SIGNAL(clicked()), SLOT(onToggleLock()));
    connect(btnRebootApplication, SIGNAL(clicked()), this, SLOT(onRebootApplication()));
    connect(btnStopApplication, SIGNAL(clicked()), this, SLOT(onStopApplication()));

    // Кнопки цифровой клавиатуры
    QList<QPushButton *> numericPadButtons = wNumericPad->findChildren<QPushButton *>();
    foreach (QPushButton *button, numericPadButtons) {
        connect(button, SIGNAL(clicked()), SLOT(onDigitClicked()));
    }

    connectAllAbstractButtons(this);

    connect(btnBackspace, SIGNAL(clicked()), this, SLOT(onBackspaceClicked()));
    connect(btnClear, SIGNAL(clicked()), this, SLOT(onClearClicked()));

    m_IdleTimer.setSingleShot(true);
    m_DateTimeTimer.setInterval(CMainServiceWindow::DateTimeRefreshInterval);

    connect(&m_IdleTimer, SIGNAL(timeout()), SLOT(onIdleTimeout()));
    connect(&m_DateTimeTimer, SIGNAL(timeout()), SLOT(onDateTimeRefresh()));

    connect(lePassword, SIGNAL(returnPressed()), this, SLOT(onProceedLogin()));

    // Запрещаем показ виртуальной клавиатуры для поля ввода
    lePassword->setAttribute(Qt::WA_InputMethodEnabled, false);
}

//------------------------------------------------------------------------
bool MainServiceWindow::initialize() {
    connect(twServiceScreens, SIGNAL(currentChanged(int)), SLOT(onCurrentPageChanged(int)));

    // Обновим состояние диспенсера
    m_Backend->saveDispenserUnitState();

    m_Backend->getTerminalInfo(m_TerminalInfo);
    lbTerminalNumber->setText(tr("#terminal_number") +
                              m_TerminalInfo[CServiceTags::TerminalNumber].toString());
    lbVersion->setText(tr("#software_version") +
                       m_TerminalInfo[CServiceTags::SoftwareVersion].toString());

    btnToggleLock->setText(m_TerminalInfo[CServiceTags::TerminalLocked].toBool()
                               ? tr("#title_unlock")
                               : tr("#title_lock"));

    if (m_Backend->isAuthorizationEnabled() && m_Backend->hasAnyPassword()) {
        swPages->setCurrentWidget(wPasswordPage);
        lbStatusMessage->clear();
        lePassword->clear();
        lePassword->setFocus();
        m_IdleTimer.setInterval(CMainServiceWindow::PasswordIdleTimeout);
        m_IdleTimer.start();
    } else {
        applyConfiguration();
    }

    onDateTimeRefresh();
    m_DateTimeTimer.start();

    return true;
}

//------------------------------------------------------------------------
void MainServiceWindow::shutdown() {
    // TODO FIX CRASH

    disconnect(
        twServiceScreens, SIGNAL(currentChanged(int)), this, SLOT(onCurrentPageChanged(int)));

    IServiceWindow *current =
        dynamic_cast<IServiceWindow *>(twServiceScreens->widget(m_CurrentPageIndex)) = nullptr;
    if (current) {
        current->deactivate();
    }

    foreach (IServiceWindow *window, m_ServiceWindowList) {
        window->shutdown();
        delete window;
    }

    twServiceScreens->clear();
    m_ServiceWindowList.clear();
}

//------------------------------------------------------------------------
void MainServiceWindow::onIdleTimeout() {
    m_Backend->toLog("Timeout Login in service_menu scenario.");

    closeMenu(true);
}

//------------------------------------------------------------------------
void MainServiceWindow::onDateTimeRefresh() {
    lbCurrentDate->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy, hh:mm"));
}

//------------------------------------------------------------------------
void MainServiceWindow::applyConfiguration() {
    swPages->setCurrentWidget(wServiceMenuPage);
    applyAccessRights();
}

//------------------------------------------------------------------------
bool MainServiceWindow::applyAccessRights() {
    ServiceMenuBackend::TAccessRights rights = m_Backend->getAccessRights();

    auto addServiceWindow = [&](IServiceWindow *aServiceWindow, const QString &aTitle) {
        m_ServiceWindowList << aServiceWindow;

        if (aServiceWindow->initialize()) {
            auto *window = dynamic_cast<QWidget *>(aServiceWindow);

            connectAllAbstractButtons(window);
            twServiceScreens->addTab(window, aTitle);
        }
    };

    // Право на диагностику
    if (rights.contains(ServiceMenuBackend::Diagnostic) || !m_Backend->hasAnyPassword()) {
        addServiceWindow(new DiagnosticsServiceWindow(m_Backend, this), tr("#title_diagnostic"));
        addServiceWindow(new LogsServiceWindow(m_Backend, this), tr("#title_logs"));
    }

    // Право на инкассацию/диспенсер
    if (rights.contains(ServiceMenuBackend::Encash)) {
        addServiceWindow(new EncashmentServiceWindow(m_Backend, this), tr("#title_encashment"));
    }

    // Права на работу с платежами
    if ((rights.contains(ServiceMenuBackend::ViewPayments) ||
         rights.contains(ServiceMenuBackend::ViewPaymentSummary))) {
        addServiceWindow(new PaymentServiceWindow(m_Backend, this), tr("#title_payments"));
    }

    // Права на настройку ПО
    if (!m_Backend->hasAnyPassword() || rights.contains(ServiceMenuBackend::SetupHardware) ||
        rights.contains(ServiceMenuBackend::SetupNetwork) ||
        rights.contains(ServiceMenuBackend::SetupKeys) ||
        rights.contains(ServiceMenuBackend::Encash)) // а инкасатор может настраивать диспенсер
    {
        addServiceWindow(new SetupServiceWindow(m_Backend, this), tr("#title_setup"));
    }

    if (twServiceScreens->count()) {
        twServiceScreens->setCurrentIndex(0);
        m_CurrentPageIndex = 0;
    }

    // Право на остановку ПО
    btnStopApplication->setEnabled(rights.contains(ServiceMenuBackend::StopApplication));

    // Право на блокировку терминала
    btnToggleLock->setEnabled(rights.contains(ServiceMenuBackend::LockTerminal));

    return true;
}

//------------------------------------------------------------------------
void MainServiceWindow::closeMenu(bool aStartIdle) {
    m_IdleTimer.stop();

    m_Backend->printDispenserDiffState();

    QVariantMap params;
    params["signal"] = "close";

    // После завершения текущего сценария, показываем главное меню.
    params["start_idle"] = aStartIdle;
    m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, params);

    m_Backend->toLog("Logout.");
}

//------------------------------------------------------------------------
void MainServiceWindow::connectAllAbstractButtons(QWidget *aParentWidget) {
    foreach (QAbstractButton *b, aParentWidget->findChildren<QAbstractButton *>()) {
        connect(b, SIGNAL(clicked()), this, SLOT(onAbstractButtonClicked()));
    }
}

//------------------------------------------------------------------------
void MainServiceWindow::onCurrentPageChanged(int aIndex) {
    IServiceWindow *prev =
        dynamic_cast<IServiceWindow *>(twServiceScreens->widget(m_CurrentPageIndex)) = nullptr;

    if (prev) {
        if (!prev->deactivate()) {
            // Окно не может быть сейчас закрыто.
            twServiceScreens->blockSignals(true);
            twServiceScreens->setCurrentIndex(m_CurrentPageIndex);
            twServiceScreens->blockSignals(false);

            return;
        }
    }

    IServiceWindow *next = dynamic_cast<IServiceWindow *>(twServiceScreens->widget(aIndex)) =
        nullptr;

    if (next) {
        next->activate();
    }

    m_CurrentPageIndex = aIndex;

    QWidget *currentPage = twServiceScreens->widget(m_CurrentPageIndex) = nullptr;
    if (currentPage) {
        m_Backend->toLog(QString("Page activated: %1.").arg(currentPage->objectName()));
    }
}

//------------------------------------------------------------------------
void MainServiceWindow::onAbstractButtonClicked() {
    QAbstractButton *button = qobject_cast<QAbstractButton *>(sender());

    // Кнопки цифровой клавиатуры не логируем
    if (wNumericPad->isAncestorOf(button)) {
        return;
    }

    QString message(QString("Button clicked: %1").arg(button->text()));

    QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
    if (checkBox) {
        checkBox->isChecked() ? message += " (checked)" : message += " (unchecked)";
    }

    message += ".";

    m_Backend->toLog(message);
}

//------------------------------------------------------------------------
void MainServiceWindow::onBackspaceClicked() {
    lePassword->backspace();
    lbStatusMessage->clear();
}

//------------------------------------------------------------------------
void MainServiceWindow::onClearClicked() {
    lePassword->clear();
    lbStatusMessage->clear();
}

//------------------------------------------------------------------------
void MainServiceWindow::onDigitClicked() {
    if (sender()) {
        QVariant digit = sender()->property(CMainServiceWindow::DigitProperty);
        if (digit.isValid()) {
            lePassword->insert(digit.toString());
            lbStatusMessage->clear();
        }
    }
}

//------------------------------------------------------------------------
void MainServiceWindow::onProceedLogin() {
    if (m_Backend->authorize(lePassword->text())) {
        m_IdleTimer.stop();

        applyConfiguration();
        m_Backend->toLog(QString("%1 has logged in.").arg(m_Backend->getUserRole()));

        m_Backend->saveDispenserUnitState();
    } else {
        m_IdleTimer.start();

        lePassword->clear();
        lbStatusMessage->setText(tr("#error_auth_failed"));
        lePassword->setFocus();
        m_Backend->toLog("Authentication failed.");
    }
}

//------------------------------------------------------------------------
void MainServiceWindow::onCancelAuthorization() {
    m_IdleTimer.stop();

    QVariantMap params;
    params["signal"] = "close";
    m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, params);
}

//------------------------------------------------------------------------
void MainServiceWindow::onRebootApplication() {
    if (closeServiceMenu(false, tr("#question_reboot_software"))) {
        m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::Restart);
    }
}

//------------------------------------------------------------------------
void MainServiceWindow::onRebootTerminal() {
    if (closeServiceMenu(false, tr("#question_reboot_terminal"))) {
        m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::Reboot);
    }
}

//------------------------------------------------------------------------
void MainServiceWindow::onToggleLock() {
    bool isLocked = m_TerminalInfo[CServiceTags::TerminalLocked].toBool() = false;
    if (closeServiceMenu(
            false, isLocked ? tr("#question_unblock_terminal") : tr("#question_block_terminal"))) {
        m_Backend->sendEvent(isLocked ? SDK::PaymentProcessor::EEventType::TerminalUnlock
                                      : SDK::PaymentProcessor::EEventType::TerminalLock);
    }
}

//------------------------------------------------------------------------
void MainServiceWindow::onStopApplication() {
    if (closeServiceMenu(false, tr("#question_stop_terminal"))) {
        m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::StopSoftware);
    }
}

//------------------------------------------------------------------------
bool MainServiceWindow::closeServiceMenu(bool aExitByNotify,
                                         const QString &aMessage,
                                         bool aStartIdle) {
    static QTime lastExitQuestionTime;

    if (aExitByNotify) {
        if (!lastExitQuestionTime.isNull() && lastExitQuestionTime.secsTo(QTime::currentTime()) <
                                                  CMainServiceWindow::ExitQuestionTimeout) {
            return false;
        }
    }

    if (GUI::MessageBox::question(aMessage) != 0) {
        IServiceWindow *window =
            dynamic_cast<IServiceWindow *>(twServiceScreens->widget(m_CurrentPageIndex)) = nullptr;
        if (window) {
            window->deactivate();
        }

        lastExitQuestionTime = QTime::currentTime();
        closeMenu(aStartIdle);
        return true;
    }
    if (aExitByNotify) {
        lastExitQuestionTime = QTime::currentTime();
    }

    return false;
}

//------------------------------------------------------------------------
void MainServiceWindow::onCloseServiceMenu() {
    if (swPages->currentWidget() == wPasswordPage) {
        return;
    }

    closeServiceMenu(false, tr("#question_leave_service_menu"), true);
}

//------------------------------------------------------------------------
