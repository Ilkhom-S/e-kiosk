#include "utils.h"

QString encodeStr(const QString &str, quint32 key) {
    QByteArray arr(str.toUtf8());
    for (char &c : arr) {
        c = c ^ static_cast<char>(key & 0xFF);
    }

    return QString::fromLatin1(arr.toBase64());
}

QString decodeStr(const QString &str, quint32 key) {
    QByteArray arr = QByteArray::fromBase64(str.toLatin1());
    for (char &c : arr) {
        c = c ^ static_cast<char>(key & 0xFF);
    }

    return QString::fromUtf8(arr);
}
