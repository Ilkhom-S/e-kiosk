/* @file Базовый ФР семейства Штрих на COM-порту. */

#pragma once

#include "Hardware/FR/PortFRBase.h"

//--------------------------------------------------------------------------------
class ShtrihSerialFRBase : public TSerialFRBase {
public:
    ShtrihSerialFRBase() {
        using namespace SDK::Driver::IOPort::COM;

        // данные порта
        m_PortParameters[EParameters::BaudRate].append(
            EBaudRate::BR115200); // default for all except Shtrih Mini
        m_PortParameters[EParameters::BaudRate].append(
            EBaudRate::BR19200); // default for Shtrih Mini
        m_PortParameters[EParameters::BaudRate].append(
            EBaudRate::BR4800); // default after resetting to zero
        m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
        m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
        m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

        m_PortParameters[EParameters::Parity].append(EParity::No);
    }
};

//--------------------------------------------------------------------------------
