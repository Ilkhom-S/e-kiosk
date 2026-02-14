/* @file Менеджер событий. */

#include "Services/EventService.h"

#include <QtCore/QMetaType>
#include <QtCore/QSet>
#include <QtCore/QString>

#include <SDK/PaymentProcessor/Core/EventTypes.h>

#include "Services/ServiceNames.h"
#include "System/IApplication.h"

namespace PP = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
EventService *EventService::instance(IApplication *aApplication) {
    return dynamic_cast<EventService *>(
        aApplication->getCore()->getService(CServices::EventService));
}

//---------------------------------------------------------------------------
EventService::EventService() {
    qRegisterMetaType<SDK::PaymentProcessor::Event>("SDK::PaymentProcessor::Event");
}

//---------------------------------------------------------------------------
EventService::~EventService() = default;

//---------------------------------------------------------------------------
bool EventService::initialize() {
    return true;
}

//------------------------------------------------------------------------------
void EventService::finishInitialize() {}

//---------------------------------------------------------------------------
bool EventService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool EventService::shutdown() {
    return true;
}

//---------------------------------------------------------------------------
QString EventService::getName() const {
    return CServices::EventService;
}

//---------------------------------------------------------------------------
const QSet<QString> &EventService::getRequiredServices() const {
    static QSet<QString> requiredResources;
    return requiredResources;
}

//---------------------------------------------------------------------------
QVariantMap EventService::getParameters() const {
    return {};
}

//---------------------------------------------------------------------------
void EventService::resetParameters(const QSet<QString> & /*aParameters*/) {}

//---------------------------------------------------------------------------
void EventService::sendEvent(const SDK::PaymentProcessor::Event &aEvent) {
    emit event(aEvent);
}

//---------------------------------------------------------------------------
void EventService::sendEvent(SDK::PaymentProcessor::EEventType::Enum aType, const QVariant &aData) {
    emit event(SDK::PaymentProcessor::Event(aType, QString(), aData));
}

//---------------------------------------------------------------------------
void EventService::subscribe(const QObject *aObject, const char *aSlot) {
    connect(this,
            SIGNAL(event(const SDK::PaymentProcessor::Event &)),
            aObject,
            aSlot,
            Qt::QueuedConnection);
}

//---------------------------------------------------------------------------
void EventService::unsubscribe(const QObject *aObject, const char *aSlot) {
    disconnect(this, SIGNAL(event(const SDK::PaymentProcessor::Event &)), aObject, aSlot);
}

//---------------------------------------------------------------------------
