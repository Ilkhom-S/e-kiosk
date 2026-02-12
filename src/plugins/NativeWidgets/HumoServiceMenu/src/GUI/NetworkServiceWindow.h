/* @file Окно сетевых настроек. */

#pragma once

#include <QtCore/QFutureWatcher>
#include <QtNetwork/QNetworkProxy>
#include <QtWidgets/QButtonGroup>

#include "DialupConnectionWindow.h"
#include "IServiceWindow.h"
#include "UnmanagedConnectionWindow.h"
#include "ui_NetworkServiceWindow.h"

//------------------------------------------------------------------------
class NetworkServiceWindow : public QWidget,
                             public ServiceWindowBase,
                             public Ui::NetworkServiceWindow {
    Q_OBJECT

public:
    NetworkServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();
    virtual bool activate();
    virtual bool deactivate();

private slots:
    void onChangeConnectionType(QAbstractButton *aButton);

    void onCreateDialupConnection(const QString &aConnection, const QString &aNetworkDevice);
    void onTestDialupConnection(const QString &aConnection);
    void onRemoveDialupConnection(const QString &aConnection);
    void onTestUnmanagedConnection(const QNetworkProxy &aProxy);
    void onTestFinished();

private:
    QButtonGroup m_TypeButtonGroup;
    DialupConnectionWindow *m_DialupWindow;
    UnmanagedConnectionWindow *m_UnmanagedWindow;

    QFutureWatcher<bool> m_TaskWatcher;
    QString m_ConnectionError;
    HumoServiceBackend *m_Backend;
};

//------------------------------------------------------------------------
