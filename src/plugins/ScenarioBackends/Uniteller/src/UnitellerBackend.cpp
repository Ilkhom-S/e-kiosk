/**
 * @file Плагин сценария для оплаты картами
 */

#include "UnitellerBackend.h"

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QPair>
#include <QtCore/QSettings>

#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/PaymentProcessor/Scripting/PrinterService.h>

#include "API.h"

namespace {
/// Конструктор плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new CUnitellerBackend::UnitellerBackendPlugin(aFactory, aInstancePath);
}
} // namespace

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application,
                                      PPSDK::CComponents::ScriptFactory,
                                      CUnitellerBackend::PluginName),
                &CreatePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                UnitellerBackend);

namespace CUnitellerBackend {
//---------------------------------------------------------------------------
UnitellerBackendPlugin::UnitellerBackendPlugin(SDK::Plugin::IEnvironment *aFactory,
                                               const QString &aInstancePath)
    : m_Environment(aFactory), m_InstancePath(aInstancePath) {}

//---------------------------------------------------------------------------
PPSDK::Scripting::IBackendScenarioObject *
UnitellerBackendPlugin::create(const QString &aClassName) const {
    void *voidPtr =
        reinterpret_cast<void *>(m_Environment->getInterface(PPSDK::CInterfaces::ICore));
    PPSDK::ICore *core = reinterpret_cast<PPSDK::ICore *>(voidPtr);

    return new UnitellerCore(
        core,
        m_Environment->getLog(Uniteller::LogName),
        Uniteller::API::getInstance(m_Environment->getLog(Uniteller::LogName), core));
}

//---------------------------------------------------------------------------
UnitellerCore::UnitellerCore(SDK::PaymentProcessor::ICore *aCore,
                             ILog *aLog,
                             QSharedPointer<Uniteller::API> aAPI)
    : ILogable(aLog), m_Core(aCore), m_CountPINNumbers(0) {
    connect(&m_DummyTimer, SIGNAL(timeout()), this, SIGNAL(onTimeout()));
    m_DummyTimer.setInterval(1000);
    m_DummyTimer.start();

    connect(aAPI.data(), SIGNAL(readyToCard()), this, SIGNAL(onReadyToCard()));
    connect(aAPI.data(), SIGNAL(error(const QString &)), this, SIGNAL(onError(const QString &)));
    connect(aAPI.data(),
            SIGNAL(deviceEvent(Uniteller::DeviceEvent::Enum, Uniteller::KeyCode::Enum)),
            SLOT(onDeviceEvent(Uniteller::DeviceEvent::Enum, Uniteller::KeyCode::Enum)));
    connect(
        aAPI.data(), SIGNAL(print(const QStringList &)), SLOT(onPrintReceipt(const QStringList &)));
    connect(aAPI.data(), SIGNAL(onlineRequired()), SIGNAL(onOnlineRequired()));
    connect(aAPI.data(), SIGNAL(pinRequired()), SIGNAL(onPINRequired()));
    connect(this, SIGNAL(ejected()), aAPI.data(), SLOT(breakSell()));
}

//---------------------------------------------------------------------------
void UnitellerCore::ejectCard() {
    emit ejected();
}

//---------------------------------------------------------------------------
void UnitellerCore::onDeviceEvent(Uniteller::DeviceEvent::Enum aEvent,
                                  Uniteller::KeyCode::Enum aKeyCode) {
    switch (aEvent) {
    case Uniteller::DeviceEvent::KeyPress:
        switch (aKeyCode) {
        case Uniteller::KeyCode::Timeout:
            break;

        case Uniteller::KeyCode::Numeric:
            m_CountPINNumbers = qMin(4, m_CountPINNumbers + 1);
            emit onEnterPin(m_CountPINNumbers > 0 ? QString("*").repeated(m_CountPINNumbers) : "");
            break;

        case Uniteller::KeyCode::Clear:
            m_CountPINNumbers = qMax(0, m_CountPINNumbers - 1);
            emit onEnterPin(m_CountPINNumbers > 0 ? QString("*").repeated(m_CountPINNumbers) : "");
            break;

        case Uniteller::KeyCode::Cancel:
            m_CountPINNumbers = 0;
            emit onError(tr("#canceled_by_user"));
            break;

        case Uniteller::KeyCode::Enter:
            break;
        }
        break;

    case Uniteller::DeviceEvent::CardInserted:
        m_CountPINNumbers = 0;
        emit cardInserted();
        break;

    case Uniteller::DeviceEvent::CardCaptured:
        break;

    case Uniteller::DeviceEvent::CardOut:
        emit cardOut();
        break;

    default:
        break;
    }
}

//---------------------------------------------------------------------------
void UnitellerCore::onPrintReceipt(const QStringList &aLines) {
    QVariantMap parameters;
    parameters["EMV_DATA"] = aLines.join("[br]");

    SDK::PaymentProcessor::Scripting::Core *scriptingCore =
        static_cast<SDK::PaymentProcessor::Scripting::Core *>(
            dynamic_cast<SDK::GUI::IGraphicsHost *>(m_Core->getGUIService())
                ->getInterface<QObject>(SDK::PaymentProcessor::Scripting::CProxyNames::Core));

    SDK::PaymentProcessor::Scripting::PrinterService *ps =
        static_cast<SDK::PaymentProcessor::Scripting::PrinterService *>(
            scriptingCore->property("printer").value<QObject *>());

    ps->printReceipt("", parameters, "emv", DSDK::EPrintingModes::Continuous);
}

} // namespace CUnitellerBackend
//---------------------------------------------------------------------------
