/* @file Окно логов. */

#include "LogsServiceWindow.h"

#include <QtCore/QDir>
#include <QtGui/QKeyEvent>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include "Backend/HumoServiceBackend.h"

LogsServiceWindow::LogsServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend) {
    setupUi(this);

    connect(btnShowLog, SIGNAL(clicked()), this, SLOT(onShowLogButtonClicked()));
    connect(lvLogsList, SIGNAL(item_SelectionChanged()), this, SLOT(logsSelectionChanged()));

    connect(btnScrollHome, SIGNAL(clicked()), lvLog, SLOT(scrollToTop()));
    connect(btnScrollUp, SIGNAL(clicked()), this, SLOT(onScrollUpClicked()));
    connect(btnScrollDown, SIGNAL(clicked()), this, SLOT(onScrollDownClicked()));
    connect(btnScrollEnd, SIGNAL(clicked()), lvLog, SLOT(scrollToBottom()));

    connect(btnScrollHomeLogList, SIGNAL(clicked()), lvLogsList, SLOT(scrollToTop()));
    connect(btnScrollUpLogList, SIGNAL(clicked()), this, SLOT(onScrollUpLogListClicked()));
    connect(btnScrollDownLogList, SIGNAL(clicked()), this, SLOT(onScrollDownLogListClicked()));
    connect(btnScrollEndLogList, SIGNAL(clicked()), lvLogsList, SLOT(scrollToBottom()));

    connect(btnCloseLog, SIGNAL(clicked()), this, SLOT(onCloseLogClicked()));

    lvLog->setModel(&m_Model);

    // Используем reinterpret_cast через void* для корректной работы с multiple inheritance
    // См. docs/multiple-inheritance-rtti-casting.md
    void *terminalSettingsPtr =
        reinterpret_cast<void *>(m_Backend->getCore()->getSettingsService()->getAdapter(
            SDK::PaymentProcessor::CAdapterNames::TerminalAdapter));
    SDK::PaymentProcessor::TerminalSettings *terminalSettings =
        reinterpret_cast<SDK::PaymentProcessor::TerminalSettings *>(terminalSettingsPtr);

    QString logPath(QString("%1/../logs").arg(terminalSettings->getAppEnvironment().userDataPath));

    QDir logDir(logPath, "*.log", QDir::Name, QDir::Files);
    foreach (QFileInfo fileInfo, logDir.entryInfoList()) {
        m_Logs.insert(fileInfo.fileName(), fileInfo.filePath());
    }

    stackedWidget->setCurrentIndex(0);
}

//------------------------------------------------------------------------
bool LogsServiceWindow::activate() {
    lvLogsList->clear();
    lvLogsList->insertItems(0, m_Logs.keys());
    return true;
}

//------------------------------------------------------------------------
bool LogsServiceWindow::deactivate() {
    return true;
}

//------------------------------------------------------------------------
bool LogsServiceWindow::initialize() {
    return true;
}

//------------------------------------------------------------------------
bool LogsServiceWindow::shutdown() {
    return true;
}

//------------------------------------------------------------------------
void LogsServiceWindow::onShowLogButtonClicked() {
    if (lvLogsList->selectedItems().size() > 0) {
        QFile logFile(m_Logs.value(lvLogsList->selectedItems().first()->text()));

        if (logFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&logFile);

            QStringList splitted = stream.readAll().split("\r\n", Qt::SkipEmptyParts);

            m_Model.setStringList(splitted);
            lvLog->scrollToTop();

            stackedWidget->setCurrentIndex(1);

            labelLogName->setText(lvLogsList->selectedItems().first()->text());
        }
    }
}

//------------------------------------------------------------------------
void LogsServiceWindow::onCloseLogClicked() {
    stackedWidget->setCurrentIndex(0);
}

//------------------------------------------------------------------------
void LogsServiceWindow::onScrollDownClicked() {
    scrollPgDown(lvLog);
}

//------------------------------------------------------------------------
void LogsServiceWindow::onScrollUpClicked() {
    scrollPgUp(lvLog);
}

//------------------------------------------------------------------------
void LogsServiceWindow::logsSelectionChanged() {
    btnShowLog->setEnabled(lvLogsList->selectedItems().size() > 0);
}

//------------------------------------------------------------------------
void LogsServiceWindow::onScrollUpLogListClicked() {
    scrollPgUp(lvLogsList);
}

//------------------------------------------------------------------------
void LogsServiceWindow::onScrollDownLogListClicked() {
    scrollPgDown(lvLogsList);
}

//------------------------------------------------------------------------
