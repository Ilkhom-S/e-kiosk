/* @file Декорация визарда для локального соединения. */

#pragma once

#include <QtCore/QFutureWatcher>
#include <QtCore/QVariantMap>
#include <QtNetwork/QNetworkProxy>

#include "WizardPage.h"

class UnmanagedConnectionWindow;

//---------------------------------------------------------------------------
class UnmanagedWizardPage : public WizardPageBase {
    Q_OBJECT

public:
    UnmanagedWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();

private slots:
    void onUserSelectionChanged();
    void onTestConnection(QNetworkProxy aProxy);
    void onTestFinished();

private:
    QVariantMap m_ConnectionSettings;
    UnmanagedConnectionWindow *m_ConnectionWindow;
    QFutureWatcher<bool> m_TaskWatcher;
    QVariantMap m_StaticParameters;
};

//---------------------------------------------------------------------------
