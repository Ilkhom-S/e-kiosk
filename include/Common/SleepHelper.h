#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <Common/QtHeadersEnd.h>

class SleepHelper : public QThread {
  public:
    using QThread::msleep;
    using QThread::sleep;
    using QThread::usleep;
};

//---------------------------------------------------------------------------
