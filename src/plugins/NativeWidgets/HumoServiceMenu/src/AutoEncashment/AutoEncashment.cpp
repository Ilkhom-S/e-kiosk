/* @file Плагин авто инкассации терминала */

#include "AutoEncashment.h"

#include <QtWidgets/QGraphicsScene>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "Backend/HumoServiceBackend.h"

namespace CAutoEncashment {
const QString PluginName = "AutoEncashment";
} // namespace CAutoEncashment

//--------------------------------------------------------------------------
namespace {

/// Конструктор плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new AutoEncashment(aFactory, aInstancePath);
}

} // namespace

REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application,
                         SDK::PaymentProcessor::CComponents::GraphicsItem,
                         CAutoEncashment::PluginName),
                &CreatePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                AutoEncashment);

//--------------------------------------------------------------------------
AutoEncashment::AutoEncashment(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : m_MainWidget(0), m_Environment(aFactory), m_InstancePath(aInstancePath),
      m_AutoEncashmentWindow(0), m_IsReady(false) {
    SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        m_Environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    if (core) {
        m_Backend = QSharedPointer<HumoServiceBackend>(new HumoServiceBackend(
            m_Environment, m_Environment->getLog(CHumoServiceBackend::LogName)));
    } else {
        m_Environment->getLog("HumoService")->write(LogLevel::Error, "Failed to get ICore");
    }

    m_IsReady = core != 0;

    if (m_IsReady) {
        m_MainWidget = new QGraphicsProxyWidget();

        m_AutoEncashmentWindow = new AutoEncashmentWindow(m_Backend.data());
        m_AutoEncashmentWindow->initialize();

        m_MainWidget->setWidget(m_AutoEncashmentWindow);
        m_MainWidget->setScale(qMin(core->getGUIService()->getScreenSize(0).width() /
                                        qreal(m_AutoEncashmentWindow->width()),
                                    core->getGUIService()->getScreenSize(0).height() /
                                        qreal(m_AutoEncashmentWindow->height())));

        qreal newWidgetWidth =
            core->getGUIService()->getScreenSize(0).width() / m_MainWidget->scale();
        m_MainWidget->setMinimum_Width(newWidgetWidth);
        m_MainWidget->setMaximum_Width(newWidgetWidth);

        qreal newWidgetHeight =
            core->getGUIService()->getScreenSize(0).height() / m_MainWidget->scale();
        m_MainWidget->setMinimum_Height(newWidgetHeight);
        m_MainWidget->setMaximum_Height(newWidgetHeight);
    }
}

//--------------------------------------------------------------------------
AutoEncashment::~AutoEncashment() {
    if (m_MainWidget) {
        m_AutoEncashmentWindow->shutdown();
        m_MainWidget->deleteLater();
    }
}

//--------------------------------------------------------------------------
QString AutoEncashment::getPluginName() const {
    return CAutoEncashment::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap AutoEncashment::getConfiguration() const {
    return m_Parameters;
}

//--------------------------------------------------------------------------
void AutoEncashment::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//--------------------------------------------------------------------------
QString AutoEncashment::getConfigurationName() const {
    return m_InstancePath;
}

//--------------------------------------------------------------------------
bool AutoEncashment::saveConfiguration() {
    return true;
}

//--------------------------------------------------------------------------
bool AutoEncashment::isReady() const {
    return m_IsReady;
}

//---------------------------------------------------------------------------
void AutoEncashment::show() {}

//---------------------------------------------------------------------------
void AutoEncashment::hide() {}

//---------------------------------------------------------------------------
void AutoEncashment::notify(const QString & /*aReason*/, const QVariantMap & /*aParameters*/) {}

//---------------------------------------------------------------------------
void AutoEncashment::reset(const QVariantMap & /*aParameters*/) {
    if (m_AutoEncashmentWindow) {
        m_AutoEncashmentWindow->shutdown();
        m_AutoEncashmentWindow->initialize();
    }
}

//---------------------------------------------------------------------------
QQuickItem *AutoEncashment::getWidget() const {
    // return m_MainWidget;
    // FIXME
    return nullptr;
}

//---------------------------------------------------------------------------
QVariantMap AutoEncashment::getContext() const {
    // TODO
    return QVariantMap();
}

//---------------------------------------------------------------------------
bool AutoEncashment::isValid() const {
    return m_MainWidget != 0;
}

//---------------------------------------------------------------------------
QString AutoEncashment::getError() const {
    return QString();
}

//---------------------------------------------------------------------------
