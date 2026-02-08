/* @file Модуль управления сторожевым сервисом через сокет. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtWidgets/QMenu>
#include <QtWidgets/QSystemTrayIcon>

#include <Common/ILog.h>

#include <WatchServiceClient/IWatchServiceClient.h>

//----------------------------------------------------------------------------
class WatchServiceController : public QObject {
    Q_OBJECT

    // Последняя команда выполненная пользователем
    enum LastCommand { Unknown, Start, Stop };

public:
    WatchServiceController();
    ~WatchServiceController();

private:
    static ILog *getLog();

    // Helper to create template icons for macOS
    static QIcon createTemplateIcon(const QString &path);

    // Helper to create app icons with multiple sizes for better scaling
    static QIcon createAppIcon(const QString &path);

    // Helper to get platform-specific executable path
    static QString getExecutablePath(const QString &baseName);

private slots:
    // Попытка соединения со сторожевым сервисом.
    void onCheck();

    // Update tray icon asynchronously to avoid layout issues
    void updateTrayIcon();

    // Закрыто соединение со сторожевым сервисом.
    void onDisconnected();

    // Получение команды на закрытие
    void onCloseCommandReceived();

    void onTrayIconActivated(QSystemTrayIcon::ActivationReason aReason);

    void onStartServiceClicked(const QString &aArguments);
    void onStopServiceClicked();
    static void onCloseIconClicked();

    // Direct action slots
    void onStartServiceMenuClicked();
    void onStartFirstSetupClicked();
    void onStartServiceClickedDirect();

private:
    QSharedPointer<IWatchServiceClient> m_Client;

    LastCommand m_LastCommand;
    bool m_PreviousConnectionState; // Track previous connection state to avoid unnecessary UI
                                    // updates

    QTimer m_Timer;

    QSystemTrayIcon m_Icon;
    QMenu m_Menu;

    // Pre-created icons to avoid layout issues during updates
    QIcon m_ConnectedIcon;
    QIcon m_DisconnectedIcon;

    QList<QAction *> m_StartServiceActions;
    QAction *m_StopServiceAction;
    QAction *m_CloseTrayIconAction;

#ifdef Q_OS_MAC
    void setMacOSTrayIconAsTemplate();
#endif
};

//----------------------------------------------------------------------------
