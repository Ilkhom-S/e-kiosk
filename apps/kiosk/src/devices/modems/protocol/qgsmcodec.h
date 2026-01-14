#pragma once
/*
  This file is part of the Qt Extended Opensource Package.
  Copyright (C) 2009 Trolltech ASA.
  Contact: Qt Extended Information (info@qtextended.org)

  This file may be used under the terms of the GNU General Public License
  version 2.0 as published by the Free Software Foundation (see LICENSE.GPL).
  Please review: https://www.fsf.org/licensing/licenses/info/GPLv2.html
*/

#include <QtCore/QString>
#include <QtCore/QTextCodec>

class QGsmCodec : public QTextCodec {
  public:
    explicit QGsmCodec(bool noLoss = false);
    ~QGsmCodec();

    QByteArray name() const;
    int mibEnum() const;

    static char singleFromUnicode(QChar ch);
    static QChar singleToUnicode(char ch);

    static unsigned short twoByteFromUnicode(QChar ch);
    static QChar twoByteToUnicode(unsigned short ch);

  protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;

  private:
    bool noLoss;
};
