/* @file ФР ПРИМ c эжектором. */

#pragma once

#include "Prim_EjectorFR.h"

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrim_FR {
inline TModels EjectorModels() {
    return TModels() << CPrim_FR::Models::PRIM_21K_03;
}
} // namespace CPrim_FR

class Prim_EjectorFRBase : public Prim_EjectorFR<Prim_FRBase> {
public:
    Prim_EjectorFRBase() {
        m_Models = CPrim_FR::EjectorModels();
        m_DeviceName = CPrim_FR::ModelData[CPrim_FR::Models::PRIM_21K_03].name;
    }

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList() { return CPrim_FR::getModelList(CPrim_FR::EjectorModels()); }
};

//--------------------------------------------------------------------------------
