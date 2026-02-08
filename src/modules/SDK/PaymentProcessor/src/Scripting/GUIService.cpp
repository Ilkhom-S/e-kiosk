/* @file Прокси класс для работы с графическим движком в скриптах. */

#include <QtCore/QRect>

#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Scripting/GUIService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
GUIService::GUIService(ICore *aCore) : m_Core(aCore), m_GUIService(m_Core->getGUIService()) {}

//------------------------------------------------------------------------------
bool GUIService::show(const QString &aWidget, const QVariantMap &aParameters) {
    if (m_TopWidgetName != aWidget) {
        m_TopWidgetName = aWidget;

        emit topSceneChange();
    }

    return m_GUIService->show(aWidget, aParameters);
}

//------------------------------------------------------------------------------
bool GUIService::showPopup(const QString &aWidget, const QVariantMap &aParameters) {
    return m_GUIService->showPopup(aWidget, aParameters);
}

//------------------------------------------------------------------------------
bool GUIService::hidePopup(const QVariantMap &aParameters) {
    return m_GUIService->hidePopup(aParameters);
}

//------------------------------------------------------------------------------
void GUIService::notify(const QString &aEvent, const QVariantMap &aParameters) {
    return m_GUIService->notify(aEvent, aParameters);
}

//------------------------------------------------------------------------------
void GUIService::reset() {
    m_GUIService->reset();
}

//------------------------------------------------------------------------------
void GUIService::reload(const QVariantMap &aParams) {
    emit skinReload(aParams);
}

//------------------------------------------------------------------------------
QString GUIService::getTopScene() const {
    return m_TopWidgetName;
}

//------------------------------------------------------------------------------
QVariantMap GUIService::getParametersUI() const {
    return m_GUIService->getUiSettings("ui");
}

//------------------------------------------------------------------------------
QVariantMap GUIService::getParametersAd() const {
    return m_GUIService->getUiSettings("ad");
}

//------------------------------------------------------------------------------
bool GUIService::isDisabled() const {
    return m_GUIService->isDisabled();
}

//------------------------------------------------------------------------------
int GUIService::getWidth() const {
    return m_GUIService->getScreenSize(0).width();
}

//------------------------------------------------------------------------------
int GUIService::getHeight() const {
    return m_GUIService->getScreenSize(0).height();
}

//------------------------------------------------------------------------------

} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
