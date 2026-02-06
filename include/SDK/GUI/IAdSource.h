/* @file Интерфейс поставщика рекламы. */

#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace SDK {
namespace GUI {

//---------------------------------------------------------------------------
/// Интерфейс поставщика рекламы.
class IAdSource {
public:
    /// Получить содержимое рекламного контента
    virtual QString getContent(const QString &aName) const = 0;

    /// Зарегистрировать событие в статистике
    virtual void addEvent(const QString &aName, const QVariantMap &aParameters) = 0;

protected:
    virtual ~IAdSource() {}
};

} // namespace GUI
} // namespace SDK

//---------------------------------------------------------------------------
