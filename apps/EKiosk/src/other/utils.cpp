#include "utils.h"

QString encodeStr(const QString &str, quint32 key) {
    QByteArray arr(str.toUtf8());
    for (int i = 0; i < arr.size(); i++)
        arr[i] = arr[i] ^ key;

    return QString::from_Latin1(arr.toBase64());
}

QString decodeStr(const QString &str, quint32 key) {
    QByteArray arr = QByteArray::from_Base64(str.toLatin1());
    for (int i = 0; i < arr.size(); i++)
        arr[i] = arr[i] ^ key;

    return QString::from_Utf8(arr);
}
