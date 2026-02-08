/* @file Прокси-класс для работы с рекламным контентом в скриптах. */

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Scripting/AdService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
AdService::AdService(ICore *aCore) : m_Core(aCore), m_AdSource(nullptr) {}

//------------------------------------------------------------------------------
void AdService::addEvent(const QString &aEvent, const QVariantMap &aParameters) {
    if (getAdSource()) {
        getAdSource()->addEvent(aEvent, aParameters);
    }
}

//------------------------------------------------------------------------------
QString AdService::getBanner(const QString &aBanner) {
    return getContent(aBanner);
}

//------------------------------------------------------------------------------
QString AdService::getReceiptHeader() {
    return {};
}

//------------------------------------------------------------------------------
QString AdService::getReceiptFooter() {
    return getContent("receipt");
}

//------------------------------------------------------------------------------
SDK::GUI::IAdSource *AdService::getAdSource() {
    if (!m_AdSource) {
        m_AdSource = m_Core->getGUIService()->getAdSource();
    }

    return m_AdSource;
}

//------------------------------------------------------------------------------
QString AdService::getContent(const QString &aName) {
    return getAdSource() ? getAdSource()->getContent(aName) : "";
}

//------------------------------------------------------------------------------

} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
// Definitions for CAdService constants
namespace SDK {
namespace PaymentProcessor {
namespace Scripting {
namespace CAdService {
extern const char DefaultBanner[] = "banner";
} // namespace CAdService
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
