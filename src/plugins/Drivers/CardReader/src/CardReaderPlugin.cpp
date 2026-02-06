/* @file Плагин с драйверами кардридеров. */

#include "Hardware/Plugins/CommonParameters.h"

#ifdef Q_OS_WIN32
#include <Hardware/CardReaders/CreatorReader.h>

#include "../../../../modules/Hardware/Cardreaders/src/IDTech/IDTechReader.h"
#endif

#ifdef Q_OS_WIN32
#include "MifareReader.h"
#endif

using namespace SDK::Plugin;
using namespace SDK::Driver;

//------------------------------------------------------------------------------
template <class T> IPlugin *CreatePlugin(IEnvironment *aEnvironment, const QString &aInstancePath) {
    return new DevicePluginBase<T>("Card readers", aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <class T> TParameterList EnumParameters() {
    return createNamedList<T>(T::getModelList(), CComponents::CardReader);
}

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
#ifdef Q_OS_WIN32
SIMPLE_COMMON_DRIVER(MifareReader, EnumParameters)
#endif
#ifdef Q_OS_WIN32
SIMPLE_COMMON_DRIVER(CreatorReader, EnumParameters)
#endif
// SIMPLE_COMMON_DRIVER(IDTechReader, EnumParameters)
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
