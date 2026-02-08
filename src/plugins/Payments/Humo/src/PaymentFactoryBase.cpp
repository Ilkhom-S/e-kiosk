/* @file База для фабрики платежей. */

#include "PaymentFactoryBase.h"

#include <QtCore/QDateTime>

#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/Plugins/IExternalInterface.h>

namespace CPaymentFactory {
const char LogName[] = "Payments";
} // namespace CPaymentFactory

//------------------------------------------------------------------------------
PaymentFactoryBase::PaymentFactoryBase(SDK::Plugin::IEnvironment *aFactory,
                                       const QString &aInstancePath)
    : m_Initialized(false), m_Factory(aFactory), m_InstancePath(aInstancePath), m_Core(0),
      m_CryptEngine(0) {
    try {
        m_Core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
            m_Factory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
        m_CryptEngine = m_Core->getCryptService()->getCryptEngine();
        m_Network = m_Core->getNetworkService()->getNetworkTaskManager();

        m_Initialized = true;
    } catch (const SDK::PaymentProcessor::ServiceIsNotImplemented &e) {
        m_Initialized = false;

        LOG(getLog(),
            LogLevel::Error,
            QString("Failed to initialize payment factory: %1.").arg(e.what()));
    }
}

//------------------------------------------------------------------------------
QVariantMap PaymentFactoryBase::getConfiguration() const {
    return m_Parameters;
}

//------------------------------------------------------------------------------
void PaymentFactoryBase::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//------------------------------------------------------------------------------
QString PaymentFactoryBase::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::saveConfiguration() {
    // У плагина нет параметров
    return true;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::restorePayment(
    SDK::PaymentProcessor::IPayment *aPayment,
    const QList<SDK::PaymentProcessor::IPayment::SParameter> &aParameters) {
    return aPayment->restore(aParameters);
}

//------------------------------------------------------------------------------
void PaymentFactoryBase::setSerializer(
    boost::function<bool(SDK::PaymentProcessor::IPayment *)> aSerializer) {
    m_Serializer = aSerializer;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::convertPayment(const QString & /*aTargetType*/,
                                        SDK::PaymentProcessor::IPayment * /*aPayment*/) {
    return false;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::savePayment(SDK::PaymentProcessor::IPayment *aPayment) {
    if (m_Serializer) {
        return m_Serializer(aPayment);
    }

    return false;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::ICore *PaymentFactoryBase::getCore() const {
    return m_Core;
}

//------------------------------------------------------------------------------
ILog *PaymentFactoryBase::getLog(const char *aLogName /*= nullptr*/) const {
    return m_Factory->getLog(aLogName ? aLogName : CPaymentFactory::LogName);
}

//------------------------------------------------------------------------------
ICryptEngine *PaymentFactoryBase::getCryptEngine() const {
    return m_CryptEngine;
}

//------------------------------------------------------------------------------
NetworkTaskManager *PaymentFactoryBase::getNetworkTaskManager() const {
    return m_Network;
}

//------------------------------------------------------------------------------
