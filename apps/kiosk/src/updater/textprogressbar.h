#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

class TextProgressBar {
  public:
    TextProgressBar();

    void clear();
    void update();
    void setMessage(const QString &message);
    void setStatus(qint64 value, qint64 maximum);

  private:
    QString message;
    qint64 value;
    qint64 maximum;
    int iteration;
};

