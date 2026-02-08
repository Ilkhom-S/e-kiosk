/* @file Реализация фабрики плагина HumoPayments - инициализация метаданных. */

#include "HumoPaymentsFactory.h"

HumoPaymentsFactory::HumoPaymentsFactory() {
    m_ModuleName = "humo_payments";
    m_Name = "Humo Payments";
    m_Description = "Платежный плагин для HUMO, поддержка мультистейдж и дилерских платежей";
    m_Author = "Humo";
    m_Version = "1.0";
}
