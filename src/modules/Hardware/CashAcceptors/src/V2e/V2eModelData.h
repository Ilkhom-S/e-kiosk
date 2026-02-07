/* @file Данные моделей устройств на протоколе V2e. */

#pragma once

#include "Hardware/CashAcceptors/ModelData.h"

//--------------------------------------------------------------------------------
namespace CV2e {
namespace Models {
extern const char Aurora[];
extern const char Falcon[];
extern const char Argus[];
} // namespace Models

typedef QSet<QString> TModelKeys;

class ModelData : public CSpecification<TModelKeys, SBaseModelData> {
public:
    ModelData();
    SBaseModelData getData(const QString &aKey);

private:
    void add(const QString &aName, const TModelKeys &aModelKeys, bool aVerified = false);
};
} // namespace CV2e

bool operator<(const CV2e::TModelKeys &aKeys1, const CV2e::TModelKeys &aKeys2);

//--------------------------------------------------------------------------------
