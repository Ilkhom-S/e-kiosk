/* @file Принтер Citizen PPU-700. */

#pragma once

#include "../../EjectorPOS/EjectorPOS.h"
#include "../CitizenBase.h"

//--------------------------------------------------------------------------------
namespace CCitizenPPU700 {
/// Команды.
namespace Command {
extern const char GetFirmware[];     /// Получение версии прошивки.
extern const char GetSerialNumber[]; /// Получение серийного номера.

const QByteArray GetMemorySwitch5 =
    QByteArray::fromRawData("\x1D\x28\x45\x02\x00\x04\x05", 7); /// Получить значение мем-свича 5.
} // namespace Command

/// Мем-свичи.
namespace MemorySwitches {
/// Размер ответа на запрос мем-свича.
const int AnswerSize = 11;

/// Таймауты ожидания ответа на чтение, [мс].
const int ReadingTimeout = 300;
} // namespace MemorySwitches
} // namespace CCitizenPPU700

//--------------------------------------------------------------------------------
template <class T> class CitizenPPU700 : public CitizenBase<EjectorPOS<T>> {
    SET_SUBSERIES("CitizenPPU700")

public:
    CitizenPPU700();

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);

protected:
    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Запросить и сохранить параметры устройства.
    virtual void processDeviceData();

    /// Получить ответ.
    virtual bool getNULStoppedAnswer(QByteArray &aAnswer, int aTimeout) const;

    /// Доступны дополнительные мем-свичи.
    bool mOptionMSW;
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CitizenPPU700<TSerialPrinterBase>> TSerialCitizenPPU700;
typedef CitizenPPU700<TLibUSBPrinterBase> TLibUSBCitizenPPU700;

//--------------------------------------------------------------------------------
class SerialCitizenPPU700 : public TSerialCitizenPPU700 {
public:
    SerialCitizenPPU700() {
        using namespace SDK::Driver::IOPort::COM;

        this->mPortParameters.insert(EParameters::BaudRate,
                                     POSPrinters::TSerialDevicePortParameter()
                                         << EBaudRate::BR38400 << EBaudRate::BR19200
                                         << EBaudRate::BR4800 << EBaudRate::BR9600);
    }
};

//--------------------------------------------------------------------------------
template <class T> class CitizenPPU700II : public CitizenPPU700<T> {
    SET_SUBSERIES("CitizenPPU700II")

public:
    CitizenPPU700II() {
        this->mDeviceName = "Citizen PPU-700II";
        this->mOptionMSW = true;
    }
};

//--------------------------------------------------------------------------------
class LibUSBCitizenPPU700II : public CitizenPPU700II<TLibUSBPrinterBase> {
public:
    LibUSBCitizenPPU700II() {
        this->mDetectingData->set(CUSBVendors::Citizen1, this->mDeviceName, 0x201e);
    }
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CitizenPPU700II<TSerialPrinterBase>> SerialCitizenPPU700II;

//--------------------------------------------------------------------------------
