/* @file Модель со списком провайдеров, загруженных клиентом. */

#include "ProviderListModel.h"

#include <QtCore/QSet>
#include <QtCore/QTimer>

#include "GroupModel.h"
#include "ProviderConstants.h"

ProviderListModel::ProviderListModel(QObject *aParent, QSharedPointer<GroupModel> aGroupModel)
    : QAbstractListModel(aParent), m_GroupModel(aGroupModel) {
    m_Roles[IdRole] = "id";
    m_Roles[NameRole] = "name";
    m_Roles[InfoRole] = "info";
    m_Roles[ImageRole] = "image";
}

//------------------------------------------------------------------------------
ProviderListModel::~ProviderListModel() {}

//------------------------------------------------------------------------------
QHash<int, QByteArray> ProviderListModel::roleNames() const {
    return m_Roles;
}

//------------------------------------------------------------------------------
int ProviderListModel::rowCount(const QModelIndex & /*parent = QModelIndex()*/) const {
    return m_ProviderList.size();
}

//------------------------------------------------------------------------------
QVariant ProviderListModel::data(const QModelIndex &aIndex, int aRole) const {
    if (aIndex.row() >= 0 && aIndex.row() < m_ProviderList.count()) {
        switch (aRole) {
        case IdRole:
        case ImageRole:
            return m_ProviderList[aIndex.row()].id;
        case NameRole:
            return m_ProviderList[aIndex.row()].name;
        case InfoRole:
            return m_ProviderList[aIndex.row()].info;
        }
    }

    return QVariant();
}

//------------------------------------------------------------------------------
void ProviderListModel::groupsUpdated() {
    if (!m_ProvidersId.empty() || !m_ProviderList.empty())
        return;

    if (m_GroupModel) {
        auto providersId = m_GroupModel->allProviders().toList();

        // Исключаем повторную загрузку и индексацию
        if (providersId.size() > m_ProviderList.size() + m_ProvidersId.size()) {
            m_ProviderList.clear();
            m_ProvidersId = providersId;

            QMetaObject::invokeMethod(this, "getNextProviderInfo", Qt::QueuedConnection);
        }
    }
}

//------------------------------------------------------------------------------
void ProviderListModel::getNextProviderInfo() {
    if (!m_ProvidersId.isEmpty()) {
        SProvider provider(m_ProvidersId.first());

        if (provider.id > 0 && provider.id != Providers::AutodetectID) {
            QObject *providerObject = nullptr;

            if (QMetaObject::invokeMethod(m_PaymentService.data(),
                                          "getProvider",
                                          Q_RETURN_ARG(QObject *, providerObject),
                                          Q_ARG(qint64, provider.id)) &&
                providerObject) {
                // Проверка - если провайдера нет в operators
                if (providerObject->property("id").value<qint64>() == provider.id) {
                    provider.name = providerObject->property("name").value<QString>();
                    provider.info =
                        provider.name
                            .toLower(); // +
                                        // providerObject->property("comment").value<QString>().toLower();

                    if (provider.info.size() < 256) {
                        provider.info += QString().fill(' ', 256 - provider.info.size());
                    }

                    foreach (auto receiptString,
                             providerObject->property("receiptParameters")
                                 .value<QVariantMap>()
                                 .values()) {
                        provider.info += receiptString.toString().toLower();
                    }
                } else {
                    // Если не смогли добыть полное описание провайдера берем его имя из модели
                    // групп
                    provider.name = m_GroupModel->getProviderName(provider.id);
                    provider.info = provider.name.toLower();
                }

                providerObject->deleteLater();
            }

            emit beginInsertRows(QModelIndex(), m_ProviderList.size(), m_ProviderList.size());

            m_ProviderList << provider;

            emit endInsertRows();
        }

        m_ProvidersId.removeAll(provider.id);
    }

    if (!m_ProvidersId.isEmpty()) {
        QMetaObject::invokeMethod(this, "getNextProviderInfo", Qt::QueuedConnection);
    }
}

//------------------------------------------------------------------------------
void ProviderListModel::setPaymentService(QObject *aPaymentService) {
    m_PaymentService = aPaymentService;
}
