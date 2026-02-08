/* @file Базовый кодек для создания кастомных кодеков. */

#pragma once

#include <QtCore/QReadWriteLock>
#include <QtCore/QStringConverter>

#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/Specifications.h"

//---------------------------------------------------------------------------
namespace CCodec {
/// Символ по умолчанию для неизвестных символов.
const char DefaultCharacter = '?';
} // namespace CCodec

struct SCharData {
    QString character; /// UTF-8-cимвол по коду.
    bool main;         /// Признак основного символа при обратной перекодировке.

    SCharData() : character(QChar(CCodec::DefaultCharacter)), main(false) {}
    SCharData(const QString &aCharacter, bool aMain) : character(aCharacter), main(aMain) {}
    SCharData(const char *aCharacter, bool aMain)
        : character(QString::fromUtf8(aCharacter)), main(aMain) {}
    SCharData(const QChar &aCharacter, bool aMain) : character(aCharacter), main(aMain) {}

    bool operator==(const SCharData &aData) const {
        return (character == aData.character) && (main == aData.main);
    }
};

//---------------------------------------------------------------------------
class CharacterData : public CSpecification<char, SCharData> {
public:
    CharacterData() { setDefault(SCharData()); }

    void add(char aCode, const QString &aCharacter, bool aMain = true) {
        append(aCode, SCharData(aCharacter, aMain));
    }

    void add(char aCode, const char *aCharacter, bool aMain = true) {
        append(aCode, SCharData(aCharacter, aMain));
    }

    void add(char aCode) { append(aCode, m_DefaultValue); }
};

//---------------------------------------------------------------------------
class CodecBase {
public:
    CodecBase();

    /// Получить название.
    virtual QByteArray name() const;

    /// Получить список алиасов.
    virtual QList<QByteArray> aliases() const;

    /// Получить Id.
    virtual int mibEnum() const;

    /// Конвертировать массив байтов в юникодовую строку.
    virtual QString convertToUnicode(const char *aBuffer, int aLength) const;

    /// Конвертировать юникодовую строку в массив байтов.
    virtual QByteArray convertFromUnicode(const QChar *aBuffer, int aLength) const;

protected:
    /// Имя кодека.
    QByteArray m_Name;

    /// Id кодека >= 3000. http://www.iana.org/assignments/character-sets/character-sets.xhtml
    int m_MIB;

    /// Массив данных для перекодировки.
    CharacterData m_Data;

    /// Сторож данных.
    mutable QReadWriteLock m_DataGuard;

    /// Минимальное значение unicode-символа для использования кодека для перекодировки.
    ushort m_MinValueActive;
};

//---------------------------------------------------------------------------
