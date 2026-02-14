/**
 * @file Виджет HumoService
 */

#include "HumoServiceMenu.h"

#include <QtCore/QTime>
#include <QtWidgets/QGraphicsScene>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginInitializer.h>

#include <utility>

#include "Backend/HumoServiceBackend.h"
#include "MessageBox/MessageBox.h"

namespace CHumoServiceMenu {
const QString PluginName = "HumoServiceMenu";
} // namespace CHumoServiceMenu

namespace PPSDK = SDK::PaymentProcessor;

//--------------------------------------------------------------------------
namespace {

/// Конструктор плагина.
SDK::Plugin::IPlugin *createPlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new HumoServiceMenu(aFactory, aInstancePath);
}

} // namespace

using map_type = QMap<int, bool>;
Q_DECLARE_METATYPE(map_type)

static SDK::Plugin::TParameterList enumerateParameters() {
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
                                         CHumoServiceMenu::PluginName),
                                &createPlugin,
                                &enumerateParameters,
                                HumoServiceMenu);

//--------------------------------------------------------------------------
HumoServiceMenu::HumoServiceMenu(SDK::Plugin::IEnvironment *aFactory, QString aInstancePath)
    : m_Environment(aFactory), m_InstancePath(std::move(aInstancePath)),
      m_MainHumoServiceMenuWindow(nullptr), m_IsReady(true) {
    void *voidPtr = reinterpret_cast<void *>(
        m_Environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
    auto *core = reinterpret_cast<SDK::PaymentProcessor::ICore *>(voidPtr);

    if (core) {
        m_Backend = QSharedPointer<HumoServiceBackend>(new HumoServiceBackend(
            m_Environment, m_Environment->getLog(CHumoServiceBackend::LogName)));
    } else {
        m_Environment->getLog(CHumoServiceBackend::LogName)
            ->write(LogLevel::Error, "Failed to get ICore");
    }

    m_MainHumoServiceMenuWindow = new MainHumoServiceMenuWindow(m_Backend.data());
    m_MainHumoServiceMenuWindow->initialize();
}

//--------------------------------------------------------------------------
HumoServiceMenu::~HumoServiceMenu() {
    if (m_MainHumoServiceMenuWindow) {
        saveConfiguration();
    }
}

//--------------------------------------------------------------------------
QString HumoServiceMenu::getPluginName() const {
    return CHumoServiceMenu::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap HumoServiceMenu::getConfiguration() const {
    return m_Backend->getConfiguration();
}

//--------------------------------------------------------------------------
void HumoServiceMenu::setConfiguration(const QVariantMap &aParameters) {
    m_Backend->setConfiguration(aParameters);
}

//--------------------------------------------------------------------------
QString HumoServiceMenu::getConfigurationName() const {
    return m_InstancePath;
}

//--------------------------------------------------------------------------
bool HumoServiceMenu::saveConfiguration() {
    return m_Environment->saveConfiguration(m_InstancePath, m_Backend->getConfiguration());
}

//--------------------------------------------------------------------------
bool HumoServiceMenu::isReady() const {
    return m_IsReady;
}

//---------------------------------------------------------------------------
void HumoServiceMenu::show() {
    GUI::MessageBox::setParentWidget(m_MainHumoServiceMenuWindow);

    m_Backend->startHeartbeat();
}

//---------------------------------------------------------------------------
void HumoServiceMenu::hide() {
    GUI::MessageBox::hide();
    m_Backend->stopHeartbeat();
}

//---------------------------------------------------------------------------
void HumoServiceMenu::notify(const QString &aReason, const QVariantMap &aParameters) {
    if (aReason.toInt() == PPSDK::EEventType::TryStopScenario) {
        m_MainHumoServiceMenuWindow->closeHumoServiceMenu(
            true, QObject::tr("#need_restart_application"), true);
    } else {
        GUI::MessageBox::emitSignal(aParameters);
    }
}

//---------------------------------------------------------------------------
void HumoServiceMenu::reset(const QVariantMap & /*aParameters*/) {
    if (m_MainHumoServiceMenuWindow) {
        m_MainHumoServiceMenuWindow->shutdown();
        m_MainHumoServiceMenuWindow->initialize();
    }
}

//---------------------------------------------------------------------------
QQuickItem *HumoServiceMenu::getWidget() const {
    return nullptr;
}

//---------------------------------------------------------------------------
QWidget *HumoServiceMenu::getNativeWidget() const {
    return m_MainHumoServiceMenuWindow;
}

//---------------------------------------------------------------------------
QVariantMap HumoServiceMenu::getContext() const {
    // TODO
    return {};
}

//---------------------------------------------------------------------------
bool HumoServiceMenu::isValid() const {
    return true;
}

//---------------------------------------------------------------------------
QString HumoServiceMenu::getError() const {
    return {};
}

//---------------------------------------------------------------------------
