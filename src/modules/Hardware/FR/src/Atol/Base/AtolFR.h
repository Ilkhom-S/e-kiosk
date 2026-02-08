/* @file ПД АТОЛ. */

#pragma once

#include "AtolSerialFR.h"

//--------------------------------------------------------------------------------
class PayCTS2000 : public AtolSerialFR {
    SET_SUBSERIES("PayCTS2000")

public:
    PayCTS2000::PayCTS2000() {
        m_DeviceName = CAtolFR::Models::PayCTS2000K;
        m_SupportedModels = QStringList() << m_DeviceName;
    }
};

//--------------------------------------------------------------------------------
class AtolFR : public AtolSerialFR {
public:
    AtolFR::AtolFR() {
        m_DeviceName = "ATOL FR";
        m_SupportedModels = getModelList();
    }

    static QStringList getModelList() {
        return CAtolFR::CModelData().getModelList(EFRType::EKLZ, true);
    }
};

//--------------------------------------------------------------------------------
class AtolFRSingle : public AtolFR {
    SET_SUBSERIES("Single")

public:
    AtolFRSingle::AtolFRSingle() {
        m_DeviceName = "ATOL single FR";
        m_SupportedModels = getModelList();
    }

    static QStringList getModelList() {
        return CAtolFR::CModelData().getModelList(EFRType::EKLZ, false);
    }
};

//--------------------------------------------------------------------------------
class AtolDP : public AtolSerialFR {
    SET_DEVICE_TYPE(DocumentPrinter)

public:
    AtolDP::AtolDP() {
        m_DeviceName = "ATOL DP";
        m_SupportedModels = getModelList();
    }

    static QStringList getModelList() {
        return CAtolFR::CModelData().getModelList(EFRType::NoEKLZ, true);
    }
};

//--------------------------------------------------------------------------------
class AtolDPSingle : public AtolDP {
    SET_SUBSERIES("Single")

public:
    AtolDPSingle::AtolDPSingle() {
        m_DeviceName = "ATOL single DP";
        m_SupportedModels = getModelList();
    }

    static QStringList getModelList() {
        return CAtolFR::CModelData().getModelList(EFRType::NoEKLZ, false);
    }
};

//--------------------------------------------------------------------------------
