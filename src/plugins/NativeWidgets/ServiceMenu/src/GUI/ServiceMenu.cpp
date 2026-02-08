/* @file Виджет сервисного меню */

#include "ServiceMenu.h"

#include <QtCore/QTime>
#include <QtWidgets/QGraphicsScene>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "Backend/ServiceMenuBackend.h"
#include "MessageBox/MessageBox.h"

namespace CServiceMenu {
const QString PluginName = "ServiceMenu";
} // namespace CServiceMenu

namespace PPSDK = SDK::PaymentProcessor;

//--------------------------------------------------------------------------
namespace {

/// Конструктор плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new ServiceMenu(aFactory, aInstancePath);
}

} // namespace

typedef QMap<int, bool> map_type;
Q_DECLARE_METATYPE(map_type)

static SDK::Plugin::TParameterList EnumerateParameters() {
    return SDK::Plugin::TParameterList()
           << SDK::Plugin::SPluginParameter("columnVisibility",
                                            SDK::Plugin::SPluginParameter::MultiSet,
                                            true,
                                            "columnVisibility",
                                            QString(),
                                            QVariantList())

           << SDK::Plugin::SPluginParameter(SDK::Plugin::Parameters::Singleton,
                                            SDK::Plugin::SPluginParameter::Bool,
                                            false,
                                            SDK::Plugin::Parameters::Singleton,
                                            QString(),
                                            true,
                                            QVariantMap(),
                                            true);
}

REGISTER_PLUGIN_WITH_PARAMETERS(makePath(SDK::PaymentProcessor::Application,
                                         SDK::PaymentProcessor::CComponents::GraphicsItem,
                                         CServiceMenu::PluginName),
                                &CreatePlugin,
                                &EnumerateParameters,
                                ServiceMenu);

//--------------------------------------------------------------------------
ServiceMenu::ServiceMenu(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : m_Environment(aFactory), m_InstancePath(aInstancePath), m_MainServiceWindow(nullptr),
      m_IsReady(true) {
    SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        m_Environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    if (core) {
        m_Backend = QSharedPointer<ServiceMenuBackend>(new ServiceMenuBackend(
            m_Environment, m_Environment->getLog(CServiceMenuBackend::LogName)));
    } else {
        m_Environment->getLog(CServiceMenuBackend::LogName)
            ->write(LogLevel::Error, "Failed to get ICore");
    }

    m_MainServiceWindow = new MainServiceWindow(m_Backend.data());
    m_MainServiceWindow->initialize();
}

//--------------------------------------------------------------------------
ServiceMenu::~ServiceMenu() {
    if (m_MainServiceWindow) {
        saveConfiguration();
    }
}

//--------------------------------------------------------------------------
QString ServiceMenu::getPluginName() const {
    return CServiceMenu::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap ServiceMenu::getConfiguration() const {
    return m_Backend->getConfiguration();
}

//--------------------------------------------------------------------------
void ServiceMenu::setConfiguration(const QVariantMap &aParameters) {
    m_Backend->setConfiguration(aParameters);
}

//--------------------------------------------------------------------------
QString ServiceMenu::getConfigurationName() const {
    return m_InstancePath;
}

//--------------------------------------------------------------------------
bool ServiceMenu::saveConfiguration() {
    return m_Environment->saveConfiguration(m_InstancePath, m_Backend->getConfiguration());
}

//--------------------------------------------------------------------------
bool ServiceMenu::isReady() const {
    return m_IsReady;
}

//---------------------------------------------------------------------------
void ServiceMenu::show() {
    GUI::MessageBox::setParentWidget(m_MainServiceWindow);

    m_Backend->startHeartbeat();
}

//---------------------------------------------------------------------------
void ServiceMenu::hide() {
    GUI::MessageBox::hide();
    m_Backend->stopHeartbeat();
}

//---------------------------------------------------------------------------
void ServiceMenu::notify(const QString &aReason, const QVariantMap &aParameters) {
    if (aReason.toInt() == PPSDK::EEventType::TryStopScenario) {
        m_MainServiceWindow->closeServiceMenu(true, QObject::tr("#need_restart_application"), true);
    } else {
        GUI::MessageBox::emitSignal(aParameters);
    }
}

//---------------------------------------------------------------------------
void ServiceMenu::reset(const QVariantMap & /*aParameters*/) {
    if (m_MainServiceWindow) {
        m_MainServiceWindow->shutdown();
        m_MainServiceWindow->initialize();
    }
}

//---------------------------------------------------------------------------
QQuickItem *ServiceMenu::getWidget() const {
    return nullptr;
}

//---------------------------------------------------------------------------
QWidget *ServiceMenu::getNativeWidget() const {
    return m_MainServiceWindow;
}

//---------------------------------------------------------------------------
QVariantMap ServiceMenu::getContext() const {
    // TODO
    return QVariantMap();
}

//---------------------------------------------------------------------------
bool ServiceMenu::isValid() const {
    return true;
}

//---------------------------------------------------------------------------
QString ServiceMenu::getError() const {
    return QString();
}

//---------------------------------------------------------------------------
