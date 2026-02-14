/* @file База для фабрики платежей. */

#include "PaymentFactoryBase.h"

#include <QtCore/QDateTime>

#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/Plugins/IExternalInterface.h>

#include <utility>

namespace CPaymentFactory {
const char LogName[] = "Ad";
} // namespace CPaymentFactory

//---------------------------------------------------------------------------
// Конструктор базовой фабрики платежей
PaymentFactoryBase::PaymentFactoryBase(SDK::Plugin::IEnvironment *aFactory, QString aInstancePath)
    : m_Initialized(false), m_Factory(aFactory), m_InstancePath(std::move(aInstancePath)),
      m_Core(nullptr), m_CryptEngine(nullptr) {
    try {
        LOG(getLog(),
            LogLevel::Debug,
            QString("PaymentFactoryBase: requesting ICore interface..."));

        void *voidPtr = reinterpret_cast<void *>(
            m_Factory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
        m_Core = reinterpret_cast<SDK::PaymentProcessor::ICore *>(voidPtr);

        LOG(getLog(),
            LogLevel::Debug,
            QString("PaymentFactoryBase: m_Core = 0x%1").arg(qlonglong(m_Core), 0, 16));

        if (!m_Core) {
            throw SDK::PaymentProcessor::ServiceIsNotImplemented("ICore interface not available");
        }

        auto *cryptService = m_Core->getCryptService();
        if (!cryptService) {
            throw SDK::PaymentProcessor::ServiceIsNotImplemented("CryptService not available");
        }
        m_CryptEngine = cryptService->getCryptEngine();

        auto *networkService = m_Core->getNetworkService();
        if (!networkService) {
            throw SDK::PaymentProcessor::ServiceIsNotImplemented("NetworkService not available");
        }
        m_Network = networkService->getNetworkTaskManager();

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
