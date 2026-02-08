/* @file Принтер Custom VKP-80. */

#pragma once

#include <Hardware/Printers/EjectorPOS.h>

//--------------------------------------------------------------------------------
/// Константы принтера Custom VKP-80.
namespace CCustom_VKP80 {
/// Ожидание прихода XOn после XOff, [мс].
const SWaitingData XOnWaiting = SWaitingData(100, 60 * 1000);

/// Ожидание окончания печати, [мс].
const SWaitingData PrintingWaiting = SWaitingData(100, 10 * 1000);
} // namespace CCustom_VKP80

//--------------------------------------------------------------------------------
template <class T> class Custom_VKP80 : public EjectorPOS<T> {
    SET_SUBSERIES("Custom_VKP80")

public:
    Custom_VKP80();

    /// Инициализация устройства.
    virtual bool updateParametersOut();

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);

protected:
    /// Напечатать чек.
    virtual bool printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt);
};

//--------------------------------------------------------------------------------
class LibUSBCustom_VKP80 : public Custom_VKP80<TLibUSBPrinterBase> {
public:
    LibUSBCustom_VKP80() {
        this->m_DetectingData->set(CUSBVendors::Custom, this->m_DeviceName, 0x015d);
    }
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<Custom_VKP80<TSerialPrinterBase>> SerialCustom_VKP80;

//--------------------------------------------------------------------------------
