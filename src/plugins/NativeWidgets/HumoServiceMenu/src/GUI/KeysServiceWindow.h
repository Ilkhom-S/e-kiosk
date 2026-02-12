/* @file Окно настроек. */

#pragma once

#include "IServiceWindow.h"
#include "KeysWindow.h"
#include "ui_KeysServiceWindow.h"

//------------------------------------------------------------------------
class KeysServiceWindow : public QFrame, public ServiceWindowBase, protected Ui::KeysServiceWindow {
    Q_OBJECT

public:
    KeysServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();
    virtual bool activate();
    virtual bool deactivate();

private slots:
    void onBeginGenerating();
    void onEndGenerating();

    void onError(QString aError);

private:
    KeysWindow *m_Window{};
};

//------------------------------------------------------------------------
