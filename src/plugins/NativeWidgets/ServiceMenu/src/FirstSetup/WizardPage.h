/* @file Окно визарда. */

#pragma once

#include <QtWidgets/QFrame>

#include "GUI/IServiceWindow.h"

//------------------------------------------------------------------------
class WizardPageBase : public QFrame, public ServiceWindowBase {
    Q_OBJECT

public:
    WizardPageBase(ServiceMenuBackend *aBackend, QWidget *aParent = 0);

signals:
    void pageEvent(const QString &aContext, bool aFlag);
};

//------------------------------------------------------------------------
