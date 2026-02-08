/* @file Прокси-класс для работы с оборудованием. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IDeviceService;

namespace Scripting {

//------------------------------------------------------------------------------
/// Прокси-класс для работы с оборудованием.
class DeviceService : public QObject {
    Q_OBJECT

public:
    /// Конструктор.
    DeviceService(ICore *aCore);

public slots:
    /// Обнаружить устройства.
    void detect();
    /// Обработчик обнаружения устройства.
    void onDeviceDetected(const QString &aConfigName);
    /// Обработчик остановки обнаружения.
    void onDetectionStopped();

signals:
    /// Сигнал об обнаружении устройства.
    void deviceDetected(const QString &aConfigName);
    /// Сигнал об остановке обнаружения.
    void detectionStopped();

private:
    /// Указатель на ядро.
    ICore *m_Core;
    /// Указатель на сервис устройств.
    IDeviceService *m_DeviceService;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
