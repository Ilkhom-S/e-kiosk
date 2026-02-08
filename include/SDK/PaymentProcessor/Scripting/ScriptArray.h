/* @file Прокси класс для списков, контролирующих время жизни своих элементов.
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QObjectList>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
/// Прокси класс для списков, контролирующих время жизни своих элементов.
class ScriptArray : public QObject {
    Q_OBJECT

    /// Контейнер.
    Q_PROPERTY(QObjectList values READ getValues)

    /// Размер контейнера (кол-во элементов).
    Q_PROPERTY(int length READ getLength)

public:
    /// Конструктор.
    ScriptArray(QObject *aParent) : QObject(aParent) {}

    /// Добавить объект.
    ScriptArray &append(QObject *aObject) {
        m_Container.append(aObject);
        return *this;
    }

public slots:
    /// Проверка контейнера на пустоту.
    bool isEmpty() const { return m_Container.isEmpty(); }

private:
    /// Получить значения.
    QObjectList getValues() const { return m_Container; }
    /// Получить длину.
    int getLength() const { return m_Container.size(); }

private:
    /// Контейнер объектов.
    QObjectList m_Container;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
