/* @file Модуль управления сторожевым сервисом через сокет. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
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

    // Create menu actions with direct connections instead of signal mapper
    {
        auto settingsAction =
            mMenu.addAction(createTemplateIcon(":/icons/menu-settingsTemplate.png"), tr("#start_service_menu"));
        connect(settingsAction, SIGNAL(triggered(bool)), this, SLOT(onStartServiceMenuClicked()));
        mStartServiceActions << settingsAction;

        auto setupAction =
            mMenu.addAction(createTemplateIcon(":/icons/menu-setupTemplate.png"), tr("#start_first_setup"));
        connect(setupAction, SIGNAL(triggered(bool)), this, SLOT(onStartFirstSetupClicked()));
        mStartServiceActions << setupAction;

        mMenu.addSeparator();
    }

    auto playAction = mMenu.addAction(createTemplateIcon(":/icons/menu-playTemplate.png"), tr("#start_service"));
    connect(playAction, SIGNAL(triggered(bool)), this, SLOT(onStartServiceClickedDirect()));
    mStartServiceActions << playAction;

    mStopServiceAction = mMenu.addAction(createTemplateIcon(":/icons/menu-stopTemplate.png"), tr("#stop_service"));
    mMenu.addSeparator();
    mCloseTrayIconAction = mMenu.addAction(createTemplateIcon(":/icons/menu-closeTemplate.png"), tr("#close"));

    connect(mStopServiceAction, SIGNAL(triggered(bool)), SLOT(onStopServiceClicked()));
    connect(mCloseTrayIconAction, SIGNAL(triggered(bool)), SLOT(onCloseIconClicked()));

    connect(&mIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    mIcon.setContextMenu(&mMenu);
    mIcon.setIcon(createTemplateIcon(":/icons/controller-monogramTemplate.png"));
    mIcon.show();

    LOG(getLog(), LogLevel::Normal, "WatchServiceController started.");
}

//----------------------------------------------------------------------------
// Helper to create template icons for macOS with multiple sizes for better scaling
QIcon WatchServiceController::createTemplateIcon(const QString &basePath)
{
    QIcon icon;

    // Extract base name without extension and without resource prefix
    QString baseName = basePath;
    if (baseName.startsWith(":/"))
    {
        baseName = baseName.mid(2); // Remove :/ prefix
    }
    if (baseName.endsWith(".png"))
    {
        baseName = baseName.left(baseName.length() - 4); // Remove .png extension
    }

    // Load multiple sizes for better scaling on different DPI and contexts
    // Available sizes: 16, 32, 48, 64, 96, 128, 256, 512
    static const QList<int> sizes = {16, 32, 48, 64, 96, 128, 256, 512};
    QStringList loadedSizes;

    for (int size : sizes)
    {
        QString sizePath;
        if (size == 48)
        {
            // Default size - use the base path
            sizePath = basePath;
        }
        else
        {
            // Size-specific variants (ico sizes for smaller ones)
            if (size <= 48)
            {
                sizePath = QString(":/%1-ico-%2.png").arg(baseName).arg(size);
            }
            else
            {
                sizePath = QString(":/%1-%2.png").arg(baseName).arg(size);
            }
        }

        // Check if the size variant exists in resources by trying to load it
        QPixmap pixmap(sizePath);
        if (!pixmap.isNull())
        {
            icon.addPixmap(pixmap);
            loadedSizes << QString("%1px").arg(size);
        }
    }

    // If no sizes were loaded, fall back to the original single-size approach
    if (icon.isNull())
    {
        icon = QIcon(basePath);
    }

    icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
    return icon;
}

//----------------------------------------------------------------------------
// Helper to create app icons with multiple sizes for better scaling (non-template icons)
QIcon WatchServiceController::createAppIcon(const QString &basePath)
{
    QIcon icon;

    // Extract base name without extension and without resource prefix
    QString baseName = basePath;
    if (baseName.startsWith(":/"))
    {
        baseName = baseName.mid(2); // Remove :/ prefix
    }
    if (baseName.endsWith(".png"))
    {
        baseName = baseName.left(baseName.length() - 4); // Remove .png extension
    }

    // Load multiple sizes for better scaling on different DPI and contexts
    // Available sizes: 16, 32, 48, 64, 96, 128, 256, 512
    static const QList<int> sizes = {16, 32, 48, 64, 96, 128, 256, 512};
    QStringList loadedSizes;

    for (int size : sizes)
    {
        QString sizePath;
        if (size == 48)
        {
            // Default size - use the base path
            sizePath = basePath;
        }
        else
        {
            // Size-specific variants (ico sizes for smaller ones)
            if (size <= 48)
            {
                sizePath = QString(":/%1-ico-%2.png").arg(baseName).arg(size);
            }
            else
            {
                sizePath = QString(":/%1-%2.png").arg(baseName).arg(size);
            }
        }

        // Check if the size variant exists in resources by trying to load it
        QPixmap pixmap(sizePath);
        if (!pixmap.isNull())
        {
            icon.addPixmap(pixmap);
            loadedSizes << QString("%1px").arg(size);
        }
    }

    // If no sizes were loaded, fall back to the original single-size approach
    if (icon.isNull())
    {
        icon = QIcon(basePath);
    }

    // Note: No setIsMask(true) for app icons - they should render normally
    return icon;
}

//----------------------------------------------------------------------------
// Helper to get platform-specific executable path
QString WatchServiceController::getExecutablePath(const QString &baseName) const
{
    QString executableName = baseName;

    // Add platform-specific extension
#ifdef Q_OS_WIN
    executableName += ".exe";
    // Get working directory and construct full path
    QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();
    return QDir::cleanPath(QDir::toNativeSeparators(workingDir + QDir::separator() + executableName));
#elif defined(Q_OS_MAC)
    executableName += ".app/Contents/MacOS/" + baseName;
    // Get working directory and construct full path
    QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();
    return QDir::cleanPath(QDir::toNativeSeparators(workingDir + QDir::separator() + executableName));
#else
    // Get working directory and construct full path
    QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();
    return QDir::cleanPath(QDir::toNativeSeparators(workingDir + QDir::separator() + executableName));
#endif
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
        mIcon.setIcon(createTemplateIcon(":/icons/controller-monogramTemplate.png"));
    }
    else
    {
        // Disconnected state: show slashed H icon to indicate stopped state
        mIcon.setIcon(createTemplateIcon(":/icons/controller-monogram-stoppedTemplate.png"));
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
        QString path = getExecutablePath("watchdog");

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
    // Use multi-size loading for crisp rendering
    QIcon appIcon = createAppIcon(":/icons/controller-app-icon.png");
    QPixmap iconPixmap = appIcon.pixmap(64, 64, QIcon::Normal, QIcon::On);
    if (!iconPixmap.isNull())
    {
        msgBox.setIconPixmap(iconPixmap);
    }
#endif

    msgBox.setWindowIcon(createAppIcon(":/icons/controller-app-icon.png"));

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
// Direct connection slots for menu actions
void WatchServiceController::onStartServiceMenuClicked()
{
    onStartServiceClicked("-start_scenario=service_menu");
}

void WatchServiceController::onStartFirstSetupClicked()
{
    onStartServiceClicked("-start_scenario=first_setup");
}

void WatchServiceController::onStartServiceClickedDirect()
{
    onStartServiceClicked("--disable-web-security");
}

//----------------------------------------------------------------------------
