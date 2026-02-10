/* @file Фабрика платежей. */

#include "PaymentFactory.h"

#include <QtCore/QMutexLocker>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Humo/RequestSender.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "DealerPayment.h"
#include "MultistagePayment.h"
#include "Payment.h"
#include "PinGetCardListRequest.h"
#include "PinGetCardListResponse.h"
#include "PinPayment.h"

namespace CPaymentFactory {
const char PluginName[] = "HumoPayments";
} // namespace CPaymentFactory

namespace CProcessorType {
const QString Humo = "humo";
const QString HumoPin = "humo_pin";
const QString Dealer = "dealer";
const QString Multistage = "multistage";
} // namespace CProcessorType

//------------------------------------------------------------------------------
namespace {

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *createPaymentFactory(SDK::Plugin::IEnvironment *aFactory,
                                           const QString &aInstancePath) {
    return new PaymentFactory(aFactory, aInstancePath);
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application,
                         SDK::PaymentProcessor::CComponents::PaymentFactory,
                         CPaymentFactory::PluginName),
                &createPaymentFactory,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                HumoPaymentFactory);
} // namespace

//------------------------------------------------------------------------------
PaymentFactory::PaymentFactory(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : PaymentFactoryBase(aFactory, aInstancePath), m_PinLoader(nullptr) {}

//------------------------------------------------------------------------------
QString PaymentFactory::getPluginName() const {
    return CPaymentFactory::PluginName;
}

//------------------------------------------------------------------------------
bool PaymentFactory::initialize() {
    m_PinLoader = new PinLoader(this);
    return true;
}

//------------------------------------------------------------------------------
void PaymentFactory::shutdown() {
    delete m_PinLoader;
    m_PinLoader = nullptr;
}

//------------------------------------------------------------------------------
QStringList PaymentFactory::getSupportedPaymentTypes() const {
    return QStringList() << CProcessorType::Humo << CProcessorType::HumoPin
                         << CProcessorType::Dealer << CProcessorType::Multistage;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::IPayment *PaymentFactory::createPayment(const QString &aType) {
    if (aType.toLower() == CProcessorType::Humo) {
        return new Payment(this);
    }
    if (aType.toLower() == CProcessorType::HumoPin) {
        return new PinPayment(this);
    }
    if (aType.toLower() == CProcessorType::Dealer) {
        return new DealerPayment(this);
    }
    if (aType.toLower() == CProcessorType::Multistage) {
        return new MultistagePayment(this);
    }

    return nullptr;
}

//------------------------------------------------------------------------------
void PaymentFactory::releasePayment(SDK::PaymentProcessor::IPayment *aPayment) {
    delete dynamic_cast<Payment *>(aPayment);
}

//------------------------------------------------------------------------------
PPSDK::SProvider PaymentFactory::getProviderSpecification(const PPSDK::SProvider &aProvider) {
    if (aProvider.processor.type == CProcessorType::HumoPin) {
        return m_PinLoader->getProviderSpecification(aProvider);
    }
    if (aProvider.processor.type == CProcessorType::Humo) {
        return aProvider;
    }
    if (aProvider.processor.type == CProcessorType::Dealer) {
        return aProvider;
    }
    if (aProvider.processor.type == CProcessorType::Multistage) {
        return aProvider;
    }
    return {};
}

//------------------------------------------------------------------------------
QList<SPinCard> PaymentFactory::getPinCardList(qint64 aProvider) {
    return m_PinLoader ? m_PinLoader->getPinCardList(aProvider) : QList<SPinCard>();
}

//------------------------------------------------------------------------------
