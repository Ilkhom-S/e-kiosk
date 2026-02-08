/* @file Верификатор данных по алгоритму MD5. */

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QString>

#include "IVerifier.h"

//------------------------------------------------------------------------
namespace CHashVerifier {
const int MD5HashSize = 32;
const int Sha256HashSize = 64;
} // namespace CHashVerifier

class IHashVerifier : public IVerifier {
public:
    virtual QString referenceHash() const = 0;
    virtual QString calculatedHash() const = 0;
};

//------------------------------------------------------------------------
class Md5Verifier : public IHashVerifier {
public:
    Md5Verifier(const QString &aMD5);

    virtual bool verify(NetworkTask *aTask, const QByteArray &aData);

    QString referenceHash() const { return m_MD5; }
    QString calculatedHash() const { return m_CalculatedMD5; }

private:
    QString m_MD5;
    QString m_CalculatedMD5;
};

//------------------------------------------------------------------------
class Sha256Verifier : public IHashVerifier {
public:
    Sha256Verifier(const QString &aSha256);

    virtual bool verify(NetworkTask *aTask, const QByteArray &aData);

    QString referenceHash() const { return m_Sha256; }
    QString calculatedHash() const { return m_CalculatedSha256; }

private:
    QString m_Sha256;
    QString m_CalculatedSha256;
};

//------------------------------------------------------------------------
