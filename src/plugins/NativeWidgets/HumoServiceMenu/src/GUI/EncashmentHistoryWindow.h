/* @file Окошко для отображения истории инкассаций. */

#pragma once

#include <QtCore/QSignalMapper>

#include "ui_encashmentHistoryWindow.h"

class HumoServiceBackend;

//------------------------------------------------------------------------
class EncashmentHistoryWindow : public QWidget, protected Ui_EncashmentHistoryWindow {
    Q_OBJECT

public:
    EncashmentHistoryWindow(HumoServiceBackend *aBackend, QWidget *aParent);

    virtual ~EncashmentHistoryWindow();

public slots:
    void updateHistory();

private slots:
    void printEncashment(int aIndex);

protected:
    HumoServiceBackend *mBackend;
    QList<QWidget *> mWidgets;
    QSignalMapper *mSignalMapper;
};

//------------------------------------------------------------------------
