/* @file Реализация фабрики плагина HumoPayments - инициализация метаданных. */

#include "HumoPaymentsFactory.h"

HumoPaymentsFactory::HumoPaymentsFactory() {
    mModuleName = "humo_payments";
    mName = "Humo Payments";
    mDescription = "Платежный плагин для HUMO, поддержка мультистейдж и дилерских платежей";
    mAuthor = "Humo";
    mVersion = "1.0";
}
