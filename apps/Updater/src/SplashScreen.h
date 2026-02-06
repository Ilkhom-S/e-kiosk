/* @file Вспомогательный экран, закрывающий рабочий стол. */

#pragma once

#include <QtGui/QCloseEvent>
#include <QtWidgets/QWidget>

#include "ui_SplashScreen.h"

//----------------------------------------------------------------------------
/// Вспомогательный экран, закрывающий рабочий стол. Предоставляет доступ к сервисному меню.
class SplashScreen : public QWidget {
    Q_OBJECT

public:
    /// Конструктор.
    SplashScreen(QWidget *aParent = 0);

    /// Деструктор.
    virtual ~SplashScreen();

    /// Сворачивает окно вместо его закрытия.
    virtual void closeEvent(QCloseEvent *aEvent);

private:
    Ui::SplashScreenClass ui;
};

//----------------------------------------------------------------------------
