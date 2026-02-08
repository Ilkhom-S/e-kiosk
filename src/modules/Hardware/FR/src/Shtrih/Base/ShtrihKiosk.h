/* @file Штрих Киоск-ФР-К. */

#pragma once

#include "ShtrihRetractorFR.h"

//--------------------------------------------------------------------------------
class ShtrihKioskFRK : public ShtrihRetractorFR {
    SET_SUBSERIES("Kiosk")

public:
    ShtrihKioskFRK() {
        m_DeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::ShtrihKioskFRK_2].name;
        m_SupportedModels = QStringList() << m_DeviceName;
        setConfigParameter(CHardware::Printer::PresenterEnable, true);
    }

protected:
    /// Вытолкнуть чек.
    virtual bool push() {
        return processCommand(CShtrihFR::Commands::Push, QByteArray(1, CShtrihFR::PushNoPresenter));
    }
};

//--------------------------------------------------------------------------------
