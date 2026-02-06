/* @file Классы приложений. */

#pragma once

#include <QtWidgets/QApplication>

#include <singleapplication.h>

//------------------------------------------------------------------------
/// Класс GUI приложения, обрабатывающий исключения в обработчике событий.
class SafeQApplication : public SingleApplication {
    Q_OBJECT

public:
    /// Конструктор.
    SafeQApplication(int &aArgc,
                     char **aArgv,
                     bool allowSecondary = false,
                     SingleApplication::Options options = SingleApplication::Mode::User,
                     int timeout = 1000,
                     const QString &userData = {})
        : SingleApplication(aArgc, aArgv, allowSecondary, options, timeout, userData) {}

public:
    /// Обработчик событий с обработкой исключений.
    virtual bool notify(QObject *aReceiver, QEvent *aEvent);
};
