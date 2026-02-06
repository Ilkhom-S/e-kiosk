/* @file Типы данных купюроприемников на протоколе CCNet. */

#pragma once

#include <QtCore/QByteArray>

#include <Hardware/Common/Specifications.h>
#include <Hardware/Protocols/CashAcceptor/CCNetDataTypes.h>

//--------------------------------------------------------------------------------
namespace CCCNet {
/// Интервалы поллинга, [мс].
namespace PollingIntervals {
/// При выключении на прием денег.
const int Disabled = 800;

/// При включении на прием денег.
const int Enabled = 220;
} // namespace PollingIntervals

namespace Commands {
/// Спецификация команд.
class Data : public CSpecification<QByteArray, SData> {
public:
    Data() { setDefault(SData(true, false, PollingIntervals::Enabled)); }

    void add(const QByteArray &aCommand,
             bool aHostACK,
             int aTimeout = PollingIntervals::Enabled,
             bool aDeviceACK = false) {
        append(aCommand, SData(aDeviceACK, aHostACK, aTimeout));
    }
};
} // namespace Commands
} // namespace CCCNet

//--------------------------------------------------------------------------------