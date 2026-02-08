/* @file Онлайн ФР ПРИМ 06-Ф и 08-Ф. */

#pragma once

#include "Prim_OnlineFRBase.h"

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrim_FR {
TModels OnlineModels68() {
    return TModels() << CPrim_FR::Models::PRIM_06F << CPrim_FR::Models::PRIM_08F;
}
} // namespace CPrim_FR

//--------------------------------------------------------------------------------
class Prim_OnlineFR68 : public Prim_OnlineFRBase {
    SET_SUBSERIES("68F")

public:
    Prim_OnlineFR68() { m_Models = CPrim_FR::OnlineModels68(); }

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList() { return CPrim_FR::getModelList(CPrim_FR::OnlineModels68()); }

protected:
    /// Инициализация устройства.
    virtual bool updateParameters() {
        if (!Prim_OnlineFRBase::updateParameters()) {
            return false;
        }

        if (m_FFDFR > EFFD::F10) {
            m_AFDFont = CPrim_FR::FiscalFont::Narrow;
        }

        return true;
    }

    /// Получить параметр 3 ФР.
    ushort getParameter3() { return 0; }
};

//--------------------------------------------------------------------------------
