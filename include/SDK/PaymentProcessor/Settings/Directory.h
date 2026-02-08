/* @file Справочники. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include <Common/ILogable.h>
#include <Common/PropertyTree.h>

#include <SDK/PaymentProcessor/Connection/Connection.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Settings/Range.h>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
class Directory : public ISettingsAdapter, public ILogable {
public:
    Directory(TPtree &aProperties);
    ~Directory();

    /// Валидация данных.
    virtual bool isValid() const;

    /// Получить имя адаптера.
    static QString getAdapterName();

    /// Получить шаблоны соединения.
    QList<SConnectionTemplate> getConnectionTemplates() const;

    /// Возвращает диапазоны для заданного номера.
    QList<SRange> getRangesForNumber(qint64 aNumber) const;

    /// Возвращает список ID операторов, которые имеют виртуальные ёмкости.
    QSet<qint64> getOverlappedIDs() const;

private:
    Directory(const Directory &);
    void operator=(const Directory &);

private:
    TPtree &m_Properties;

    QVector<SRange> m_Ranges;
    QVector<SRange> m_OverlappedRanges;
    QSet<qint64> m_OverlappedIDs;
    QDateTime m_RangesTimestamp;
};

} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
