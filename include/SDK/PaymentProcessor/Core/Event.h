/* @file Описание системного события. */

#pragma once

#include <QtCore/QVariant>

#include <SDK/PaymentProcessor/Core/EventTypes.h>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
/// Системное событие, имеющее тип, отправителя и данные. Для передачи сложных типов данных
/// требуется унаследоваться от класса EventData. Объект забирается внутрь, память освобождается
/// самим классом Event.
class Event {
public:
    Event() : m_Type(-1), m_Data() {}
    Event(int aType, const QString &aSender = "") : m_Type(aType), m_Sender(aSender) {}
    Event(int aType, const QString &aSender, const QVariant &aData)
        : m_Type(aType), m_Sender(aSender), m_Data(aData) {}

    virtual ~Event() {}

    /// Возвращает тип события.
    inline int getType() const { return m_Type; }

    /// Возвращает отправителя события.
    inline QString getSender() const { return m_Sender; }

    /// Возвращает true, если событие имеет данные.
    inline bool hasData() const { return !m_Data.isNull(); }

    /// Возвращает данные события.
    inline const QVariant &getData() const { return m_Data; }

private:
    int m_Type;
    QString m_Sender;
    QVariant m_Data;
};

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
