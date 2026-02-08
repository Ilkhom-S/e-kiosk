/* @file Базовый протокол. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QtEndian>

#include <Common/ILogable.h>
#include <Common/SleepHelper.h>

#include <SDK/Drivers/IIOPort.h>

#include "Hardware/Common/CommandResults.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"

//--------------------------------------------------------------------------------
class ProtocolBase : public ILogable {
public:
    ProtocolBase() : m_Port(nullptr) {}

    /// Установить порт.
    void setPort(SDK::Driver::IIOPort *aPort) { m_Port = aPort; }

protected:
    /// Порт.
    SDK::Driver::IIOPort *m_Port;
};

//--------------------------------------------------------------------------------
