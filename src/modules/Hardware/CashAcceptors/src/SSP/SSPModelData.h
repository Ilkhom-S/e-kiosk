/* @file Данные моделей устройств на протоколе SSP. */

#pragma once

#include "Hardware/CashAcceptors/ModelData.h"

//--------------------------------------------------------------------------------
namespace CSSP {
namespace Models {
/// Название устройства по умолчанию.
extern const char Default[];

extern const char NV9[];
extern const char NV10[];
extern const char NV150[];
extern const char NV200[];
extern const char NV200Spectral[];
extern const char SH3[];
extern const char SH4[];
extern const char BV20[];
extern const char BV50[];
extern const char BV100[];

class CData : public CSpecification<QString, SBaseModelData> {
public:
    CData() {
        add("NV0009", NV9);
        add("NV0010", NV10);
        add("NV0150", NV150);
        add("NV200", NV200);
        add("NVS200", NV200Spectral, true);
        add("SH0003", SH3);
        add("SH0004", SH4);
        add("BV0020", BV20);
        add("BV0050", BV50);
        add("BV0100", BV100);

        setDefault(SBaseModelData(Default));
    }

private:
    void add(const QString &aId, const QString &aModelName, bool aVerified = false) {
        append(aId, SBaseModelData(aModelName, aVerified, true));
    }
};

static CData Data;
} // namespace Models
} // namespace CSSP

//--------------------------------------------------------------------------------
