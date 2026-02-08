/* @file Абстрактный запрос к СУБД. */

#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>

//--------------------------------------------------------------------------------
namespace CIDatabaseQuery {
/// Имя лога по умолчанию.
const QString DefaultLog = "DatabaseProxy";
} // namespace CIDatabaseQuery

//--------------------------------------------------------------------------------
class IDatabaseQuery {
public:
    /// Деструктор.
    virtual ~IDatabaseQuery() {};

    /// Подготавливает запрос.
    virtual bool prepare(const QString &aQuery) = 0;
    /// Привязывает значение по имени.
    virtual void bindValue(const QString &aName, const QVariant &aValue) = 0;
    /// Привязывает значение по позиции.
    virtual void bindValue(int aPos, const QVariant &aValue) = 0;
    /// Выполняет запрос.
    virtual bool exec() = 0;
    /// Очищает запрос.
    virtual void clear() = 0;

    /// Переходит к первой записи.
    virtual bool first() = 0;
    /// Переходит к следующей записи.
    virtual bool next() = 0;
    /// Переходит к последней записи.
    virtual bool last() = 0;

    /// Проверяет валидность.
    virtual bool isValid() = 0;
    /// Возвращает количество затронутых строк.
    virtual int num_RowsAffected() const = 0;

    /// Возвращает значение по индексу.
    virtual QVariant value(int i) const = 0;
};
//--------------------------------------------------------------------------------
