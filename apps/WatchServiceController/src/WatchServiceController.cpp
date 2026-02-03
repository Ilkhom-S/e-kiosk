/* @file Модуль управления сторожевым сервисом через сокет. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtGui/QPainter>
#include <QtGui/QStyleHints>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>

// System
#include <WatchServiceClient/Constants.h>

// Project
#include "WatchServiceController.h"

namespace CWatchServiceController
{
    const int CheckTimeout = 3 * 1000;
} // namespace CWatchServiceController

//----------------------------------------------------------------------------
WatchServiceController::WatchServiceController()
    : mClient(createWatchServiceClient(CWatchService::Modules::WatchServiceController)), mLastCommand(Unknown)
{
    connect(&mTimer, &QTimer::timeout, this, &WatchServiceController::onCheck);

    mClient->subscribeOnDisconnected(this);
    mClient->subscribeOnCloseCommandReceived(this);

    mTimer.setInterval(CWatchServiceController::CheckTimeout);
    mTimer.start();

    mSignalMapper = new QSignalMapper(this);

    {
        auto settingsAction =
            mMenu.addAction(createTemplateIcon(":/icons/menu-settingsTemplate.png"), tr("#start_service_menu"));
        connect(settingsAction, &QAction::triggered, mSignalMapper,
                [this, settingsAction]() { mSignalMapper->map(settingsAction); });
        mSignalMapper->setMapping(settingsAction, QString("-start_scenario=service_menu"));
        mStartServiceActions << settingsAction;

        auto setupAction =
            mMenu.addAction(createTemplateIcon(":/icons/menu-setupTemplate.png"), tr("#start_first_setup"));
        connect(setupAction, &QAction::triggered, mSignalMapper,
                [this, setupAction]() { mSignalMapper->map(setupAction); });
        mSignalMapper->setMapping(setupAction, QString("-start_scenario=first_setup"));
        mStartServiceActions << setupAction;

        mMenu.addSeparator();
    }

    auto playAction = mMenu.addAction(createTemplateIcon(":/icons/menu-playTemplate.png"), tr("#start_service"));
    connect(playAction, &QAction::triggered, mSignalMapper, [this, playAction]() { mSignalMapper->map(playAction); });

    mSignalMapper->setMapping(playAction, QString(""));
    mStartServiceActions << playAction;

    mStopServiceAction = mMenu.addAction(createTemplateIcon(":/icons/menu-stopTemplate.png"), tr("#stop_service"));
    mMenu.addSeparator();
    mCloseTrayIconAction = mMenu.addAction(createTemplateIcon(":/icons/menu-closeTemplate.png"), tr("#close"));

    connect(mSignalMapper, SIGNAL(mapped(QString)), SLOT(onStartServiceClicked(QString)));
    connect(mStopServiceAction, &QAction::triggered, this, &WatchServiceController::onStopServiceClicked);
    connect(mCloseTrayIconAction, &QAction::triggered, this, &WatchServiceController::onCloseIconClicked);

    connect(&mIcon, &QSystemTrayIcon::activated, this, &WatchServiceController::onTrayIconActivated);

    mIcon.setContextMenu(&mMenu);
    mIcon.setIcon(createTemplateIcon(":/icons/tray-monogramTemplate.png"));
    qDebug() << "Tray icon is null:" << mIcon.icon().isNull() << "available sizes:" << mIcon.icon().availableSizes();
    mIcon.show();

    LOG(getLog(), LogLevel::Normal, "WatchServiceController started.");
}

//----------------------------------------------------------------------------
// Helper to create template icons for macOS
QIcon WatchServiceController::createTemplateIcon(const QString &path)
{
    QIcon icon(path);
    icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
    return icon;
}

//----------------------------------------------------------------------------
// Helper to get platform-specific executable path
QString WatchServiceController::getExecutablePath(const QString &baseName) const
{
    QString executableName = baseName;

    // Add debug suffix if in debug mode
#ifdef QT_DEBUG
    executableName += "d";
#endif

    // Add platform-specific extension
#ifdef Q_OS_WIN
    executableName += ".exe";
#elif defined(Q_OS_MAC)
    executableName += ".app";
#endif

    // Get working directory and construct full path
    QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();
    return QDir::cleanPath(QDir::toNativeSeparators(workingDir + QDir::separator() + executableName));
}

//----------------------------------------------------------------------------
WatchServiceController::~WatchServiceController()
{
    // Stop timer to prevent further processing
    mTimer.stop();

    // Hide tray icon
    mIcon.hide();

    // Disconnect all signals (Qt does this automatically, but explicit for clarity)
    disconnect(&mTimer, &QTimer::timeout, this, &WatchServiceController::onCheck);
    disconnect(&mIcon, &QSystemTrayIcon::activated, this, &WatchServiceController::onTrayIconActivated);
    disconnect(mSignalMapper, SIGNAL(mapped(QString)), this, SLOT(onStartServiceClicked(QString)));
    disconnect(mStopServiceAction, &QAction::triggered, this, &WatchServiceController::onStopServiceClicked);
    disconnect(mCloseTrayIconAction, &QAction::triggered, this, &WatchServiceController::onCloseIconClicked);

    LOG(getLog(), LogLevel::Normal, "WatchServiceController stopped.");
}

//----------------------------------------------------------------------------
ILog *WatchServiceController::getLog()
{
    if (BasicApplication::getInstance())
    {
        return BasicApplication::getInstance()->getLog();
    }

    return nullptr;
}

//----------------------------------------------------------------------------
void WatchServiceController::onCheck()
{
    if (!mClient->isConnected())
    {
        mLastCommand = Unknown;

        mClient->start();
    }

    if (mClient->isConnected())
    {
        // Connected state: show normal template icon
        mIcon.setIcon(createTemplateIcon(":/icons/tray-monogramTemplate.png"));
    }
    else
    {
        // Disconnected state: show slashed H icon to indicate stopped state
        mIcon.setIcon(createTemplateIcon(":/icons/tray-monogram-stoppedTemplate.png"));
    }

    // Always enable start/stop service actions regardless of connection status
    foreach (auto action, mStartServiceActions)
    {
        action->setEnabled(true);
    }
    mStopServiceAction->setEnabled(true);
    // Keep close action enabled
    mCloseTrayIconAction->setEnabled(true);

    mIcon.show();
}

//----------------------------------------------------------------------------
void WatchServiceController::onDisconnected()
{
    onCheck();
}

//----------------------------------------------------------------------------
void WatchServiceController::onCloseCommandReceived()
{
    if (mLastCommand != Stop)
    {
        LOG(getLog(), LogLevel::Normal, "Close tray by command from watch service.");

        QCoreApplication::instance()->quit();
    }
    else
    {
        LOG(getLog(), LogLevel::Normal, "Ignore close command, because I initiate it.");
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onStartServiceClicked(const QString &aArguments)
{
    LOG(getLog(), LogLevel::Normal, QString("User say: start service. %1").arg(aArguments));

    mLastCommand = Start;

    if (!mClient->isConnected())
    {
        // Validate application instance
        if (!BasicApplication::getInstance())
        {
            LOG(getLog(), LogLevel::Error, "Cannot start service: BasicApplication instance is null");
            return;
        }

        QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();

        // Validate working directory
        if (!QDir(workingDir).exists())
        {
            LOG(getLog(), LogLevel::Error,
                QString("Cannot start service: working directory does not exist: %1").arg(workingDir));
            return;
        }

        // Get platform-specific executable path
        QString path = getExecutablePath("guard");

        // Validate executable exists
        if (!QFile::exists(path))
        {
            LOG(getLog(), LogLevel::Error, QString("Cannot start service: executable does not exist: %1").arg(path));
            return;
        }

        QStringList parameters;

        if (!aArguments.isEmpty())
        {
            parameters << QString("-client_options=%1").arg(aArguments);
        }

        // Attempt to start the process
        bool started = QProcess::startDetached(path, parameters, workingDir);

        if (started)
        {
            LOG(getLog(), LogLevel::Normal, QString("Successfully started service process: %1").arg(path));
        }
        else
        {
            LOG(getLog(), LogLevel::Error, QString("Failed to start service process: %1").arg(path));
        }
    }
    else
    {
        mClient->restartService(QStringList());
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onStopServiceClicked()
{
    LOG(getLog(), LogLevel::Normal, "User say: stop service.");

    mLastCommand = Stop;

    if (mClient->isConnected())
    {
        mClient->stopService();
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onCloseIconClicked()
{
    QMessageBox msgBox(nullptr);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(tr("#exit"));
    msgBox.setText(tr("#confirm_close_trayicon"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);

#ifdef Q_OS_MAC
    // On macOS, add the icon to the dialog since window icons don't show in title bar
    QPixmap iconPixmap(":/icons/tray-app-icon.png");
    if (!iconPixmap.isNull())
    {
        msgBox.setIconPixmap(iconPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
#endif

    msgBox.setWindowIcon(QIcon(":/icons/tray-app-icon.png"));

    int result = msgBox.exec();
    if (result == QMessageBox::Yes)
    {
        QCoreApplication::instance()->quit();
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onTrayIconActivated(QSystemTrayIcon::ActivationReason aReason)
{
    onCheck();

    // Handle tray icon activation based on platform
#ifdef Q_OS_MAC
    // On macOS, the context menu is shown automatically by the system
    // when the tray icon has a context menu set, so we don't need to show it manually
    Q_UNUSED(aReason)
#else
    // On Windows/Linux, show context menu on left-click or right-click
    if (aReason == QSystemTrayIcon::Trigger || aReason == QSystemTrayIcon::Context)
    {
        mMenu.popup(QCursor::pos());
        mMenu.activateWindow();
    }
#endif
}

//----------------------------------------------------------------------------
