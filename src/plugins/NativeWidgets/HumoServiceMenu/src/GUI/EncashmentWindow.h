/* @file Базовый виджет для инкассации */
#pragma once

#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

#include "EncashmentHistoryWindow.h"
#include "IServiceWindow.h"
#include "InputBox.h"

//---------------------------------------------------------------------------
class HumoServiceBackend;

//---------------------------------------------------------------------------
class EncashmentWindow : public QWidget, public ServiceWindowBase {
    Q_OBJECT

public:
    EncashmentWindow(HumoServiceBackend *aBackend, QWidget *aParent = 0);
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
    QString m_MessageSuccess;
    QString m_MessageError;
    bool m_EncashmentWithZReport;
    qint64 m_LastPrintJob;

protected:
    QTimer m_IdleTimer;
    InputBox *m_InputBox;
    EncashmentHistoryWindow *m_HistoryWindow;
};

//---------------------------------------------------------------------------
