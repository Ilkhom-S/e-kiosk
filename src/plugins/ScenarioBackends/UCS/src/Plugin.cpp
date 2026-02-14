/**
 * @file Плагин виртуального драйвера UCS.
 */

// Plugin SDK
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/InteractionTypes.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/Plugins/DevicePluginBase.h"
#include "UcsDevice.h"

using namespace SDK::Driver;
using namespace SDK::Plugin;

//------------------------------------------------------------------------------
static IPlugin *CreatePlugin(IEnvironment *aEnvironment, const QString &aInstancePath) {
    auto plugin = new DevicePluginBase<UcsDevice>(Ucs::ModelName, aEnvironment, aInstancePath);

    void *voidPtr = reinterpret_cast<void *>(
        aEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
    plugin->setCore(reinterpret_cast<SDK::PaymentProcessor::ICore *>(voidPtr));
    plugin->setLog(aEnvironment->getLog(Ucs::LogName));

    return plugin;
}

TParameterList defaultParameters() {
    return TParameterList() << SPluginParameter(CHardwareSDK::ModelName,
                                                false,
                                                CPPT::ModelName,
                                                QString(),
                                                Ucs::ModelName,
                                                QStringList() << Ucs::ModelName,
                                                true);
}

// Регистрация плагина.
REGISTER_DRIVER_WITH_PARAMETERS(UcsDevice, &CreatePlugin, &defaultParameters)

//--------------------------------------------------------------------------------------
