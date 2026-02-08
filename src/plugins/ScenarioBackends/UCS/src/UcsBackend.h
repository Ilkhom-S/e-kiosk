/* @file Плагин сценария для оплаты картами */
#pragma once

#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

// Plugin SDK
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IFactory.h>
#include <SDK/Plugins/PluginInitializer.h>

// PP SDK
#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Scripting/IBackendScenarioObject.h>

#include "Ucs.h"

class IApplication;

//--------------------------------------------------------------------------
namespace SDK {
namespace PaymentProcessor {
class ICore;
} // namespace PaymentProcessor

namespace Plugin {
class IEnvironment;
} // namespace Plugin
} // namespace SDK

namespace PPSDK = SDK::PaymentProcessor;

namespace Ucs {
const QString PluginName = "Ucs";

//--------------------------------------------------------------------------
class UcsBackendPlugin
    : public SDK::Plugin::IFactory<SDK::PaymentProcessor::Scripting::IBackendScenarioObject> {
public:
    UcsBackendPlugin(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
        : m_InstancePath(aInstancePath), m_Environment(aFactory) {}

public:
    /// Возвращает название плагина.
    virtual QString getPluginName() const { return Ucs::PluginName; }

    /// Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const { return QVariantMap(); }

    /// Настраивает плагин.
    virtual void setConfiguration(const QVariantMap &aParameters) { Q_UNUSED(aParameters); }

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    virtual QString getConfigurationName() const { return m_InstancePath; }

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной
    /// программы).
    virtual bool saveConfiguration() {
        return m_Environment->saveConfiguration(Ucs::PluginName, getConfiguration());
    }

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const { return true; }

    /// Возвращает список имен классов, которые создает фабрика.
    virtual QStringList getClassNames() const { return QStringList(Ucs::PluginName); }

    /// Создает класс c заданным именем.
    virtual PPSDK::Scripting::IBackendScenarioObject *create(const QString &aClassName) const {
        PPSDK::ICore *core =
            dynamic_cast<PPSDK::ICore *>(m_Environment->getInterface(PPSDK::CInterfaces::ICore));
        return Ucs::API::getInstance(core, m_Environment->getLog(PluginName)).data();
    }

private:
    QString m_InstancePath;
    SDK::Plugin::IEnvironment *m_Environment;
};

} // namespace Ucs