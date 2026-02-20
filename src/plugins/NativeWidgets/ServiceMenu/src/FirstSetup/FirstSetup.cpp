/* @file Виджет первоначальной настройки терминала */

#include "FirstSetup.h"

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginInitializer.h>

#include <utility>

#include "Backend/ServiceMenuBackend.h"
#include "GUI/MessageBox/MessageBox.h"

namespace CFirstSetup {
const QString PluginName = "FirstSetup";
} // namespace CFirstSetup

//--------------------------------------------------------------------------
namespace {

/// Конструктор плагина.
SDK::Plugin::IPlugin *createPlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new FirstSetup(aFactory, aInstancePath);
}

} // namespace

REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application,
                         SDK::PaymentProcessor::CComponents::GraphicsItem,
                         CFirstSetup::PluginName),
                &createPlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                FirstSetup);

//--------------------------------------------------------------------------
FirstSetup::FirstSetup(SDK::Plugin::IEnvironment *aFactory, QString aInstancePath)
    : m_Environment(aFactory), m_InstancePath(std::move(aInstancePath)), m_IsReady(false) {
    void *voidPtr = reinterpret_cast<void *>(
        m_Environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
    auto *core = reinterpret_cast<SDK::PaymentProcessor::ICore *>(voidPtr);

    if (core) {
        m_Backend = QSharedPointer<ServiceMenuBackend>(new ServiceMenuBackend(
            m_Environment, m_Environment->getLog(CServiceMenuBackend::LogName)));
    } else {
        m_Environment->getLog("ServiceMenu")->write(LogLevel::Error, "Failed to get ICore");
    }

    m_IsReady = core != nullptr;

    if (m_IsReady) {
        m_WizardFrame = new WizardFrame(m_Backend.data());
        m_WizardFrame->initialize();
        m_WizardFrame->setStatus(QObject::tr("#humo_copyright"));
    }
}

//--------------------------------------------------------------------------
FirstSetup::~FirstSetup() {
    if (m_WizardFrame) {
        m_WizardFrame->shutdown();
        m_WizardFrame->deleteLater();
    }
}

//--------------------------------------------------------------------------
QString FirstSetup::getPluginName() const {
    return CFirstSetup::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap FirstSetup::getConfiguration() const {
    return m_Parameters;
}

//--------------------------------------------------------------------------
void FirstSetup::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//--------------------------------------------------------------------------
QString FirstSetup::getConfigurationName() const {
    return m_InstancePath;
}

//--------------------------------------------------------------------------
bool FirstSetup::saveConfiguration() {
    return true;
}

//--------------------------------------------------------------------------
bool FirstSetup::isReady() const {
    return m_IsReady;
}

//---------------------------------------------------------------------------
void FirstSetup::show() {}

//---------------------------------------------------------------------------
void FirstSetup::hide() {}

//---------------------------------------------------------------------------
void FirstSetup::notify(const QString & /*aReason*/, const QVariantMap &aParameters) {
    GUI::MessageBox::emitSignal(aParameters);
}

//---------------------------------------------------------------------------
void FirstSetup::reset(const QVariantMap & /*aParameters*/) {}

//---------------------------------------------------------------------------
QQuickItem *FirstSetup::getWidget() const {
    return nullptr;
}

//---------------------------------------------------------------------------
QVariantMap FirstSetup::getContext() const {
    // TODO
    return {};
}

//---------------------------------------------------------------------------
bool FirstSetup::isValid() const {
    return m_WizardFrame != nullptr;
}

//---------------------------------------------------------------------------
QString FirstSetup::getError() const {
    return {};
}

//---------------------------------------------------------------------------
