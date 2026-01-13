#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

class ISysUtils {
 public:
  static QString rmBOM(const QString &s) {
    if (s.isEmpty())
      return s;

    QString res = s;
    // Remove Unicode BOM if present
    if (!res.isEmpty() && res.at(0) == QChar(0xFEFF)) {
      res.remove(0, 1);
      return res;
    }

    // Also handle UTF-8 BOM as 3 byte sequence in QString's utf8 representation
    QByteArray a = res.toUtf8();
    static const QByteArray utf8bom("\xEF\xBB\xBF");
    if (a.startsWith(utf8bom)) {
      a.remove(0, utf8bom.size());
      return QString::fromUtf8(a);
    }

    return res;
  }
};
