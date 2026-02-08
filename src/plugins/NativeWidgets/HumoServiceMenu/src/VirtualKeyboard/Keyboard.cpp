/* @file Плагин виртуальной клавиатуры */

#include "Keyboard.h"

#include <QtCore/QObject>
#include <QtWidgets/QWidget>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "KeyboardWindow.h"

namespace CKeyboard {
const QString PluginName = "VirtualKeyboard";
} // namespace CKeyboard

//--------------------------------------------------------------------------
namespace {

/// Конструктор плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new Keyboard(aFactory, aInstancePath);
}

} // namespace

REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application,
                         SDK::PaymentProcessor::CComponents::GraphicsItem,
                         CKeyboard::PluginName),
                &CreatePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                VirtualKeyboard);

//--------------------------------------------------------------------------
Keyboard::Keyboard(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : m_MainWidget(0), m_Environment(aFactory), m_InstancePath(aInstancePath), m_KeyboardWindow(0),
      m_IsReady(false) {
    SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        m_Environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    m_IsReady = core != 0;

    if (m_IsReady) {
        m_MainWidget = new QGraphicsProxyWidget();

        m_KeyboardWindow = new KeyboardWindow();
        m_KeyboardWindow->initialize();

        m_MainWidget->setWidget(m_KeyboardWindow);
        m_MainWidget->setFlag(QGraphicsItem::Item_IsFocusable, false);

        m_MainWidget->setScale(qMin(1.0,
                                    qreal(qMin(core->getGUIService()->getScreenSize(0).width() /
                                                   qreal(m_KeyboardWindow->width()),
                                               core->getGUIService()->getScreenSize(0).height() /
                                                   qreal(m_KeyboardWindow->height())))));
    }
}

//--------------------------------------------------------------------------
Keyboard::~Keyboard() {
    if (m_MainWidget) {
        m_KeyboardWindow->shutdown();
        m_MainWidget->deleteLater();
    }
}

//--------------------------------------------------------------------------
QString Keyboard::getPluginName() const {
    return CKeyboard::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap Keyboard::getConfiguration() const {
    return m_Parameters;
}

//--------------------------------------------------------------------------
void Keyboard::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//--------------------------------------------------------------------------
QString Keyboard::getConfigurationName() const {
    return m_InstancePath;
}

//--------------------------------------------------------------------------
bool Keyboard::saveConfiguration() {
    return true;
}

//--------------------------------------------------------------------------
bool Keyboard::isReady() const {
    return m_IsReady;
}

//--------------------------------------------------------------------------
void Keyboard::show() {
    if (m_MainWidget) {
        m_MainWidget->show();
    }
}

//--------------------------------------------------------------------------
void Keyboard::reset(const QVariantMap &aParameters) {
    Q_UNUSED(aParameters)
}

//--------------------------------------------------------------------------
void Keyboard::hide() {
    if (m_MainWidget) {
        m_MainWidget->hide();
    }
}

//--------------------------------------------------------------------------
void Keyboard::notify(const QString &aReason, const QVariantMap &aParameters) {
    Q_UNUSED(aReason)
    Q_UNUSED(aParameters)
}

//--------------------------------------------------------------------------
bool Keyboard::isValid() const {
    return true;
}

//--------------------------------------------------------------------------
QString Keyboard::getError() const {
    return QString();
}

//--------------------------------------------------------------------------
QQuickItem *Keyboard::getWidget() const {
    return 0;
}

//--------------------------------------------------------------------------
QVariantMap Keyboard::getContext() const {
    return QVariantMap();
}

//--------------------------------------------------------------------------