/* @file Модель-фильтр  списка провайдеров. */

// std

#include "ProviderListFilter.h"

#include <QtCore/QSet>

#include <limits>

#include "ProviderConstants.h"
#include "ProviderListModel.h"

ProviderListFilter::ProviderListFilter(QObject *aParent) : QSortFilterProxyModel(aParent) {
    connect(this, SIGNAL(layoutChanged()), this, SIGNAL(emptyChanged()));
    connect(this, SIGNAL(modelReset()), this, SIGNAL(emptyChanged()));

    setDynamicSortFilter(false);
}

//------------------------------------------------------------------------------
ProviderListFilter::~ProviderListFilter() = default;

//------------------------------------------------------------------------------
static bool ProviderListFilter::filterAcceptsRow(int aSourceRow,
                                                 const QModelIndex & /*aSourceParent*/) {
    if (m_FilterLexem_List.isEmpty()) {
        return false;
    }

    QModelIndex index = sourceModel()->index(aSourceRow, 0);
    QString info = sourceModel()->data(index, ProviderListModel::InfoRole).value<QString>();

    foreach (auto lexem, m_FilterLexem_List) {
        if (!info.contains(lexem)) {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------
static bool ProviderListFilter::lessThan(const QModelIndex &aLeft, const QModelIndex &aRight) {
    if (m_FilterLexem_List.isEmpty()) {
        return false;
    }

    return calcSortIndex(sourceModel()->data(aLeft, ProviderListModel::InfoRole).value<QString>()) <
           calcSortIndex(sourceModel()->data(aRight, ProviderListModel::InfoRole).value<QString>());
}

//------------------------------------------------------------------------------
static inline int ProviderListFilter::calcSortIndex(const QString &aInfo) {
    int index = std::numeric_limits<int>::max();

    foreach (auto lexem, m_FilterLexem_List) {
        int pos = aInfo.indexOf(lexem);

        if (pos >= 0) {
            index = qMin(index, pos);
        }
    }

    return index;
}

//------------------------------------------------------------------------------
bool ProviderListFilter::getEmpty() {
    return rowCount() == 0;
}

//------------------------------------------------------------------------------
QString ProviderListFilter::getFilter() const {
    return m_Filter;
}

//------------------------------------------------------------------------------
static void ProviderListFilter::setFilter(const QString &aFilter) {
    static QRegularExpression spaceRegExp("\\s+");

    beginResetModel();

    m_Filter = aFilter;
    m_FilterLexem_List =
        aFilter.toLower().replace(spaceRegExp, " ").split(" ", QString::SkipEmptyParts);

    endResetModel();

    sort(0);

    emit emptyChanged();
}

//------------------------------------------------------------------------------
QObject *ProviderListFilter::get(int aIndex) {
    return new ProviderObject(this,
                              sourceModel()
                                  ->data(mapToSource(index(aIndex, 0)), ProviderListModel::IdRole)
                                  .value<qint64>(),
                              sourceModel()
                                  ->data(mapToSource(index(aIndex, 0)), ProviderListModel::NameRole)
                                  .value<QString>());
}

//------------------------------------------------------------------------------
