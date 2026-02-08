#pragma once

#include <QtCore/QCryptographicHash>
#include <QtCore/QObject>
#include <QtGui/QValidator>

namespace {
const int Md5ValidatorMinLength = 5;
} // namespace

//------------------------------------------------------------------------------
class Md5Validator : public QValidator {
    Q_OBJECT
    Q_PROPERTY(QString hash READ getHash WRITE setHash)

public:
    Md5Validator(QObject *aParent = nullptr) : QValidator(aParent) {}

public:
    virtual QValidator::State validate(QString &aInput, int &aPos) const {
        Q_UNUSED(aPos)

        if (m_Hash.isEmpty()) {
            return QValidator::Intermediate;
        }

        if (aInput.length() < Md5ValidatorMinLength) {
            return QValidator::Intermediate;
        }

        return QCryptographicHash::hash(aInput.toLatin1(), QCryptographicHash::Md5).toHex() == m_Hash
                   ? QValidator::Acceptable
                   : QValidator::Intermediate;
    }

private slots:
    QString getHash() const { return m_Hash; }

    void setHash(const QString &aHash) { m_Hash = aHash.toLower(); }

private:
    QString m_Hash;
};