/* @file Плагин драйвера GSM модема. */

#include "../../../../modules/Hardware/Modems/src/ATModem/ATGSMModem.h"
#include "Hardware/Plugins/CommonParameters.h"

using namespace SDK::Plugin;

//------------------------------------------------------------------------------
TParameterList enumParameters() {
    return createNamedList<ATGSMModem>("GSM AT compatible modem");
}

// Регистрация плагина.
REGISTER_DRIVER("Modem", ATGSMModem, &enumParameters);

//-----------------------------------------------------------------------------
