/* @file Данные о последних версиях прошивок. */

#pragma once

#include <SDK/Drivers/CashAcceptor/CurrencyList.h>

#include "Hardware/Common/Specifications.h"
#include "Models.h"

//--------------------------------------------------------------------------------
namespace CCCNet {
typedef QSet<int> TFirmwareVersionSet;
typedef QMap<int, QMap<bool, TFirmwareVersionSet>> TFirmwareVersions;

class CFirmwareVersions : public CSpecification<QString, TFirmwareVersions> {
public:
    CFirmwareVersions() {
        data()[Models::CashcodeGX][Currency::RUB][true] = TFirmwareVersionSet() << 1208;

        data()[Models::CashcodeSM][Currency::RUB][true] = TFirmwareVersionSet() << 1361;

        data()[Models::CashcodeSM][Currency::RUB][false] = TFirmwareVersionSet() << 1387 << 1434;
        data()[Models::CashcodeMSM][Currency::RUB][false] = TFirmwareVersionSet() << 1115;
        data()[Models::CashcodeMSM][Currency::EUR][false] = TFirmwareVersionSet()
                                                            << 1130 << 1228 << 1329 << 1411 << 1527;
        data()[Models::CashcodeMVU][Currency::RUB][false] = TFirmwareVersionSet() << 1330;
        data()[Models::CashcodeMFL][Currency::RUB][false] = TFirmwareVersionSet() << 1143;
        data()[Models::CashcodeSL][Currency::RUB][false] = TFirmwareVersionSet()
                                                           << 1013 << 2004 << 3003 << 0005;

        data()[Models::CashcodeMVU][Currency::KZT][false] = TFirmwareVersionSet() << 1314;
        data()[Models::CashcodeMFL][Currency::KZT][false] = TFirmwareVersionSet() << 1124;
        data()[Models::CashcodeMSM][Currency::KZT][false] = TFirmwareVersionSet() << 1126;

        data()[Models::CashcodeG200][Currency::RUB][false] = TFirmwareVersionSet() << 1523;
    }
};

class COutdatedFirmwareSeries : public CSpecification<QString, TFirmwareVersions> {
public:
    COutdatedFirmwareSeries() {
        data()[Models::CashcodeSM][Currency::RUB][false] = TFirmwareVersionSet() << 1600 << 1500;
        data()[Models::CashcodeMVU][Currency::KZT][false] = TFirmwareVersionSet() << 1700;
    }
};

static CFirmwareVersions FirmwareVersions;
static COutdatedFirmwareSeries OutdatedFirmwareSeries;
} // namespace CCCNet

//--------------------------------------------------------------------------------
