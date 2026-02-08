/* @file Верификатор данных по алгоритму MD5. */

#include "HashVerifier.h"

#include <QtCore/QCryptographicHash>

Md5Verifier::Md5Verifier(const QString &aMd5) {
    m_MD5 = aMd5;
}

//------------------------------------------------------------------------
bool Md5Verifier::verify(NetworkTask * /*aTask*/, const QByteArray &aData) {
    m_CalculatedMD5 = QCryptographicHash::hash(aData, QCryptographicHash::Md5).toHex();

    return (m_MD5.compare(m_CalculatedMD5, Qt::CaseInsensitive) == 0);
}

//------------------------------------------------------------------------
Sha256Verifier::Sha256Verifier(const QString &aSha256) {
    m_Sha256 = aSha256;
}

//------------------------------------------------------------------------
bool Sha256Verifier::verify(NetworkTask * /*aTask*/, const QByteArray &aData) {
#if QT_VERSION >= 0x050000
    m_CalculatedSha256 = QCryptographicHash::hash(aData, QCryptographicHash::Sha256).toHex();
#else
    m_CalculatedSha256 = CCryptographicHash::hash(aData, CCryptographicHash::Sha256).toHex();
#endif

    return (m_Sha256.compare(m_CalculatedSha256, Qt::CaseInsensitive) == 0);
}
//------------------------------------------------------------------------
