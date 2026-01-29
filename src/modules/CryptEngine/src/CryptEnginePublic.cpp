/* @file Получение пароля для секретных ключей терминала (rootp, roots). */

// System
#include <Crypt/CryptEngine.h>

QList<QByteArray> CryptEngine::getRootPassword() const
{
    QList<QByteArray> result;

    result << "This is my secret password";

    return result;
}

//---------------------------------------------------------------------------
