/* @file Прокси класс для работы с объектами ядра в скриптах. */

#include <QtCore/QCryptographicHash>

#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
Core::Core(ICore *aCore)
    : m_Core(aCore), m_UserProperties(aCore->getUserProperties()), m_PaymentService(aCore),
      m_FundsService(aCore), m_PrinterService(aCore), m_NetworkService(aCore), m_GUIService(aCore),
      m_AdService(aCore), m_DeviceService(aCore), m_Settings(aCore), m_HID(aCore) {
    ISettingsService *settingsService = aCore->getSettingsService();
    auto *terminalSettings = dynamic_cast<SDK::PaymentProcessor::TerminalSettings *>(
        settingsService->getAdapter(CAdapterNames::TerminalAdapter));

    m_PaymentService.setForcePayOffline(
        terminalSettings->getCommonSettings().skipCheckWhileNetworkError);

    connect(&m_UserProperties, SIGNAL(updated()), SIGNAL(userPropertiesUpdated()));
}

//------------------------------------------------------------------------------
void Core::installService(const QString &aName, QObject *aService) {
    m_Services[aName] = aService;
}

//------------------------------------------------------------------------------
void Core::setLog(ILog *aLog) {
    m_Log.setLog(aLog);
    m_HID.setLog(aLog);
}

//------------------------------------------------------------------------------
ICore *Core::getCore() const {
    return m_Core;
}

//------------------------------------------------------------------------------
QObject *Core::getService(const QString &aName) {
    if (m_Services.contains(aName)) {
        return m_Services[aName];
    }
    return 0;
}

//------------------------------------------------------------------------------
QObject *Core::getPayment() {
    return &m_PaymentService;
}

//------------------------------------------------------------------------------
QObject *Core::getPrinter() {
    return &m_PrinterService;
}

//------------------------------------------------------------------------------
QObject *Core::getCharge() {
    return &m_FundsService;
}

//------------------------------------------------------------------------------
QObject *Core::getHID() {
    return &m_HID;
}

//------------------------------------------------------------------------------
QObject *Core::getNetwork() {
    return &m_NetworkService;
}

//------------------------------------------------------------------------------
QObject *Core::getGraphics() {
    return &m_GUIService;
}

//------------------------------------------------------------------------------
QObject *Core::getAd() {
    return &m_AdService;
}

//------------------------------------------------------------------------------
QObject *Core::getHardware() {
    return &m_DeviceService;
}

//------------------------------------------------------------------------------
QObject *Core::getSettings() {
    return &m_Settings;
}

//------------------------------------------------------------------------------
QObject *Core::getLog() {
    return &m_Log;
}

//------------------------------------------------------------------------------
QObject *Core::getUserProperties() {
    return &m_UserProperties;
}

//------------------------------------------------------------------------------
void Core::postEvent(int aEvent, QVariant aParameters) {
    QMetaObject::invokeMethod(this,
                              "onPostEvent",
                              Qt::QueuedConnection,
                              Q_ARG(int, aEvent),
                              Q_ARG(QVariant, aParameters));
}

//------------------------------------------------------------------------------
void Core::onPostEvent(int aEvent, QVariant aParameters) const {
    IEventService *service = m_Core->getEventService();

    if (service) {
        service->sendEvent(Event(aEvent, "ScriptingCore::postEvent", aParameters));
    }
}

//------------------------------------------------------------------------------
QString Core::getMD5Hash(const QString &aSource) {
    return QCryptographicHash::hash(aSource.toLatin1(), QCryptographicHash::Md5).toHex();
}

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
// Definitions for CProxyNames constants
namespace SDK {
namespace PaymentProcessor {
namespace Scripting {
namespace CProxyNames {
extern const char Core[] = "Core";
extern const char EventType[] = "EventType";
extern const char PaymentStep[] = "PaymentStep";
extern const char PaymentStepResult[] = "PaymentStepResult";
extern const char Payment[] = "Payment";
} // namespace CProxyNames
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
