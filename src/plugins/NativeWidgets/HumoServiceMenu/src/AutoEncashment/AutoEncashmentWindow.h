/* @file Виджет авто инкассации */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QSharedPointer>
#include "ui_AutoEncashmentWindow.h"
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

// Project
#include "GUI/EncashmentWindow.h"

class HumoServiceBackend;
class InputBox;

//---------------------------------------------------------------------------
class AutoEncashmentWindow : public EncashmentWindow
{
    Q_OBJECT

  public:
    AutoEncashmentWindow(HumoServiceBackend *aBackend, QWidget *aParent = 0);
    ~AutoEncashmentWindow();

  public:
    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();

  private:
    virtual void updateUI();

  private slots:
    void onTestPrinter();
    void onEncashment();
    void onEncashmentAndZReport();
    void onEnterServiceMenu();
    void onShowHistory();
    void onExit();
    void onIdleTimeout();

    void onBack();
    void onPanelChanged(int aIndex);

    void onDateTimeRefresh();
    void onDeviceStatusChanged(const QString &aConfigName, const QString &aStatusString, const QString &aStatusColor,
                               SDK::Driver::EWarningLevel::Enum aLevel);

  private:
    Ui::AutoEncashmentWindow ui;

  private:
    QTimer mDateTimeTimer;
    QVariantMap mTerminalInfo;
};

//---------------------------------------------------------------------------
