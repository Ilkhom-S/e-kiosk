/* @file Виртуальный ФР на протоколе Штрих на виртуальном COM-порту. */

#pragma once

#include "ProtoShtrihFR.h"
#include "ShtrihFRBaseConstants.h"

//--------------------------------------------------------------------------------
class VirtualShtrihFR : public ProtoShtrihFR<ShtrihSerialFRBase> {
    SET_SERIES("ShtrihVirtual")

public:
    VirtualShtrihFR() {
        using namespace SDK::Driver::IOPort::COM;

        // данные устройства
        m_DeviceName = "NeoService";
        m_Region = ERegion::KZ;
        m_LineFeed = false;
        m_TransportTimeout = 1000;

        // данные порта
        m_PortParameters[EParameters::BaudRate].clear();
        m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);

        // ошибки
        m_ErrorData = PErrorData(new CShtrihFRBase::Errors::Data);

        // данные налогов
        m_TaxData.data().clear();
        m_TaxData.add(12, 1);
        m_TaxData.add(0, 2);
    }

protected:
    /// Попытка самоидентификации.
    virtual bool isConnected() {
        SDK::Driver::EPortTypes::Enum portType = m_IOPort->getType();

        if (portType != SDK::Driver::EPortTypes::COMEmulator) {
            toLog(LogLevel::Error, m_DeviceName + ": Port type is not COM-emulator");
            return false;
        }

        QByteArray answer;

        if (!processCommand(CShtrihFR::Commands::IdentifyVirtual, &answer)) {
            return false;
        }

        setDeviceParameter(CDeviceData::Identity, m_Codec->toUnicode(answer.mid(2)));

        m_Type = CShtrihFR::Types::KKM;
        m_Model = CShtrihFR::Models::ID::NeoService;
        m_Parameters = CShtrihFR::FRParameters::Fields[m_Model];
        m_LineSize = 40;

        m_Verified = true;
        m_ModelCompatibility = true;

        return true;
    }

    /// Получить параметры печати.
    virtual bool getPrintingSettings() {
        m_LineSize = 42;

        return true;
    }
};

//--------------------------------------------------------------------------------
