/* @file Прокси-класс для работы с оборудованием. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

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
                ICore *mCore;
                /// Указатель на сервис устройств.
                IDeviceService *mDeviceService;
            };

            //------------------------------------------------------------------------------
        } // namespace Scripting
    } // namespace PaymentProcessor
} // namespace SDK
