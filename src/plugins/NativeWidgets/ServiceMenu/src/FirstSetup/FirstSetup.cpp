/* @file Виджет первоначальной настройки терминала */

#include "FirstSetup.h"

#include <QtWidgets/QGraphicsScene>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "Backend/ServiceMenuBackend.h"
#include "GUI/MessageBox/MessageBox.h"

namespace CFirstSetup {
const QString PluginName = "FirstSetup";
} // namespace CFirstSetup

//--------------------------------------------------------------------------
namespace {

/// Конструктор плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new FirstSetup(aFactory, aInstancePath);
}

} // namespace

REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application,
                         SDK::PaymentProcessor::CComponents::GraphicsItem,
                         CFirstSetup::PluginName),
                &CreatePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                FirstSetup);

//--------------------------------------------------------------------------
FirstSetup::FirstSetup(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : m_MainWidget(0), m_Environment(aFactory), m_InstancePath(aInstancePath), m_IsReady(false) {
    SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        m_Environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    if (core) {
        m_Backend = QSharedPointer<ServiceMenuBackend>(new ServiceMenuBackend(
            m_Environment, m_Environment->getLog(CServiceMenuBackend::LogName)));
    } else {
        m_Environment->getLog("ServiceMenu")->write(LogLevel::Error, "Failed to get ICore");
    }

    m_IsReady = core != 0;

    if (m_IsReady) {
        m_MainWidget = new QGraphicsProxyWidget();

        m_WizardFrame = new WizardFrame(m_Backend.data());
        m_WizardFrame->initialize();
        m_WizardFrame->setStatus(QObject::tr("#humo_copyright"));

        m_MainWidget->setWidget(m_WizardFrame);
        m_MainWidget->setScale(qMin(
            core->getGUIService()->getScreenSize(0).width() / qreal(m_WizardFrame->width()),
            core->getGUIService()->getScreenSize(0).height() / qreal(m_WizardFrame->height())));

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
FirstSetup::~FirstSetup() {
    if (m_MainWidget) {
        m_WizardFrame->shutdown();
        m_MainWidget->deleteLater();
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
    // return m_MainWidget;
    // FIXME
    return nullptr;
}

//---------------------------------------------------------------------------
QVariantMap FirstSetup::getContext() const {
    // TODO
    return QVariantMap();
}

//---------------------------------------------------------------------------
bool FirstSetup::isValid() const {
    return m_MainWidget != 0;
}

//---------------------------------------------------------------------------
QString FirstSetup::getError() const {
    return QString();
}

//---------------------------------------------------------------------------
