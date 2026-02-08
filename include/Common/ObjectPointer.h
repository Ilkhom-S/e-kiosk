/* @file Умный указатель на произвольный интерфейс объекта, наследуемого от
 * QObject. */

#pragma once

#include <QtCore/QPointer>

//--------------------------------------------------------------------------------
template <typename I> class ObjectPointer {
    QPointer<QObject> m_Ptr;

public:
    ObjectPointer() {}
    ObjectPointer(I *aPtr) : m_Ptr(dynamic_cast<QObject *>(aPtr)) {}

    ObjectPointer<I> &operator=(I *aPtr) {
        m_Ptr = dynamic_cast<QObject *>(aPtr);

        return *this;
    }

    operator bool() const { return m_Ptr && dynamic_cast<I *>(m_Ptr.data()) != nullptr; }

    operator I *() const { return data(); }

    I *operator->() const { return m_Ptr ? dynamic_cast<I *>(m_Ptr.data()) : nullptr; }

    I *data() const { return m_Ptr ? dynamic_cast<I *>(m_Ptr.data()) : nullptr; }
};

//--------------------------------------------------------------------------------
