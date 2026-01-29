/* @file Плагин драйвера GSM модема. */

// System
#include "../../../../modules/Hardware/Modems/src/ATModem/ATGSMModem.h"
#include "Hardware/Plugins/CommonParameters.h"

using namespace SDK::Plugin;

//------------------------------------------------------------------------------
TParameterList EnumParameters()
{
    return createNamedList<ATGSMModem>("GSM AT compatible modem");
}

// Регистрация плагина.
REGISTER_DRIVER("Modem", ATGSMModem, &EnumParameters);

//-----------------------------------------------------------------------------
