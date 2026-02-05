/* @file Виджет HumoService */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTime>
#include <QtWidgets/QGraphicsScene>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginInitializer.h>

// System
#include "Backend/HumoServiceBackend.h"
#include "MessageBox/MessageBox.h"

// Project
#include "HumoServiceMenu.h"

namespace CHumoServiceMenu
{
    const QString PluginName = "HumoServiceMenu";
} // namespace CHumoServiceMenu

namespace PPSDK = SDK::PaymentProcessor;

//--------------------------------------------------------------------------
namespace
{

    /// Конструктор плагина.
    SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    {
        return new HumoServiceMenu(aFactory, aInstancePath);
    }

} // namespace

typedef QMap<int, bool> map_type;
Q_DECLARE_METATYPE(map_type)

static SDK::Plugin::TParameterList EnumerateParameters()
{
    return SDK::Plugin::TParameterList() << SDK::Plugin::SPluginParameter("columnVisibility",
                                                                          SDK::Plugin::SPluginParameter::MultiSet, true,
                                                                          "columnVisibility", QString(), QVariantList())

                                         << SDK::Plugin::SPluginParameter(SDK::Plugin::Parameters::Singleton,
                                                                          SDK::Plugin::SPluginParameter::Bool, false,
                                                                          SDK::Plugin::Parameters::Singleton, QString(),
                                                                          true, QVariantMap(), true);
}

REGISTER_PLUGIN_WITH_PARAMETERS(makePath(SDK::PaymentProcessor::Application,
                                         SDK::PaymentProcessor::CComponents::GraphicsItem,
                                         CHumoServiceMenu::PluginName),
                                &CreatePlugin, &EnumerateParameters, HumoServiceMenu);

//--------------------------------------------------------------------------
HumoServiceMenu::HumoServiceMenu(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : mEnvironment(aFactory), mInstancePath(aInstancePath), mMainHumoServiceMenuWindow(nullptr), mIsReady(true)
{
    SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    if (core)
    {
        mBackend = QSharedPointer<HumoServiceBackend>(
            new HumoServiceBackend(mEnvironment, mEnvironment->getLog(CHumoServiceBackend::LogName)));
    }
    else
    {
        mEnvironment->getLog(CHumoServiceBackend::LogName)->write(LogLevel::Error, "Failed to get ICore");
    }

    mMainHumoServiceMenuWindow = new MainHumoServiceMenuWindow(mBackend.data());
    mMainHumoServiceMenuWindow->initialize();
}

//--------------------------------------------------------------------------
HumoServiceMenu::~HumoServiceMenu()
{
    if (mMainHumoServiceMenuWindow)
    {
        saveConfiguration();
    }
}

//--------------------------------------------------------------------------
QString HumoServiceMenu::getPluginName() const
{
    return CHumoServiceMenu::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap HumoServiceMenu::getConfiguration() const
{
    return mBackend->getConfiguration();
}

//--------------------------------------------------------------------------
void HumoServiceMenu::setConfiguration(const QVariantMap &aParameters)
{
    mBackend->setConfiguration(aParameters);
}

//--------------------------------------------------------------------------
QString HumoServiceMenu::getConfigurationName() const
{
    return mInstancePath;
}

//--------------------------------------------------------------------------
bool HumoServiceMenu::saveConfiguration()
{
    return mEnvironment->saveConfiguration(mInstancePath, mBackend->getConfiguration());
}

//--------------------------------------------------------------------------
bool HumoServiceMenu::isReady() const
{
    return mIsReady;
}

//---------------------------------------------------------------------------
void HumoServiceMenu::show()
{
    GUI::MessageBox::setParentWidget(mMainHumoServiceMenuWindow);

    mBackend->startHeartbeat();
}

//---------------------------------------------------------------------------
void HumoServiceMenu::hide()
{
    GUI::MessageBox::hide();
    mBackend->stopHeartbeat();
}

//---------------------------------------------------------------------------
void HumoServiceMenu::notify(const QString &aReason, const QVariantMap &aParameters)
{
    if (aReason.toInt() == PPSDK::EEventType::TryStopScenario)
    {
        mMainHumoServiceMenuWindow->closeHumoServiceMenu(true, QObject::tr("#need_restart_application"), true);
    }
    else
    {
        GUI::MessageBox::emitSignal(aParameters);
    }
}

//---------------------------------------------------------------------------
void HumoServiceMenu::reset(const QVariantMap & /*aParameters*/)
{
    if (mMainHumoServiceMenuWindow)
    {
        mMainHumoServiceMenuWindow->shutdown();
        mMainHumoServiceMenuWindow->initialize();
    }
}

//---------------------------------------------------------------------------
QQuickItem *HumoServiceMenu::getWidget() const
{
    return nullptr;
}

//---------------------------------------------------------------------------
QWidget *HumoServiceMenu::getNativeWidget() const
{
    return mMainHumoServiceMenuWindow;
}

//---------------------------------------------------------------------------
QVariantMap HumoServiceMenu::getContext() const
{
    // TODO
    return QVariantMap();
}

//---------------------------------------------------------------------------
bool HumoServiceMenu::isValid() const
{
    return true;
}

//---------------------------------------------------------------------------
QString HumoServiceMenu::getError() const
{
    return QString();
}

//---------------------------------------------------------------------------