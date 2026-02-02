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
        QIcon icon(":/icons/menu-settingsTemplate.png");
        icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
        auto action = mMenu.addAction(icon, tr("#start_service_menu"));
        // Remove manual disabled pixmap creation - let Qt handle disabled state with template
        connect(action, &QAction::triggered, mSignalMapper, [this, action]() { mSignalMapper->map(action); });
        mSignalMapper->setMapping(action, QString("-start_scenario=service_menu"));
        mStartServiceActions << action;

        action = mMenu.addAction(QIcon(":/icons/menu-setupTemplate.png"), tr("#start_first_setup"));
        {
            QIcon icon = action->icon();
            icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
            action->setIcon(icon);
        }
        connect(action, &QAction::triggered, mSignalMapper, [this, action]() { mSignalMapper->map(action); });
        mSignalMapper->setMapping(action, QString("-start_scenario=first_setup"));
        mStartServiceActions << action;

        mMenu.addSeparator();
    }

    auto action = mMenu.addAction(QIcon(":/icons/menu-playTemplate.png"), tr("#start_service"));
    {
        QIcon icon = action->icon();
        icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
        action->setIcon(icon);
    }
    connect(action, &QAction::triggered, mSignalMapper, [this, action]() { mSignalMapper->map(action); });

    mSignalMapper->setMapping(action, QString("--disable-web-security"));
    mStartServiceActions << action;

    mStopServiceAction = mMenu.addAction(QIcon(":/icons/menu-stopTemplate.png"), tr("#stop_service"));
    {
        QIcon icon = mStopServiceAction->icon();
        icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
        mStopServiceAction->setIcon(icon);
    }
    mMenu.addSeparator();
    mCloseTrayIconAction = mMenu.addAction(QIcon(":/icons/menu-closeTemplate.png"), tr("#close"));
    {
        QIcon icon = mCloseTrayIconAction->icon();
        icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
        mCloseTrayIconAction->setIcon(icon);
    }

    connect(mSignalMapper, SIGNAL(mapped(QString)), SLOT(onStartServiceClicked(QString)));
    connect(mStopServiceAction, &QAction::triggered, this, &WatchServiceController::onStopServiceClicked);
    connect(mCloseTrayIconAction, &QAction::triggered, this, &WatchServiceController::onCloseIconClicked);

    connect(&mIcon, &QSystemTrayIcon::activated, this, &WatchServiceController::onTrayIconActivated);

    mIcon.setContextMenu(&mMenu);
    QIcon trayIcon(":/icons/tray-monogramTemplate.png");
    trayIcon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
    qDebug() << "Tray icon is null:" << trayIcon.isNull() << "available sizes:" << trayIcon.availableSizes();
    mIcon.setIcon(trayIcon);
    mIcon.show();

    LOG(getLog(), LogLevel::Normal, "WatchServiceController started.");
}

//----------------------------------------------------------------------------
WatchServiceController::~WatchServiceController()
{
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
        QIcon normalIcon(":/icons/tray-monogramTemplate.png");
        normalIcon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
        mIcon.setIcon(normalIcon);
        // Enable all menu items when connected so user can interact
        foreach (auto action, mStartServiceActions)
        {
            action->setEnabled(true);
        }
        mStopServiceAction->setEnabled(true);
    }
    else
    {
        // Disconnected state: show slashed H icon to indicate stopped state
        QIcon stoppedIcon(":/icons/tray-monogram-stoppedTemplate.png");
        stoppedIcon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
        mIcon.setIcon(stoppedIcon);
        // Disable all menu items except close when not connected
        foreach (auto action, mStartServiceActions)
        {
            action->setEnabled(false);
        }
        mStopServiceAction->setEnabled(false);
        // Keep close action enabled
        mCloseTrayIconAction->setEnabled(true);
    }

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
        // Platform-agnostic executable/app bundle name with debug suffix support
        QString executableName = "guard";
#ifdef QT_DEBUG
        executableName += "d";
#endif
#ifdef Q_OS_WIN
        executableName += ".exe";
#elif defined(Q_OS_MAC)
        executableName += ".app";
#endif

        QString path = QDir::cleanPath(QDir::toNativeSeparators(BasicApplication::getInstance()->getWorkingDirectory() +
                                                                QDir::separator() + executableName));

        QStringList parameters;

        if (!aArguments.isEmpty())
        {
            parameters << QString("-client_options=%1").arg(aArguments);
        }

        QProcess::startDetached(path, parameters, BasicApplication::getInstance()->getWorkingDirectory());
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
