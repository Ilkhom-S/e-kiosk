/* @file Помощник для сна потока. */

#pragma once

//--------------------------------------------------------------------------------
// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
class SleepHelper : public QThread
{
  public:
    /// Использование статических методов сна из QThread.
    using QThread::msleep;
    using QThread::sleep;
    using QThread::usleep;
};

//---------------------------------------------------------------------------
