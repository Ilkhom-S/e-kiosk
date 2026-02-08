/* @file Прокси-класс для работы с оборудованием. */

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Scripting/DeviceService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
DeviceService::DeviceService(ICore *aCore)
    : m_Core(aCore), m_DeviceService(m_Core->getDeviceService()) {
    connect(m_DeviceService,
            SIGNAL(deviceDetected(const QString &)),
            this,
            SLOT(onDeviceDetected(const QString &)));
    connect(m_DeviceService, SIGNAL(detectionStopped()), this, SLOT(onDetectionStopped()));
}

//------------------------------------------------------------------------------
void DeviceService::detect() {
    m_DeviceService->detect();
}

//------------------------------------------------------------------------------
void DeviceService::onDeviceDetected(const QString &aConfigName) {
    emit deviceDetected(aConfigName);
}

//------------------------------------------------------------------------------
void DeviceService::onDetectionStopped() {
    emit detectionStopped();
}

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
