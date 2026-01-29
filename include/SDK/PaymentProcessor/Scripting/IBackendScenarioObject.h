/* @file Обертка для проброса в скрипты объекта c++.
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
            /// Интерфейс для объектов сценария бэкенда.
            class IBackendScenarioObject : public QObject
            {
                Q_OBJECT

              public:
                /// Конструктор.
                IBackendScenarioObject(QObject *aParent = nullptr) : QObject(aParent)
                {
                }
                /// Деструктор.
                virtual ~IBackendScenarioObject()
                {
                }

              public:
                /// Получить имя.
                virtual QString getName() const = 0;
            };

            //------------------------------------------------------------------------------
        } // namespace Scripting
    } // namespace PaymentProcessor
} // namespace SDK
