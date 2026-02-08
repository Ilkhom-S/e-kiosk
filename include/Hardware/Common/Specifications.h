/* @file Классы для хранения [описателей] данных. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

//--------------------------------------------------------------------------------
// Базовый класс для хранения данных в виде пар ключ-значение
template <class T1, class T2> class CSpecification {
public:
    // Инициализация значения по умолчанию
    CSpecification(const T2 &aDefault = T2()) : m_DefaultValue(aDefault) {}

    virtual ~CSpecification() = default;

    // Константный оператор доступа. В 2026 году это стандарт для безопасного чтения.
    const T2 &operator[](const T1 &aKey) const { return value(aKey); }

    // Неконстантный оператор для записи.
    T2 &operator[](const T1 &aKey) { return m_Buffer[aKey]; }

    // Поиск ключа по значению (работает через итераторы мапы внутри Qt)
    T1 key(const T2 &aValue) const { return m_Buffer.key(aValue); }

    // В C++14/17 для константного метода правильно возвращать только константную ссылку.
    // Если объект не найден, возвращаем ссылку на m_DefaultValue.
    const T2 &value(const T1 &aKey) const {
        auto it = m_Buffer.constFind(aKey);
        if (it != m_Buffer.constEnd()) {
            return it.value();
        }
        return m_DefaultValue;
    }

    void append(const T1 &aKey, const T2 &aParameter) { m_Buffer.insert(aKey, aParameter); }

    // Возвращаем ссылку на данные для модификации
    QMap<T1, T2> &data() { return m_Buffer; }

    // В Qt 6 для константных данных лучше возвращать копию (из-за Implicit Sharing)
    // или константную ссылку. Копия дешевле, если мапа не меняется.
    const QMap<T1, T2> &constData() const { return m_Buffer; }

    void setDefault(const T2 &aDefaultValue) { m_DefaultValue = aDefaultValue; }

    // Исправлено: константный геттер возвращает константную ссылку.
    const T2 &getDefault() const { return m_DefaultValue; }

protected:
    QMap<T1, T2> m_Buffer;
    T2 m_DefaultValue; // Значение, возвращаемое при отсутствии ключа
};

//--------------------------------------------------------------------------------
// Класс для хранения произвольных описателей данных
template <class T> class CDescription : public CSpecification<T, QString> {
public:
    using CSpecification<T, QString>::m_Buffer;
    using CSpecification<T, QString>::m_DefaultValue;

    void append(const T &aKey, const char *aParameter) {
        m_Buffer.insert(aKey, QString::from_Utf8(aParameter));
    }
    void append(const T &aKey, const QString &aParameter) { m_Buffer.insert(aKey, aParameter); }
    void setDefault(const char *aDefaultValue) { m_DefaultValue = QString::from_Utf8(aDefaultValue); }
};

//--------------------------------------------------------------------------------
// Базовый класс для хранения данных в виде пар ключ-значение со статическим
// заполнением. При использовании для хранения статических сущностей следить за
// временем их создания.
template <class T1, class T2> class CStaticSpecification : public CSpecification<T1, T2> {
public:
    using CSpecification<T1, T2>::m_Buffer;

    CStaticSpecification() { m_Buffer = process(T1(), T2(), true); }

    static QMap<T1, T2> &process(const T1 &aKey, const T2 aValue, bool aControl = false) {
        static QMap<T1, T2> data;

        if (!aControl) {
            data.insert(aKey, aValue);
        }

        return data;
    }
};

//--------------------------------------------------------------------------------
// Класс для хранения произвольных описателей данных в виде битовой маски
template <class T> class CBitmapDescription : public CDescription<T> {
public:
    using CDescription<T>::data;

    virtual QString getValues(T aValue) {
        QStringList result;

        for (auto it = data().begin(); it != data().end(); ++it) {
            if (it.key() & aValue) {
                result << it.value();
            }
        }

        return result.join(", ");
    }

protected:
    void addBit(int aBit, const char *aParameter) {
        append(T(1) << aBit, QString::from_Utf8(aParameter));
    }
};

//--------------------------------------------------------------------------------
#define APPEND(aKey) append(aKey, #aKey)
#define ADD(aKey) add(aKey, #aKey)

//--------------------------------------------------------------------------------
// Базовый класс для хранения данных в виде пар ключ-значение со статическим
// заполнением данных
template <class T1, class T2> class CSSpecification : public CSpecification<T1, T2> {
public:
    using CSpecification<T1, T2>::m_Buffer;

    CSSpecification() { m_Buffer = addData(T1(), T2(), true); }

    static QMap<T1, T2> addData(const T1 &aKey, const T2 &aValue, bool aMode = false) {
        static QMap<T1, T2> data;

        if (aMode) {
            return data;
        }

        data.insert(aKey, aValue);

        return QMap<T1, T2>();
    }
};

#define ADD_STATIC_DATA(aClass, T1, aName, aKey, aValue)                                           \
    const T1 aName = []() -> T1 {                                                                  \
        aClass::addData(aKey, aValue);                                                             \
        return aKey;                                                                               \
    }();

//---------------------------------------------------------------------------
