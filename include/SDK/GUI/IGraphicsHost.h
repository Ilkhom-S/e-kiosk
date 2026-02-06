/* @file Интерфейс класса, использующего GUI контейнер. */

#pragma once

#include <QtCore/QStringList>

namespace SDK {
namespace GUI {

//---------------------------------------------------------------------------
/// Интерфейс хоста, использующего графический движок и предоставляющий доступ к
/// различным сервисам.
class IGraphicsHost {
public:
    /// Возвращает список имеющихся интерфейсов
    virtual QStringList getInterfacesName() const = 0;

    /// Возвращает указатель на сервис с именем aInterface.
    template <class T> T *getInterface(const QString &aInterface) {
        return reinterpret_cast<T *>(getInterface(aInterface));
    }

protected:
    /// Возвращает указатель на сервис с именем aInterface. Внутренняя функция,
    /// которую должен реализовать потомок этого интерфейса.
    virtual void *getInterface(const QString &aInterface) = 0;

    virtual ~IGraphicsHost() {}
};

} // namespace GUI
} // namespace SDK

//---------------------------------------------------------------------------
