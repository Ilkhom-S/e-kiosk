/* @file Описатели состояний устройств. */

#pragma once

#include <QtCore/QString>

#include "Hardware/Common/BaseStatus.h"
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
/// Описатель состояний устройства, для внутреннего пользования в протоколах.
struct SDeviceCodeSpecification {
    /// Общий статус-код.
    int statusCode;

    /// Описание специфичного состояния для конкретного протокола.
    QString description;

    bool operator==(const SDeviceCodeSpecification &aDeviceCodeSpecification) const {
        return (aDeviceCodeSpecification.statusCode == statusCode) &&
               (aDeviceCodeSpecification.description == description);
    }

    SDeviceCodeSpecification() : statusCode(DeviceStatusCode::OK::Unknown) {}
    SDeviceCodeSpecification(int aStatusCode, QString aDescription)
        : statusCode(aStatusCode), description(aDescription) {}
};

const char *getUnknownDeviceCodeDescription();

/// Спецификация состояний устройства.
template <class T>
class DeviceCodeSpecificationBase : public CSpecification<T, SDeviceCodeSpecification> {
public:
    DeviceCodeSpecificationBase() {
        this->setDefault(SDeviceCodeSpecification(0, getUnknownDeviceCodeDescription()));
    }

    void appendStatus(T aCode, int aStatusCode, const QString &aDescription = "") {
        this->append(aCode, SDeviceCodeSpecification(aStatusCode, aDescription));
    }
};

//--------------------------------------------------------------------------------
