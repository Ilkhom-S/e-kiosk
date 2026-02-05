/* @file Виджет первоначальной настройки терминала */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QGraphicsScene>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginInitializer.h>

// System
#include "Backend/HumoServiceBackend.h"
#include "GUI/MessageBox/MessageBox.h"

// Project
#include "FirstSetup.h"

namespace CFirstSetup
{
    const QString PluginName = "FirstSetup";
} // namespace CFirstSetup

//--------------------------------------------------------------------------
namespace
{

    /// Конструктор плагина.
    SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    {
        return new FirstSetup(aFactory, aInstancePath);
    }

} // namespace

REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::GraphicsItem,
                         CFirstSetup::PluginName),
                &CreatePlugin, &SDK::Plugin::PluginInitializer::emptyParameterList, FirstSetup);

//--------------------------------------------------------------------------
FirstSetup::FirstSetup(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : mMainWidget(0), mEnvironment(aFactory), mInstancePath(aInstancePath), mIsReady(false)
{
    SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    if (core)
    {
        mBackend = QSharedPointer<HumoServiceBackend>(
            new HumoServiceBackend(mEnvironment, mEnvironment->getLog("HumoService")));
    }
    else
    {
        mEnvironment->getLog("HumoService")->write(LogLevel::Error, "Failed to get ICore");
    }

    mIsReady = core != 0;

    if (mIsReady)
    {
        mMainWidget = new QGraphicsProxyWidget();

        mWizardFrame = new WizardFrame(mBackend.data());
        mWizardFrame->initialize();
        mWizardFrame->setStatus(QObject::tr("#humo_copyright"));

        mMainWidget->setWidget(mWizardFrame);
        mMainWidget->setScale(qMin(core->getGUIService()->getScreenSize(0).width() / qreal(mWizardFrame->width()),
                                   core->getGUIService()->getScreenSize(0).height() / qreal(mWizardFrame->height())));

        qreal newWidgetWidth = core->getGUIService()->getScreenSize(0).width() / mMainWidget->scale();
        mMainWidget->setMinimumWidth(newWidgetWidth);
        mMainWidget->setMaximumWidth(newWidgetWidth);

        qreal newWidgetHeight = core->getGUIService()->getScreenSize(0).height() / mMainWidget->scale();
        mMainWidget->setMinimumHeight(newWidgetHeight);
        mMainWidget->setMaximumHeight(newWidgetHeight);
    }
}

//--------------------------------------------------------------------------
FirstSetup::~FirstSetup()
{
    if (mMainWidget)
    {
        mWizardFrame->shutdown();
        mMainWidget->deleteLater();
    }
}

//--------------------------------------------------------------------------
QString FirstSetup::getPluginName() const
{
    return CFirstSetup::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap FirstSetup::getConfiguration() const
{
    return mParameters;
}

//--------------------------------------------------------------------------
void FirstSetup::setConfiguration(const QVariantMap &aParameters)
{
    mParameters = aParameters;
}

//--------------------------------------------------------------------------
QString FirstSetup::getConfigurationName() const
{
    return mInstancePath;
}

//--------------------------------------------------------------------------
bool FirstSetup::saveConfiguration()
{
    return true;
}

//--------------------------------------------------------------------------
bool FirstSetup::isReady() const
{
    return mIsReady;
}

//--------------------------------------------------------------------------
void FirstSetup::show()
{
    if (mMainWidget)
    {
        mMainWidget->show();
    }
}

//--------------------------------------------------------------------------
void FirstSetup::reset(const QVariantMap &aParameters)
{
    Q_UNUSED(aParameters)
}

//--------------------------------------------------------------------------
void FirstSetup::hide()
{
    if (mMainWidget)
    {
        mMainWidget->hide();
    }
}

//--------------------------------------------------------------------------
void FirstSetup::notify(const QString &aReason, const QVariantMap &aParameters)
{
    Q_UNUSED(aReason)
    Q_UNUSED(aParameters)
}

//--------------------------------------------------------------------------
bool FirstSetup::isValid() const
{
    return true;
}

//--------------------------------------------------------------------------
QString FirstSetup::getError() const
{
    return QString();
}

//--------------------------------------------------------------------------
QQuickItem *FirstSetup::getWidget() const
{
    return 0;
}

//--------------------------------------------------------------------------
QVariantMap FirstSetup::getContext() const
{
    return QVariantMap();
}

//--------------------------------------------------------------------------