/* @file Прокси класс для списков, контролирующих время жизни своих элементов.
 */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QObjectList>
#include <Common/QtHeadersEnd.h>

namespace SDK
{
    namespace PaymentProcessor
    {
        namespace Scripting
        {

            //------------------------------------------------------------------------------
            /// Прокси класс для списков, контролирующих время жизни своих элементов.
            class ScriptArray : public QObject
            {
                Q_OBJECT

                /// Контейнер.
                Q_PROPERTY(QObjectList values READ getValues)

                /// Размер контейнера (кол-во элементов).
                Q_PROPERTY(int length READ getLength)

              public:
                /// Конструктор.
                ScriptArray(QObject *aParent) : QObject(aParent)
                {
                }

                /// Добавить объект.
                ScriptArray &append(QObject *aObject)
                {
                    mContainer.append(aObject);
                    return *this;
                }

              public slots:
                /// Проверка контейнера на пустоту.
                bool isEmpty() const
                {
                    return mContainer.isEmpty();
                }

              private:
                /// Получить значения.
                QObjectList getValues() const
                {
                    return mContainer;
                }
                /// Получить длину.
                int getLength() const
                {
                    return mContainer.size();
                }

              private:
                /// Контейнер объектов.
                QObjectList mContainer;
            };

            //------------------------------------------------------------------------------
        } // namespace Scripting
    } // namespace PaymentProcessor
} // namespace SDK
