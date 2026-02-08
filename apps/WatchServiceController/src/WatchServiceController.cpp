/* @file Модуль управления сторожевым сервисом через сокет. */

#include "WatchServiceController.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtGui/QPainter>
#include <QtGui/QStyleHints>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include <Common/BasicApplication.h>

#include <WatchServiceClient/Constants.h>

namespace CWatchServiceController {
const int CheckTimeout = 3 * 1000;
} // namespace CWatchServiceController

//----------------------------------------------------------------------------
WatchServiceController::WatchServiceController()
    : m_Client(createWatchServiceClient(CWatchService::Modules::WatchServiceController)),
      m_LastCommand(Unknown), m_PreviousConnectionState(false) {
    connect(&m_Timer, &QTimer::timeout, this, &WatchServiceController::onCheck);

    m_Client->subscribeOnDisconnected(this);
    m_Client->subscribeOnCloseCommandReceived(this);

    m_Timer.setInterval(CWatchServiceController::CheckTimeout);
    m_Timer.start();

    // Create menu actions with direct connections instead of signal mapper
    {
        auto settingsAction = m_Menu.addAction(
            createTemplateIcon(":/icons/menu-settingsTemplate.png"), tr("#start_service_menu"));
        connect(settingsAction, SIGNAL(triggered(bool)), this, SLOT(onStartServiceMenuClicked()));
        m_StartServiceActions << settingsAction;

        auto setupAction = m_Menu.addAction(createTemplateIcon(":/icons/menu-setupTemplate.png"),
                                           tr("#start_first_setup"));
        connect(setupAction, SIGNAL(triggered(bool)), this, SLOT(onStartFirstSetupClicked()));
        m_StartServiceActions << setupAction;

        m_Menu.addSeparator();
    }

    auto playAction =
        m_Menu.addAction(createTemplateIcon(":/icons/menu-playTemplate.png"), tr("#start_service"));
    connect(playAction, SIGNAL(triggered(bool)), this, SLOT(onStartServiceClickedDirect()));
    m_StartServiceActions << playAction;

    m_StopServiceAction =
        m_Menu.addAction(createTemplateIcon(":/icons/menu-stopTemplate.png"), tr("#stop_service"));
    m_Menu.addSeparator();
    m_CloseTrayIconAction =
        m_Menu.addAction(createTemplateIcon(":/icons/menu-closeTemplate.png"), tr("#close"));

    connect(m_StopServiceAction, SIGNAL(triggered(bool)), SLOT(onStopServiceClicked()));
    connect(m_CloseTrayIconAction, SIGNAL(triggered(bool)), SLOT(onCloseIconClicked()));

    connect(&m_Icon,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    m_Icon.setContextMenu(&m_Menu);

    // Set initial icon based on connection state
    updateTrayIcon();

    m_Icon.show();

    LOG(getLog(), LogLevel::Normal, "WatchServiceController started.");
}

//----------------------------------------------------------------------------
// Helper to create template icons for macOS with multiple sizes for better scaling
QIcon WatchServiceController::createTemplateIcon(const QString &basePath) {
    QIcon icon;

    // Extract base name without extension and without resource prefix
    QString baseName = basePath;
    if (baseName.startsWith(":/")) {
        baseName = baseName.mid(2); // Remove :/ prefix
    }
    if (baseName.endsWith(".png")) {
        baseName = baseName.left(baseName.length() - 4); // Remove .png extension
    }

    // Load multiple sizes for better scaling on different DPI and contexts
    // Available sizes: 16, 32, 48, 64, 96, 128, 256, 512
    static const QList<int> sizes = {16, 32, 48, 64, 96, 128, 256, 512};
    QStringList loadedSizes;

    for (int size : sizes) {
        QString sizePath;
        if (size == 48) {
            // Default size - use the base path
            sizePath = basePath;
        } else {
            // Size-specific variants (ico sizes for smaller ones)
            if (size <= 48) {
                sizePath = QString(":/%1-ico-%2.png").arg(baseName).arg(size);
            } else {
                sizePath = QString(":/%1-%2.png").arg(baseName).arg(size);
            }
        }

        // Check if the size variant exists in resources by trying to load it
        QPixmap pixmap(sizePath);
        if (!pixmap.isNull()) {
            icon.addPixmap(pixmap);
            loadedSizes << QString("%1px").arg(size);
        }
    }

    // If no sizes were loaded, fall back to the original single-size approach
    if (icon.isNull()) {
        icon = QIcon(basePath);
    }

    icon.setIsMask(true); // Ensures the 'Template' behavior is activated for macOS
    return icon;
}

//----------------------------------------------------------------------------
// Helper to create app icons with multiple sizes for better scaling (non-template icons)
QIcon WatchServiceController::createAppIcon(const QString &basePath) {
    QIcon icon;

    // Extract base name without extension and without resource prefix
    QString baseName = basePath;
    if (baseName.startsWith(":/")) {
        baseName = baseName.mid(2); // Remove :/ prefix
    }
    if (baseName.endsWith(".png")) {
        baseName = baseName.left(baseName.length() - 4); // Remove .png extension
    }

    // Load multiple sizes for better scaling on different DPI and contexts
    // Available sizes: 16, 32, 48, 64, 96, 128, 256, 512
    static const QList<int> sizes = {16, 32, 48, 64, 96, 128, 256, 512};
    QStringList loadedSizes;

    for (int size : sizes) {
        QString sizePath;
        if (size == 48) {
            // Default size - use the base path
            sizePath = basePath;
        } else {
            // Size-specific variants (ico sizes for smaller ones)
            if (size <= 48) {
                sizePath = QString(":/%1-ico-%2.png").arg(baseName).arg(size);
            } else {
                sizePath = QString(":/%1-%2.png").arg(baseName).arg(size);
            }
        }

        // Check if the size variant exists in resources by trying to load it
        QPixmap pixmap(sizePath);
        if (!pixmap.isNull()) {
            icon.addPixmap(pixmap);
            loadedSizes << QString("%1px").arg(size);
        }
    }

    // If no sizes were loaded, fall back to the original single-size approach
    if (icon.isNull()) {
        icon = QIcon(basePath);
    }

    // Note: No setIsMask(true) for app icons - they should render normally
    return icon;
}

//----------------------------------------------------------------------------
// Helper to get platform-specific executable path
QString WatchServiceController::getExecutablePath(const QString &baseName) const {
    QString executableName = baseName;

    // Add platform-specific extension
#ifdef Q_OS_WIN
    executableName += ".exe";
    // Get working directory and construct full path
    QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();
    return QDir::cleanPath(
        QDir::toNativeSeparators(workingDir + QDir::separator() + executableName));
#elif defined(Q_OS_MAC)
    executableName += ".app/Contents/MacOS/" + baseName;
    // Get working directory and construct full path
    QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();
    return QDir::cleanPath(
        QDir::toNativeSeparators(workingDir + QDir::separator() + executableName));
#else
    // Get working directory and construct full path
    QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();
    return QDir::cleanPath(
        QDir::toNativeSeparators(workingDir + QDir::separator() + executableName));
#endif
}

//----------------------------------------------------------------------------
WatchServiceController::~WatchServiceController() {
    // Stop timer to prevent further processing
    m_Timer.stop();

    // Hide tray icon
    m_Icon.hide();

    // Disconnect all signals (Qt does this automatically, but explicit for clarity)
    disconnect(&m_Timer, &QTimer::timeout, this, &WatchServiceController::onCheck);
    disconnect(
        &m_Icon, &QSystemTrayIcon::activated, this, &WatchServiceController::onTrayIconActivated);
    disconnect(m_StopServiceAction,
               &QAction::triggered,
               this,
               &WatchServiceController::onStopServiceClicked);
    disconnect(m_CloseTrayIconAction,
               &QAction::triggered,
               this,
               &WatchServiceController::onCloseIconClicked);

    LOG(getLog(), LogLevel::Normal, "WatchServiceController stopped.");
}

//----------------------------------------------------------------------------
ILog *WatchServiceController::getLog() {
    if (BasicApplication::getInstance()) {
        return BasicApplication::getInstance()->getLog();
    }

    return nullptr;
}

//----------------------------------------------------------------------------
void WatchServiceController::onCheck() {
    bool wasConnected = m_Client->isConnected();

    if (!wasConnected) {
        m_LastCommand = Unknown;
        m_Client->start();
    }

    // Enable/disable start/stop service actions based on connection status
    bool isConnected = m_Client->isConnected();
    foreach (auto action, m_StartServiceActions) {
        action->setEnabled(!isConnected); // Enable start when not connected
    }
    m_StopServiceAction->setEnabled(isConnected); // Enable stop when connected
    // Keep close action enabled
    m_CloseTrayIconAction->setEnabled(true);

    // Only update tray icon when connection state actually changes to avoid layout recursion
    if (isConnected != m_PreviousConnectionState) {
        m_PreviousConnectionState = isConnected;
        LOG(getLog(),
            LogLevel::Normal,
            QString("Connection state changed: %1 -> %2").arg(wasConnected).arg(isConnected));

        // Use queued invocation to defer icon update and avoid layout conflicts
        QMetaObject::invokeMethod(this, "updateTrayIcon", Qt::QueuedConnection);
    }

    // Note: m_Icon.show() is called once in constructor, no need to call it repeatedly
}

//----------------------------------------------------------------------------
void WatchServiceController::updateTrayIcon() {
    if (m_Client->isConnected()) {
        // Connected state: show normal template icon
        m_Icon.setIcon(createTemplateIcon(":/icons/controller-monogram_Template.png"));
    } else {
        // Disconnected state: show slashed H icon to indicate stopped state
        m_Icon.setIcon(createTemplateIcon(":/icons/controller-monogram-stoppedTemplate.png"));
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onDisconnected() {
    onCheck();
}

//----------------------------------------------------------------------------
void WatchServiceController::onCloseCommandReceived() {
    if (m_LastCommand != Stop) {
        LOG(getLog(), LogLevel::Normal, "Close tray by command from watch service.");

        QCoreApplication::instance()->quit();
    } else {
        LOG(getLog(), LogLevel::Normal, "Ignore close command, because I initiate it.");
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onStartServiceClicked(const QString &aArguments) {
    LOG(getLog(), LogLevel::Normal, QString("User say: start service. %1").arg(aArguments));

    m_LastCommand = Start;

    if (!m_Client->isConnected()) {
        // Validate application instance
        if (!BasicApplication::getInstance()) {
            LOG(getLog(),
                LogLevel::Error,
                "Cannot start service: BasicApplication instance is null");
            return;
        }

        QString workingDir = BasicApplication::getInstance()->getWorkingDirectory();

        // Validate working directory
        if (!QDir(workingDir).exists()) {
            LOG(getLog(),
                LogLevel::Error,
                QString("Cannot start service: working directory does not exist: %1")
                    .arg(workingDir));
            return;
        }

        // Get platform-specific executable path
        QString path = getExecutablePath("watchdog");

        // Validate executable exists
        if (!QFile::exists(path)) {
            LOG(getLog(),
                LogLevel::Error,
                QString("Cannot start service: executable does not exist: %1").arg(path));
            return;
        }

        QStringList parameters;

        if (!aArguments.isEmpty()) {
            parameters << QString("-client_options=%1").arg(aArguments);
        }

        // Attempt to start the process
        bool started = QProcess::startDetached(path, parameters, workingDir);

        if (started) {
            LOG(getLog(),
                LogLevel::Normal,
                QString("Successfully started service process: %1").arg(path));
        } else {
            LOG(getLog(),
                LogLevel::Error,
                QString("Failed to start service process: %1").arg(path));
        }
    } else {
        m_Client->restartService(QStringList());
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onStopServiceClicked() {
    LOG(getLog(), LogLevel::Normal, "User say: stop service.");

    m_LastCommand = Stop;

    if (m_Client->isConnected()) {
        m_Client->stopService();
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onCloseIconClicked() {
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
    if (!iconPixmap.isNull()) {
        msgBox.setIconPixmap(iconPixmap);
    }
#endif

    msgBox.setWindowIcon(createAppIcon(":/icons/controller-app-icon.png"));

    int result = msgBox.exec();
    if (result == QMessageBox::Yes) {
        QCoreApplication::instance()->quit();
    }
}

//----------------------------------------------------------------------------
void WatchServiceController::onTrayIconActivated(QSystemTrayIcon::ActivationReason aReason) {
    onCheck();

    // Handle tray icon activation based on platform
#ifdef Q_OS_MAC
    // On macOS, the context menu is shown automatically by the system
    // when the tray icon has a context menu set, so we don't need to show it manually
    Q_UNUSED(aReason)
#else
    // On Windows/Linux, show context menu on left-click or right-click
    if (aReason == QSystemTrayIcon::Trigger || aReason == QSystemTrayIcon::Context) {
        m_Menu.popup(QCursor::pos());
        m_Menu.activateWindow();
    }
#endif
}

//----------------------------------------------------------------------------
// Direct connection slots for menu actions
void WatchServiceController::onStartServiceMenuClicked() {
    onStartServiceClicked("-start_scenario=service_menu");
}

void WatchServiceController::onStartFirstSetupClicked() {
    onStartServiceClicked("-start_scenario=first_setup");
}

void WatchServiceController::onStartServiceClickedDirect() {
    onStartServiceClicked("--disable-web-security");
}

//----------------------------------------------------------------------------
