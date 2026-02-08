/* @file Фабрика платежей. */

#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QMutexLocker>
#include <QtCore/QStringDecoder>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Humo/RequestSender.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "AdBackend/Campaign.h"
#include "AdPayment.h"
#include "AdRemotePlugin.h"

namespace CPaymentFactory {
const char PluginName[] = "AdPayments";
const char ContentName[] = "banner";
} // namespace CPaymentFactory

namespace CProcessorType {
const QString Ad = "ad";
} // namespace CProcessorType

//------------------------------------------------------------------------------
namespace {

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *CreatePaymentFactory(SDK::Plugin::IEnvironment *aFactory,
                                           const QString &aInstancePath) {
    return new PaymentFactory(aFactory, aInstancePath);
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application,
                         SDK::PaymentProcessor::CComponents::PaymentFactory,
                         CPaymentFactory::PluginName),
                &CreatePaymentFactory,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                PaymentFactory);
} // namespace

//---------------------------------------------------------------------------
// Конструктор фабрики платежей
PaymentFactory::PaymentFactory(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : PaymentFactoryBase(aFactory, aInstancePath) {}

//------------------------------------------------------------------------------
QString PaymentFactory::getPluginName() const {
    return CPaymentFactory::PluginName;
}

//------------------------------------------------------------------------------
bool PaymentFactory::initialize() {
    return true;
}

//------------------------------------------------------------------------------
void PaymentFactory::shutdown() {}

//------------------------------------------------------------------------------
QStringList PaymentFactory::getSupportedPaymentTypes() const {
    return QStringList() << CProcessorType::Ad;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::IPayment *PaymentFactory::createPayment(const QString &aType) {
    if (aType.toLower() == CProcessorType::Ad) {
        AdPayment *adPayment = new AdPayment(this);

        adPayment->setParameter(SDK::PaymentProcessor::IPayment::SParameter(
            "AD_ID", getAdClientInstance(m_Factory)->getAd(CPaymentFactory::ContentName).id, true));

        return adPayment;
    }

    return nullptr;
}

//------------------------------------------------------------------------------
void PaymentFactory::releasePayment(SDK::PaymentProcessor::IPayment *aPayment) {
    delete dynamic_cast<AdPayment *>(aPayment);
}

//------------------------------------------------------------------------------
PPSDK::SProvider PaymentFactory::getProviderSpecification(const PPSDK::SProvider &aProvider) {
    if (aProvider.processor.type == CProcessorType::Ad) {
        QMutexLocker lock(&m_Mutex);

        PPSDK::SProvider provider = aProvider;

        QFile json(QString("%1/%2.json")
                       .arg(getAdClientInstance(m_Factory)->getContent(CPaymentFactory::ContentName))
                       .arg(CPaymentFactory::ContentName));
        if (json.open(QIODevice::ReadOnly)) {
            QStringDecoder decoder("UTF-8");
            provider.fields += PPSDK::SProvider::json2Fields(decoder(json.readAll()));
        } else {
            // TODO
            //  Поля не прочитали, провайдера не обновили
        }

        return provider;
    } else {
        return PPSDK::SProvider();
    }
}

//------------------------------------------------------------------------------
