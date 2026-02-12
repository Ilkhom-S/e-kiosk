/* @file Окошко для отображения истории инкассаций. */

#pragma once

#include <QtCore/QSignalMapper>

#include "ui_encashmentHistoryWindow.h"

class ServiceMenuBackend;

//------------------------------------------------------------------------
class EncashmentHistoryWindow : public QWidget, protected Ui_EncashmentHistoryWindow {
    Q_OBJECT

public:
    EncashmentHistoryWindow(ServiceMenuBackend *aBackend, QWidget *aParent);

    virtual ~EncashmentHistoryWindow();

public slots:
    void updateHistory();

private slots:
    void printEncashment(int aIndex);

protected:
    ServiceMenuBackend *m_Backend;
    QList<QWidget *> m_Widgets;
    QSignalMapper *m_SignalMapper{};
};

//------------------------------------------------------------------------
