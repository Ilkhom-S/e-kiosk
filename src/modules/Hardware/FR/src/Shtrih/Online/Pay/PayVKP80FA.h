/* @file ФР PayVKP-80K-FA на протоколе Штрих. */

#pragma once

#include "PayFRBase.h"

//--------------------------------------------------------------------------------
template <class T> class PayVKP80FA : public PayFRBase<T> {
    SET_SUBSERIES("PayVKP80FA")

public:
    PayVKP80FA() {
        m_DeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::PayVKP80KFA].name;
        m_SupportedModels = QStringList() << m_DeviceName;

        m_PrinterModelId = CPayPrinters::Custom80;
        setDeviceParameter(CHardware::FR::PrinterModel, CPayPrinters::Models[m_PrinterModelId].name);
    }
};

typedef PayVKP80FA<ShtrihOnlineFRBase<ShtrihTCPFRBase>> PayVKP80FATCP;
typedef PayVKP80FA<ShtrihOnlineFRBase<ShtrihSerialFRBase>> PayVKP80FASerial;

//--------------------------------------------------------------------------------
