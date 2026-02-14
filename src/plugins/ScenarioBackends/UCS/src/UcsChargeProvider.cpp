/**
 * @file Реализация плагина.
 */

#include "UcsChargeProvider.h"

#include <QtCore/QPointer>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/PluginInitializer.h>

namespace PPSDK = SDK::PaymentProcessor;

namespace CUcsChargeProvider {
const QString PluginName = "UcsChargeProvider";

QPointer<UcsChargeProvider> &plugin() {
    static QPointer<UcsChargeProvider> p;

    return p;
}
} // namespace CUcsChargeProvider

//------------------------------------------------------------------------------
namespace {
const char Param_RuntimePath[] = "ucs_runtime_path";

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    if (CUcsChargeProvider::plugin().isNull()) {
        CUcsChargeProvider::plugin() = new UcsChargeProvider(aFactory, aInstancePath);
    }

    return CUcsChargeProvider::plugin();
}
} // namespace

static SDK::Plugin::TParameterList EnumerateParameters() {
    return SDK::Plugin::TParameterList()
           << SDK::Plugin::SPluginParameter(SDK::Plugin::Parameters::Singleton,
                                            SDK::Plugin::SPluginParameter::Bool,
                                            false,
                                            SDK::Plugin::Parameters::Singleton,
                                            QString(),
                                            true,
                                            QVariantMap(),
                                            true);
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(
    SDK::Plugin::makePath(SDK::PaymentProcessor::Application,
                          SDK::PaymentProcessor::CComponents::ChargeProvider,
                          CUcsChargeProvider::PluginName),
    &CreatePlugin,
    &EnumerateParameters);

//------------------------------------------------------------------------------
UcsChargeProvider::UcsChargeProvider(SDK::Plugin::IEnvironment *aFactory,
                                     const QString &aInstancePath)
    : ILogable(aFactory->getLog(Ucs::LogName)), m_Factory(aFactory), m_InstancePath(aInstancePath),
      void *voidPtr = reinterpret_cast<void *>(
          aFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
m_Core(reinterpret_cast<SDK::PaymentProcessor::ICore *>(voidPtr)),
    m_Api(Ucs::API::getInstance(m_Core, aFactory->getLog(Ucs::LogName))) {
    qRegisterMetaType<SDK::PaymentProcessor::SNote>("SDK::PaymentProcessor::SNote");

    m_DealerSettings = dynamic_cast<PPSDK::DealerSettings *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));

    connect(m_Api.data(),
            SIGNAL(saleComplete(double, int, const QString &, const QString &)),
            SLOT(onSaleComplete(double, int, const QString &, const QString &)));

    connect(m_Api.data(), SIGNAL(encashmentComplete()), SLOT(onEncashmentComplete()));

    m_Core->getEventService()->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));
}

//------------------------------------------------------------------------------
UcsChargeProvider::~UcsChargeProvider() {}

//------------------------------------------------------------------------------
QString UcsChargeProvider::getPluginName() const {
    return CUcsChargeProvider::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap UcsChargeProvider::getConfiguration() const {
    return m_Parameters;
}

//------------------------------------------------------------------------------
void UcsChargeProvider::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;

    m_Api->setupRuntime(aParameters.value(Param_RuntimePath).toString());
}

//------------------------------------------------------------------------------
QString UcsChargeProvider::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::saveConfiguration() {
    // У плагина нет параметров
    return true;
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::subscribe(const char *aSignal, QObject *aReceiver, const char *aSlot) {
    return static_cast<bool>(
        QObject::connect(this,
                         aSignal,
                         aReceiver,
                         aSlot,
                         Qt::ConnectionType(Qt::UniqueConnection | Qt::QueuedConnection)));
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::unsubscribe(const char *aSignal, QObject *aReceiver) {
    return QObject::disconnect(aSignal, aReceiver);
}

//------------------------------------------------------------------------------
QString UcsChargeProvider::getMethod() {
    return m_Api->isReady() ? "card_ucs" : QString();
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::enable(PPSDK::TPaymentAmount aMaxAmount) {
    return m_Api->enable(aMaxAmount);
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::disable() {
    // Чистим ресурсы по команде сценария, т.к. нужны дополнительные движения с API
    // m_Api->disable();
    return true;
}

//------------------------------------------------------------------------------
void UcsChargeProvider::onEvent(const SDK::PaymentProcessor::Event &aEvent) {
    if (aEvent.getType() == PPSDK::EEventType::ProcessEncashment) {
        m_Api->encashment(false);
    }
}

//------------------------------------------------------------------------------
void UcsChargeProvider::onEncashmentComplete() {
    // TODO - напечатать все чеки отложенных инкассаций
    // m_Core->getPrinterService()->pri
}

//------------------------------------------------------------------------------
void UcsChargeProvider::onSaleComplete(double aAmount,
                                       int aCurrency,
                                       const QString &aRRN,
                                       const QString &aConfirmationCode) {
    toLog(LogLevel::Normal,
          QString("Sale complete: %1 RRN:%2 confirmation:%3.")
              .arg(aAmount, 0, 'f', 2)
              .arg(aRRN)
              .arg(aConfirmationCode));

    m_Core->getPaymentService()->updatePaymentField(
        m_Core->getPaymentService()->getActivePayment(),
        PPSDK::IPayment::SParameter("PAY_TOOL", 2, true, false, true));

    emit stacked(PPSDK::SNote(PPSDK::EAmountType::BankCard, aAmount, aCurrency, aRRN));
}

//------------------------------------------------------------------------------
