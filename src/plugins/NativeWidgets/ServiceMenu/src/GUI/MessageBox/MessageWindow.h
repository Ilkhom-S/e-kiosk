#pragma once

#include <QtCore/QPointer>
#include <QtWidgets/QDialog>

#include <SDK/GUI/MessageBoxParams.h>

#include "ui_MessageWindow.h"

class MessageWindow : public QDialog {
    Q_OBJECT

public:
    MessageWindow(QWidget *parent = 0);
    ~MessageWindow();

public:
    void setup(const QString &aText,
               SDK::GUI::MessageBoxParams::Enum aIcon,
               SDK::GUI::MessageBoxParams::Enum aButton);

private slots:
    void onClickedOk();
    void onClickedReject();

private:
    virtual void showEvent(QShowEvent *aEvent) override;
    virtual void hideEvent(QHideEvent *aEvent) override;

private:
    Ui::MessageWindow ui;
};
