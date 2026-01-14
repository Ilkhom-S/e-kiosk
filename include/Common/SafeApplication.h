/* @file Классы приложений. */

#pragma once

#include <singleapplication.h>
#include <QtWidgets/QApplication>

//------------------------------------------------------------------------
/// Класс GUI приложения, обрабатывающий исключения в обработчике событий.
class SafeQApplication : public SingleApplication {
    Q_OBJECT

  public:
    /// Конструктор.
    SafeQApplication(int &aArgc, char **aArgv) : SingleApplication(aArgc, aArgv) {
    }

  public:
    /// Обработчик событий с обработкой исключений.
    virtual bool notify(QObject *aReceiver, QEvent *aEvent);
};
