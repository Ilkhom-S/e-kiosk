/* @file Прокси класс для получения информации из конфигов в скриптах */

#include <QtCore/QDir>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Scripting/ScriptArray.h>
#include <SDK/PaymentProcessor/Scripting/Settings.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

namespace {
const QString DefaultSkin = "default";
const QString CommonSkinDirectory = "skins";
} // namespace

//------------------------------------------------------------------------------
DealerSettings::DealerSettings(ICore *aCore) : m_Settings(nullptr) {
    // Используем reinterpret_cast вместо dynamic_cast для обхода проблемы с множественным
    // наследованием
    void *voidPtr = reinterpret_cast<void *>(
        aCore->getSettingsService()->getAdapter(CAdapterNames::DealerAdapter));
    m_Settings = reinterpret_cast<SDK::PaymentProcessor::DealerSettings *>(voidPtr);

    if (m_Settings) {
        m_PersonalSettings = m_Settings->getPersonalSettings();
    }
}

//------------------------------------------------------------------------------
bool DealerSettings::isPaymentAllowed(const QVariantMap &aParameters) const {
    return m_Settings->isCustomerAllowed(aParameters);
}

//------------------------------------------------------------------------------
QObject *
DealerSettings::getCommissions(qint64 aProvider, const QVariantMap &aParameters, double aAmount) {
    auto *result = new ScriptArray(this);

    foreach (const Commission &commission, m_Settings->getCommissions(aProvider, aParameters)) {
        // Комиссия подходит для данной суммы если нет ограничений по сумме или выполняется условие
        // "сумма
        // <= maxLimit"
        if (!commission.hasLimits() || aAmount <= commission.getMaxLimit() ||
            qFuzzyIsNull(aAmount)) {
            result->append(new SCommission(commission, result));
        }
    }

    return result;
}

//------------------------------------------------------------------------------
TerminalSettings::TerminalSettings(ICore *aCore) : m_GuiService(aCore->getGUIService()) {
    ISettingsService *settingsService = aCore->getSettingsService();
    // Используем reinterpret_cast вместо dynamic_cast для обхода проблемы с множественным
    // наследованием
    void *voidPtr =
        reinterpret_cast<void *>(settingsService->getAdapter(CAdapterNames::TerminalAdapter));
    m_TerminalSettings = reinterpret_cast<SDK::PaymentProcessor::TerminalSettings *>(voidPtr);

    if (!m_TerminalSettings) {
        qCritical() << "TerminalSettings::TerminalSettings - adapter is NULL!";
    }
}

//------------------------------------------------------------------------------
bool TerminalSettings::isItServiceProvider(qint64 aProvider, const QVariantMap &aParameters) {
    if (m_TerminalSettings->getServiceMenuPasswords().operatorId == aProvider) {
        if ((static_cast<int>(!aParameters.empty()) != 0) &&
            aParameters.value(aParameters.keys().first()).toString() ==
                m_TerminalSettings->getServiceMenuPasswords().phone) {
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
QString TerminalSettings::getCurrentSkinPath() const {
    QString name = m_GuiService->getUiSettings("ui")["skin"].toString();

    return m_TerminalSettings->getAppEnvironment().interfacePath + QDir::separator() +
           CommonSkinDirectory + QDir::separator() + (name.isEmpty() ? DefaultSkin : name);
}

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
