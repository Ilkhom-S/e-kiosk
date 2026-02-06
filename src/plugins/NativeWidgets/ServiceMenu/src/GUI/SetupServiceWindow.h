/* @file Окно настроек. */

#pragma once

#include "IServiceWindow.h"
#include "ui_SetupServiceWindow.h"

//------------------------------------------------------------------------
class SetupServiceWindow : public QFrame, public ServiceWindowBase, public Ui::SetupServiceWindow {
    Q_OBJECT

public:
    SetupServiceWindow(ServiceMenuBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();
    virtual bool activate();
    virtual bool deactivate();

protected slots:
    // Активация/деактивация вкладок
    void onCurrentPageChanged(int aIndex);

private:
    int mCurrentPageIndex;
};

//------------------------------------------------------------------------
