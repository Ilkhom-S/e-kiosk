/* @file Обертка для проброса в скрипты объекта c++.
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QObjectList>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
/// Интерфейс для объектов сценария бэкенда.
class IBackendScenarioObject : public QObject {
    Q_OBJECT

public:
    /// Конструктор.
    IBackendScenarioObject(QObject *aParent = nullptr) : QObject(aParent) {}
    /// Деструктор.
    virtual ~IBackendScenarioObject() {}

public:
    /// Получить имя.
    virtual QString getName() const = 0;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
