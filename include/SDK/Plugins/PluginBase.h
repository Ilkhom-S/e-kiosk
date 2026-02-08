/* @file Базовый плагин. */

#include <QtCore/QString>
#include <QtCore/QVariantMap>

#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/PluginInitializer.h>

template <class T> class PluginBase : public SDK::Plugin::IPlugin, public T {
public:
    PluginBase(const QString &aName,
               SDK::Plugin::IEnvironment *aEnvironment,
               const QString &aInstancePath)
        : m_InstanceName(aInstancePath), m_Environment(aEnvironment), m_PluginName(aName) {
        setLog(aEnvironment->getLog(aName));
    }

    virtual ~PluginBase() {}

    /// Возвращает название плагина.
    virtual QString getPluginName() const { return m_PluginName; }

    /// Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const { return QVariantMap(); }

    /// Настраивает плагин.
    virtual void setConfiguration(const QVariantMap & /*aParameters*/) {}

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или
    /// хранилище прикладной программы).
    virtual bool saveConfiguration() { return true; }

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    virtual QString getConfigurationName() const { return m_InstanceName; }

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const { return true; }

private:
    QString m_PluginName;
    QString m_InstanceName;
    SDK::Plugin::IEnvironment *m_Environment;
};

//--------------------------------------------------------------------------------
