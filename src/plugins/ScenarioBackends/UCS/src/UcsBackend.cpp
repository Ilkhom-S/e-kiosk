/**
 * @file Плагин сценария для оплаты картами
 */

#include "UcsBackend.h"

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QPair>
#include <QtCore/QSettings>

#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/PaymentProcessor/Scripting/PrinterService.h>

#include "API.h"

namespace {
/// Конструктор плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new Ucs::UcsBackendPlugin(aFactory, aInstancePath);
}
} // namespace

REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application,
                                      PPSDK::CComponents::ScriptFactory,
                                      Ucs::PluginName),
                &CreatePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                UcsBackend);

//---------------------------------------------------------------------------
