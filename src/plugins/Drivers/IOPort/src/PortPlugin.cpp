/* @file Плагин с драйверами портов. */

#include <Hardware/IOPorts/TCPPort.h>

#include "Hardware/IOPorts/AsyncSerialPort.h"
#include "Hardware/IOPorts/LibUSBPort.h"
#include "Hardware/IOPorts/USBPort.h"
#include "Hardware/Plugins/CommonParameters.h"

namespace PortParameterTranslations {
static const char Name[] = QT_TRANSLATE_NOOP("PortParameters", "PortParameters#name");

namespace COM {
static const char BaudRate[] =
    QT_TRANSLATE_NOOP("Com_PortParameters", "Com_PortParameters#com_baud_rate");
static const char Parity[] =
    QT_TRANSLATE_NOOP("Com_PortParameters", "Com_PortParameters#com_parity");
static const char ByteSize[] =
    QT_TRANSLATE_NOOP("Com_PortParameters", "Com_PortParameters#com_byte_size");
static const char RTS[] = QT_TRANSLATE_NOOP("Com_PortParameters", "Com_PortParameters#com_rts");
static const char DTR[] = QT_TRANSLATE_NOOP("Com_PortParameters", "Com_PortParameters#com_dtr");
} // namespace COM

namespace TCP {
static const char Address[] = QT_TRANSLATE_NOOP("PortParameters", "PortParameters#tcp_address");
static const char Number[] = QT_TRANSLATE_NOOP("PortParameters", "PortParameters#tcp_number");
} // namespace TCP
}; // namespace PortParameterTranslations

namespace PortPT = PortParameterTranslations;

using namespace SDK::Plugin;

//------------------------------------------------------------------------------
/// Конструкторы плагинов.
template <class T> IPlugin *CreatePlugin(IEnvironment *aEnvironment, const QString &aInstancePath) {
    return new DevicePluginBase<T>("COM (RS232) asynchronous port", aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
#ifdef Q_OS_WIN32
template <>
IPlugin *CreatePlugin<USBPort>(IEnvironment *aEnvironment, const QString &aInstancePath) {
    return new DevicePluginBase<USBPort>("USB port", aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <>
IPlugin *CreatePlugin<TCPPort>(IEnvironment *aEnvironment, const QString &aInstancePath) {
    return new DevicePluginBase<TCPPort>("TCP port", aEnvironment, aInstancePath);
}
#endif

//------------------------------------------------------------------------------
TParameterList TCPParameters() {
    QVariantMap addressMask;
    addressMask.insert(CHardwareSDK::Mask, "999.999.999.999;_");

    QVariantMap portNumberMask;
    portNumberMask.insert(CHardwareSDK::Mask, "99999;_");

    return TParameterList() << SPluginParameter(CHardwareSDK::Port::TCP::IP,
                                                SPluginParameter::Text,
                                                false,
                                                PortPT::TCP::Address,
                                                QString(),
                                                "192.168.137.015",
                                                addressMask)
                            << SPluginParameter(CHardwareSDK::Port::TCP::Number,
                                                SPluginParameter::Text,
                                                false,
                                                PortPT::TCP::Number,
                                                QString(),
                                                "07778",
                                                portNumberMask);
}

//------------------------------------------------------------------------------
TParameterList COMParameters() {
    // TODO: сделать отображаемые в сервисном меню параметры понятными пользователю
    return TParameterList() << SPluginParameter(CHardwareSDK::System_Name,
                                                false,
                                                PortPT::Name,
                                                QString(),
                                                QVariant(),
                                                AsyncSerialPort::enumerateSystem_Names())

                            << SPluginParameter(CHardware::Port::COM::BaudRate,
                                                false,
                                                PortPT::COM::BaudRate,
                                                QString(),
                                                QVariant(),
                                                QStringList() << "4800" << "9600" << "14400"
                                                              << "19200" << "38400"
                                                              << "57600" << "115200")
                            << SPluginParameter(CHardware::Port::COM::Parity,
                                                false,
                                                PortPT::COM::Parity,
                                                QString(),
                                                "0",
                                                QStringList() << "0" << "1" << "2")
                            << SPluginParameter(CHardware::Port::COM::ByteSize,
                                                false,
                                                PortPT::COM::ByteSize,
                                                QString(),
                                                "8",
                                                QStringList() << "8" << "7")
                            << SPluginParameter(CHardware::Port::COM::RTS,
                                                false,
                                                PortPT::COM::RTS,
                                                QString(),
                                                "1",
                                                QStringList() << "0" << "1" << "2" << "3")
                            << SPluginParameter(CHardware::Port::COM::DTR,
                                                false,
                                                PortPT::COM::DTR,
                                                QString(),
                                                "0",
                                                QStringList() << "0" << "1" << "2");
}

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
COMMON_DRIVER(AsyncSerialPort, &COMParameters)
#ifdef Q_OS_WIN32
COMMON_DRIVER(USBPort, &PluginInitializer::emptyParameterList)
// COMMON_DRIVER(LibUSBPort, &PluginInitializer::emptyParameterList)
COMMON_DRIVER(TCPPort, &TCPParameters)
#endif
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
