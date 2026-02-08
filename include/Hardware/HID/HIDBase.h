/* @file Базовое HID-устройство. */

#pragma once

#include "Hardware/Common/DeviceBase.h"

//--------------------------------------------------------------------------------
template <class T> class HIDBase : public T {
public:
    HIDBase() : m_Enabled(false) {}

    /// Включает/выключает устройство на чтение штрих-кодов. Пикать все равно будет.
    virtual bool enable(bool aEnabled) {
        m_Enabled = aEnabled;

        return true;
    }

    /// Готов ли к работе (инициализировался успешно, ошибок нет).
    virtual bool isDeviceReady() { return m_Initialized == ERequestStatus::Success; }

protected:
    /// Признак включенности устройства.
    bool m_Enabled;

    /// Статус инициализации устройства.
    ERequestStatus::Enum m_Initialized;
};

//--------------------------------------------------------------------------------
