/* @file Реализация шаблонных функций ProtocolUtils. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Common
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
template <class T> QString ProtocolUtils::toHexLog(T aData)
{
    int size = sizeof(T) * 2;

    return "0x" + QString("%1").arg(qulonglong(aData), size, 16, QChar(ASCII::Zero)).toUpper().right(size);
}

//--------------------------------------------------------------------------------
template <class T> T ProtocolUtils::clean(const T &aData)
{
    T result(aData);
    result.replace(ASCII::TAB, ASCII::Space);

    for (char ch = ASCII::NUL; ch < ASCII::Space; ++ch)
    {
        result.replace(ch, "");
    }

    result.replace(ASCII::DEL, ASCII::Space);

    int size = 0;

    do
    {
        size = result.size();
        result.replace("  ", " ");
    } while (size != result.size());

    int index = 0;

    while (result[index++] == ASCII::Space)
    {
    }
    result.remove(0, --index);

    index = result.size();

    while (index && (result[--index] == ASCII::Space))
    {
    }
    index++;
    result.remove(index, result.size() - index);

    return (result == " ") ? "" : result;
}

//--------------------------------------------------------------------------------
template <class T> T ProtocolUtils::revert(const T &aBuffer)
{
    T result(aBuffer);

    for (int i = 1; i < aBuffer.size(); ++i)
    {
        result.prepend(result[i]);
        result.remove(i + 1, 1);
    }

    return result;
}