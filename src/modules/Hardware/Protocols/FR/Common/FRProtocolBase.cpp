/* @file Базовая реализация протоколов фискальников. */

#include "FRProtocolBase.h"

#include <cmath>

//--------------------------------------------------------------------------------
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
FRProtocolBase::FRProtocolBase() : m_SessionOpened(true), m_NeedCloseSession(false) {
    // данные порта
    m_PortParameters[EParameters::Parity].append(EParity::No);

    // кодек
    m_Codec = CodecByName[CHardware::Codepages::CP866];
}

//--------------------------------------------------------------------------------
bool FRProtocolBase::isSessionOpened() {
    return m_SessionOpened;
}

//--------------------------------------------------------------------------------
