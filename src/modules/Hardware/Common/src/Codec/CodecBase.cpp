/* @file Базовый кодек для создания кастомных кодеков. */

#include "CodecBase.h"

#include <QtCore/QDebug>

#include "Hardware/Common/ASCII.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"

//---------------------------------------------------------------------------
CodecBase::CodecBase()
    : m_MIB(3000), m_MinValueActive(0x0080), m_Name("Base"),
      m_DataGuard(QReadWriteLock::Recursive) {}

//---------------------------------------------------------------------------
QByteArray CodecBase::name() const {
    return m_Name;
}

//---------------------------------------------------------------------------
QList<QByteArray> CodecBase::aliases() const {
    return QList<QByteArray>() << m_Name;
}

//---------------------------------------------------------------------------
int CodecBase::mibEnum() const {
    return m_MIB;
}

//---------------------------------------------------------------------------
QString CodecBase::convertToUnicode(const char *aBuffer, int aLength) const {
    QReadLocker lock(&m_DataGuard);

    QByteArray buffer = QByteArray::fromRawData(aBuffer, aLength);
    auto &characterData = const_cast<CharacterData &>(m_Data);
    QByteArray defaultBuffer = characterData.getDefault().character.toLatin1();
    bool dataEmpty = characterData.data().isEmpty();

    auto replace = [&](char ch) {
        if (uchar(ch) < uchar(m_MinValueActive)) {
            dataEmpty ? buffer.replace(ch, defaultBuffer[0]) : buffer.replace(ch, "");
        }
    };

    for (char ch = 0x00; ch < ASCII::Space; ++ch) {
        replace(ch);
    }

    replace(ASCII::DEL);

    QString result;

    for (char i : buffer) {
        if ((i == 0) && (m_MinValueActive != 0u)) {
            break;
        }

        if (uchar(i) < uchar(m_MinValueActive)) {
            result += i;
        } else {
            result += m_Data[i].character;
        }
    }

    return result;
}

//---------------------------------------------------------------------------
QByteArray CodecBase::convertFrom_Unicode(const QChar *aBuffer, int aLength) const {
    QReadLocker lock(&m_DataGuard);

    QByteArray result;
    auto &characterData = const_cast<CharacterData &>(m_Data);
    QString data(aBuffer, aLength);

    for (int i = 0; i < aLength; ++i) {
        ushort unicode = aBuffer[i].unicode();
        char character = char(unicode);

        if (unicode > m_MinValueActive) {
            QList<char> characters = characterData.data().keys(SCharData(data[i], true));

            if (characters.isEmpty()) {
                characters = characterData.data().keys(SCharData(data[i], false));
            }

            QString log = QString(" for unicode character \"%1\" == %2 (%3)")
                              .arg(data[i])
                              .arg(unicode)
                              .arg(ProtocolUtils::toHexLog(unicode));

            if (characters.isEmpty()) {
                character = CCodec::DefaultCharacter;
                qDebug() << "No data" + log;
            } else {
                if (characters.size() > 1) {
                    QStringList logData;

                    foreach (char ch, characters) {
                        logData << ProtocolUtils::toHexLog(ch);
                    }

                    qDebug() << "There are a lot of values " + logData.join(", ") + log;
                }

                character = characters[0];
            }
        }

        result.append(character);
    }

    return result;
}

//---------------------------------------------------------------------------
