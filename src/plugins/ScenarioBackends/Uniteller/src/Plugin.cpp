/**
 * @file Плагин виртуального драйвера Uniteller.
 */

// Plugin SDK
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/InteractionTypes.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/Plugins/DevicePluginBase.h"
#include "UnitellerDevice.h"

using namespace SDK::Driver;
using namespace SDK::Plugin;

//------------------------------------------------------------------------------
static IPlugin *CreatePlugin(IEnvironment *aEnvironment, const QString &aInstancePath) {
    auto plugin =
        new DevicePluginBase<UnitellerDevice>(Uniteller::ModelName, aEnvironment, aInstancePath);

    void *voidPtr = reinterpret_cast<void *>(
        aEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
    plugin->setCore(reinterpret_cast<SDK::PaymentProcessor::ICore *>(voidPtr));
    plugin->setLog(aEnvironment->getLog(Uniteller::LogName));

    return plugin;
}

TParameterList defaultParameters() {
    return TParameterList() << SPluginParameter(CHardwareSDK::ModelName,
                                                false,
                                                CPPT::ModelName,
                                                QString(),
                                                Uniteller::ModelName,
                                                QStringList() << Uniteller::ModelName,
                                                true);
}

// Регистрация плагина.
REGISTER_DRIVER_WITH_PARAMETERS(UnitellerDevice, &CreatePlugin, &defaultParameters)

//--------------------------------------------------------------------------------------
