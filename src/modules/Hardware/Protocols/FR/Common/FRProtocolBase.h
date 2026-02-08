/* @file Базовый протокол фискальников. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QTime>

// Devices
#include "Hardware/Common/CodecDescriptions.h"
#include "Hardware/FR/FRStatusCodes.h"
#include "Hardware/Printers/PrinterConstants.h"
#include "Hardware/Protocols/Common/DeviceProtocolBase.h"
#include "Hardware/Protocols/FR/IFRProtocol.h"

//--------------------------------------------------------------------------------
/// Базовый класс протокола FRProtocolBase.
class FRProtocolBase : public DeviceProtocolBase<IFRProtocol> {
public:
    FRProtocolBase();

    /// Открыта ли сессия.
    virtual bool isSessionOpened();

protected:
    /// Локальный кодек.
    QTextCodec *m_Codec;

    /// Открыта ли сессия.
    bool m_SessionOpened;

    /// Необходимо выполнить Z-отчет (для ФР без буфера).
    bool m_NeedCloseSession;
};

//--------------------------------------------------------------------------------
