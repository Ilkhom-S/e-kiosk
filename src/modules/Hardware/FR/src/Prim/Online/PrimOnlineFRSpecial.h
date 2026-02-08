/* @file Онлайн ФР ПРИМ c эжектором. */

#pragma once

#include "../Ejector/Prim_EjectorFR.h"
#include "../Presenter/Prim_PresenterFR.h"

//--------------------------------------------------------------------------------
template <CPrim_FR::Models::Enum T1, class T2> class Prim_OnlineFRSpecial : public T2 {
public:
    Prim_OnlineFRSpecial() { setInitialParameters(); }

protected:
    /// Установить начальные параметры.
    void setInitialParameters() {
        m_Model = T1;
        m_Models = CPrim_FR::TModels() << m_Model;
        m_DeviceName = CPrim_FR::ModelData[m_Model].name;
    }

    /// Попытка самоидентификации.
    virtual bool isConnected() {
        if (!T2::isConnected()) {
            return false;
        }

        CPrim_FR::TData commandData = QVector<QByteArray>(3, int2ByteArray(0)).toList();
        m_ModelCompatibility = (processCommand(CPrim_FR::Commands::SetEjectorAction, commandData) ==
                               CommandResult::Device) == (T1 == CPrim_FR::Models::PRIM_21FA_Epson);

        if (m_ModelCompatibility) {
            setInitialParameters();
        }

        return true;
    }
};

typedef Prim_OnlineFRSpecial<CPrim_FR::Models::PRIM_21FA_Custom, Prim_EjectorFR<Prim_OnlineFRBase>>
    Prim_EjectorOnlineFR;
typedef Prim_OnlineFRSpecial<CPrim_FR::Models::PRIM_21FA_Epson, Prim_PresenterFR<Prim_OnlineFRBase>>
    Prim_PresenterOnlineFR;

//--------------------------------------------------------------------------------
