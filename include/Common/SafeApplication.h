/* @file Классы приложений. */

#pragma once

#include <singleapplication.h>

//------------------------------------------------------------------------
/// Класс GUI приложения, обрабатывающий исключения в обработчике событий.
class SafeQApplication : public SingleApplication {
  Q_OBJECT

public:
  SafeQApplication(int &aArgc, char **aArgv)
      : SingleApplication(aArgc, aArgv) {}

public:
  virtual bool notify(QObject *aReceiver, QEvent *aEvent);
};
