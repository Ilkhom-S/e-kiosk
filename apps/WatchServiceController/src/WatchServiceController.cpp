/* @file Модуль управления сторожевым сервисом через сокет. */

#ifdef Q_OS_MACOS
// macOS imports for native status bar access
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/qpa/qplatformwindow.h>
#import <Cocoa/Cocoa.h>
#endif

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
    connect(&mTimer, SIGNAL(timeout()), SLOT(onCheck()));

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
        connect(action, SIGNAL(triggered(bool)), mSignalMapper, SLOT(map()));
        mSignalMapper->setMapping(action, QString("-start_scenario=service_menu"));
        mStartServiceActions << action;

        action = mMenu.addAction(QIcon(":/icons/menu-setupTemplate.png"), tr("#start_first_setup"));
        {
            QIcon icon = action->icon();
            icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
            action->setIcon(icon);
        }
        connect(action, SIGNAL(triggered(bool)), mSignalMapper, SLOT(map()));
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
    connect(action, SIGNAL(triggered(bool)), mSignalMapper, SLOT(map()));

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
    connect(mStopServiceAction, SIGNAL(triggered(bool)), SLOT(onStopServiceClicked()));
    connect(mCloseTrayIconAction, SIGNAL(triggered(bool)), SLOT(onCloseIconClicked()));

    connect(&mIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    mIcon.setContextMenu(&mMenu);
    QIcon trayIcon(":/icons/tray-monogramTemplate.png");
    trayIcon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
    qDebug() << "Tray icon is null:" << trayIcon.isNull() << "available sizes:" << trayIcon.availableSizes();
    mIcon.setIcon(trayIcon);
    mIcon.show();

    // Connect to theme changes to update tray icon
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
            [this]()
            {
                if (mClient->isConnected())
                {
                    mIcon.setIcon(getBadgedTrayIcon(":/icons/tray-monogramTemplate.png", false));
                }
                else
                {
                    mIcon.setIcon(getBadgedTrayIcon(":/icons/tray-monogram-stoppedTemplate.png", true));
                }
            });

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
QIcon WatchServiceController::getBadgedTrayIcon(const QString &iconPath, bool hasNotification)
{
    // Use template icon for macOS theme adaptation
    QIcon baseIcon(iconPath);

    if (!hasNotification)
    {
// macOS: Ensure template behavior
#ifdef Q_OS_MACOS
        baseIcon.setIsMask(true);
#endif
        return baseIcon;
    }

    // Prepare pixmap for painting (32x32 for retina support)
    QPixmap pixmap = baseIcon.pixmap(32, 32);
    pixmap.setDevicePixelRatio(2.0); // Ensures sharpness on Retina/4K

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw the Badge (Red Dot) - positioned in top-right corner
    int dotSize = 8;
    int margin = 2;
    painter.setBrush(Qt::red);
    painter.setPen(QPen(Qt::white, 1)); // White border makes it pop on any theme
    painter.drawEllipse(pixmap.width() - dotSize - margin, margin, dotSize, dotSize);
    painter.end();

    QIcon finalIcon(pixmap);

// macOS: Ensure it still behaves as a template
#ifdef Q_OS_MACOS
    finalIcon.setIsMask(true);
#endif

    LOG(getLog(), LogLevel::Normal, QString("Created badged icon for %1").arg(iconPath));
    return finalIcon;
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
        // Connected state: show normal template icon without badge
        QIcon normalIcon = getBadgedTrayIcon(":/icons/tray-monogramTemplate.png", false); // false = no notification
        mIcon.setIcon(normalIcon);
        foreach (auto action, mStartServiceActions)
        {
            action->setEnabled(false);
        }
        mStopServiceAction->setEnabled(true);
    }
    else
    {
        // Create badged icon for stopped state using platform-agnostic approach
        QIcon badgedIcon =
            getBadgedTrayIcon(":/icons/tray-monogram-stoppedTemplate.png", true); // true = has notification (red dot)
        mIcon.setIcon(badgedIcon);
        foreach (auto action, mStartServiceActions)
        {
            action->setEnabled(true);
        }
        mStopServiceAction->setEnabled(false);
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
        QString path = QDir::cleanPath(QDir::toNativeSeparators(BasicApplication::getInstance()->getWorkingDirectory() +
                                                                QDir::separator() + "guard.exe"));

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

#ifdef Q_OS_MACOS
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
#ifdef Q_OS_MACOS
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

#ifdef Q_OS_MACOS
#endif

//----------------------------------------------------------------------------
