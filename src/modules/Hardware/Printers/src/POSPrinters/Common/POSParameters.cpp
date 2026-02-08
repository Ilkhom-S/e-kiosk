/* @file Параметры POS-принтеров. */

#include <QtCore/QMutexLocker>
#include <QtCore/QRecursiveMutex>

#include <Hardware/Printers/POSParameters.h>

namespace POSPrinters {

SModelData::SModelData() {}

//--------------------------------------------------------------------------------
SModelData::SModelData(const QString &aName, bool aVerified, const QString &aDescription)
    : name(aName), verified(aVerified), description(aDescription) {}

//--------------------------------------------------------------------------------
ModelData::ModelData() {}

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
