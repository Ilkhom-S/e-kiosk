/* @file Главное окно HumoServiceMenu. */

#pragma once

#include <QtCore/QTimer>

#include "ui_MainHumoServiceMenuWindow.h"

class WizardWindow;
class IServiceWindow;
class HumoServiceBackend;

//------------------------------------------------------------------------
class MainHumoServiceMenuWindow : public QWidget, public Ui::MainHumoServiceMenuWindow {
    Q_OBJECT

public:
    MainHumoServiceMenuWindow(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    bool initialize();
    void shutdown();

    QWidget *getMainWidget() { return wPasswordPage; }

    WizardWindow *getScreenByName(const QString &aScreenName);

    // Выход из HumoService меню
    bool closeHumoServiceMenu(bool aExitByNotify, const QString &aMessage, bool aStartIdle = false);

private slots:
    // Активация/деактивация вкладок
    void onCurrentPageChanged(int aIndex);

    // Авторизация пользователя
    void onProceedLogin();

    void onCancelAuthorization();

    // Выход из HumoService меню
    void onCloseHumoServiceMenu();

    // Останов приложения
    void onStopApplication();

    // Перезагрузка приложения
    void onRebootApplication();

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
    HumoServiceBackend *m_Backend;
    QButtonGroup *digitGroup;
};

//------------------------------------------------------------------------