/* @file Параметры POS-принтеров. */

#include <QtCore/QMutexLocker>
#include <QtCore/QRecursiveMutex>

#include <Hardware/Printers/POSParameters.h>
#include <utility>

namespace POSPrinters {

SModelData::SModelData() = default;

//--------------------------------------------------------------------------------
SModelData::SModelData(QString aName, bool aVerified, QString aDescription)
    : name(std::move(aName)), verified(aVerified), description(std::move(aDescription)) {}

//--------------------------------------------------------------------------------
ModelData::ModelData() = default;

//--------------------------------------------------------------------------------
void ModelData::add(char aModelId,
                    bool aVerified,
                    const QString &aName,
                    const QString &aDescription) {
    QMutexLocker locker(&m_Mutex);

    append(aModelId, SModelData(aName, aVerified, aDescription));
    m_ModelIds.insert(aModelId);
}

//--------------------------------------------------------------------------------
const TModelIds &ModelData::getModelIds() {
    QMutexLocker locker(&m_Mutex);

    return m_ModelIds;
}

} // namespace POSPrinters

//--------------------------------------------------------------------------------
