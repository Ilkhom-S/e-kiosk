/* @file Главное окно сервисного меню. */

#pragma once

#include <QtCore/QTimer>

#include "ui_MainServiceWindow.h"

class WizardWindow;
class IServiceWindow;
class ServiceMenuBackend;

//------------------------------------------------------------------------
class MainServiceWindow : public QWidget, public Ui::MainServiceWindow {
    Q_OBJECT

public:
    MainServiceWindow(ServiceMenuBackend *aBackend, QWidget *aParent = 0);

    bool initialize();
    void shutdown();

    QWidget *getMainWidget() { return wPasswordPage; }

    WizardWindow *getScreenByName(const QString &aScreenName);

    // Выход из сервисного меню
    bool closeServiceMenu(bool aExitByNotify, const QString &aMessage, bool aStartIdle = false);

private slots:
    // Активация/деактивация вкладок
    void onCurrentPageChanged(int aIndex);

    // Авторизация пользователя
    void onProceedLogin();

    void onCancelAuthorization();

    // Выход из сервисного меню
    void onCloseServiceMenu();

    // Перезагрузка приложения
    void onRebootApplication();

    // Останов приложения
    void onStopApplication();

    // Перезагрузка терминала
    void onRebootTerminal();

    // Блокирование/разблокирование терминала
    void onToggleLock();

    // Для обработки сигналов цифровой клавиатуры
    void onBackspaceClicked();
    void onClearClicked();
    void onDigitClicked(QAbstractButton *aButton);

    void onIdleTimeout();
    void onDateTimeRefresh();

    void onAbstractButtonClicked();

private:
    void applyConfiguration();
    bool applyAccessRights();
    void closeMenu(bool aStartIdle = false);

    void connectAllAbstractButtons(QWidget *aParentWidget);

private:
    int m_CurrentPageIndex;
    QTimer m_IdleTimer;
    QTimer m_DateTimeTimer;
    QVariantMap m_TerminalInfo;

    QList<IServiceWindow *> m_ServiceWindowList;
    ServiceMenuBackend *m_Backend;

    // Подавляем повторные запросы TryStopScenario, если оператор уже отказался в этой сессии
    bool m_stopNotifyDialogSuppressed{false};
};

//------------------------------------------------------------------------
