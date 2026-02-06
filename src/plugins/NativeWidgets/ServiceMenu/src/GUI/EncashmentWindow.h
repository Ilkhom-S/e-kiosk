/* @file Базовый виджет для инкассации */
#pragma once

#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

#include "EncashmentHistoryWindow.h"
#include "IServiceWindow.h"
#include "InputBox.h"

//---------------------------------------------------------------------------
class ServiceMenuBackend;

//---------------------------------------------------------------------------
class EncashmentWindow : public QWidget, public ServiceWindowBase {
    Q_OBJECT

public:
    EncashmentWindow(ServiceMenuBackend *aBackend, QWidget *aParent = 0);
    virtual ~EncashmentWindow();

public:
    virtual bool activate();
    virtual bool deactivate();

protected slots:
    void doEncashment();
    virtual bool doEncashmentProcess();
    void onPrintZReport();
    void onReceiptPrinted(qint64 aJobIndex, bool aErrorHappened);

protected:
    virtual void updateUI() = 0;

protected:
    QString mMessageSuccess;
    QString mMessageError;
    bool mEncashmentWithZReport;
    qint64 mLastPrintJob;

protected:
    QTimer mIdleTimer;
    InputBox *mInputBox;
    EncashmentHistoryWindow *mHistoryWindow;
};

//---------------------------------------------------------------------------
