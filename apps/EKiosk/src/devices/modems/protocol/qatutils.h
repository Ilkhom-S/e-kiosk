#pragma once
/*
  This file is part of the Qt Extended Opensource Package.
  Copyright (C) 2009 Trolltech ASA.
  Contact: Qt Extended Information (info@qtextended.org)

  This file may be used under the terms of the GNU General Public License
  version 2.0 as published by the Free Software Foundation (see LICENSE.GPL).
  Please review: https://www.fsf.org/licensing/licenses/info/GPLv2.html
*/

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>

class QAtResultParser;
class QTextCodec;

class QAtUtils {
private:
    QAtUtils() {}

public:
    static QString quote(const QString &str);
    static QString quote(const QString &str, QTextCodec *codec);
    static QString decode(const QString &str, QTextCodec *codec);
    static QTextCodec *codec(const QString &gsm_Charset);
    static QString toHex(const QByteArray &binary);
    static QByteArray from_Hex(const QString &hex);
    static QString decodeNumber(const QString &value, uint type);
    static QString decodeNumber(QAtResultParser &parser);
    static QString encodeNumber(const QString &value, bool keepPlus = false);
    static QString nextString(const QString &buf, uint &posn);
    static uint parseNumber(const QString &str, uint &posn);
    static void skipField(const QString &str, uint &posn);
    static QString stripNumber(const QString &number);
    static bool octalEscapes();
    static void setOctalEscapes(bool value);
    static QString decodeString(const QString &value, uint dcs);
};
