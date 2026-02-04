/* @file Модуль управления сторожевым сервисом через сокет. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtWidgets/QMenu>
#include <QtWidgets/QSystemTrayIcon>
#include <Common/QtHeadersEnd.h>

// Modules
#include <WatchServiceClient/IWatchServiceClient.h>
#include <Common/ILog.h>

//----------------------------------------------------------------------------
class WatchServiceController : public QObject
{
    Q_OBJECT

    // Последняя команда выполненная пользователем
    enum LastCommand
    {
        Unknown,
        Start,
        Stop
    };

  public:
    WatchServiceController();
    ~WatchServiceController();

  private:
    ILog *getLog();

    // Helper to create template icons for macOS
    QIcon createTemplateIcon(const QString &path);

    // Helper to create app icons with multiple sizes for better scaling
    QIcon createAppIcon(const QString &path);

    // Helper to get platform-specific executable path
    QString getExecutablePath(const QString &baseName) const;

  private slots:
    // Попытка соединения со сторожевым сервисом.
    void onCheck();

    // Закрыто соединение со сторожевым сервисом.
    void onDisconnected();

    // Получение команды на закрытие
    void onCloseCommandReceived();

    void onTrayIconActivated(QSystemTrayIcon::ActivationReason aReason);

    void onStartServiceClicked(const QString &aArguments);
    void onStopServiceClicked();
    void onCloseIconClicked();

    // Direct action slots
    void onStartServiceMenuClicked();
    void onStartFirstSetupClicked();
    void onStartServiceClickedDirect();

  private:
    QSharedPointer<IWatchServiceClient> mClient;

    LastCommand mLastCommand;

    QTimer mTimer;

    QSystemTrayIcon mIcon;
    QMenu mMenu;

    QList<QAction *> mStartServiceActions;
    QAction *mStopServiceAction;
    QAction *mCloseTrayIconAction;

#ifdef Q_OS_MAC
    void setMacOSTrayIconAsTemplate();
#endif
};

//----------------------------------------------------------------------------
