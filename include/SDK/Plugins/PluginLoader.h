/* @file Загрузчик плагинов. */

#pragma once

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPluginLoader>
#include <QtCore/QRecursiveMutex>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>

// Plugin SDK
#include <SDK/Plugins/IKernel.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/IPluginLoader.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Загрузчик плагинов.
class PluginLoader : public IPluginLoader {
public:
    PluginLoader(IKernel *aKernel);
    virtual ~PluginLoader();

#pragma region IPluginLoader interface

    /// Добавляет каталог с плагинами.
    virtual int addDirectory(const QString &aDirectory);

    /// Возвращает список доступных плагинов.
    virtual QStringList getPluginList(const QRegularExpression &aFilter) const;

    /// Возвращает список путей загруженных плагинов.
    virtual QStringList getPluginPathList(const QRegularExpression &aFilter) const;

    /// Создаёт плагин по заданному пути.
    virtual IPlugin *createPlugin(const QString &aInstancePath,
                                  const QString &aConfigInstancePath = "");

    /// Создаёт плагин по заданному пути.
    virtual std::weak_ptr<IPlugin> createPluginPtr(const QString &aInstancePath,
                                                   const QString &aConfigInstancePath = "");

    virtual TParameterList getPluginParametersDescription(const QString &aPath) const;

    /// Возвращает параметры для данного плагина, загруженные из конфигурационного файла или
    /// внешнего хранилища.
    virtual QVariantMap getPluginInstanceConfiguration(const QString &aInstancePath,
                                                       const QString &aConfigPath);

    /// Удаляет плагин.
    virtual bool destroyPlugin(IPlugin *aPlugin);

    /// Удаляет плагин.
    virtual bool destroyPluginPtr(const std::weak_ptr<IPlugin> &aPlugin);

#pragma endregion

private:
    /// Интерфейс приложения для плагинов.
    IKernel *m_Kernel;

    /// Каталоги для поиска плагинов.
    QStringList m_Directories;

    /// Загрузчики библиотек. Нужны для выгрузки библиотек.
    QList<QSharedPointer<QPluginLoader>> m_Libraries;

    /// Список доступных плагинов.
    QMap<QString, SDK::Plugin::IPluginFactory *> m_Plugins;

    /// Список созданных плагинов.
    QMap<SDK::Plugin::IPlugin *, SDK::Plugin::IPluginFactory *> m_CreatedPlugins;

    /// Список созданных плагинов.
    typedef std::shared_ptr<IPlugin> TPluginPtr;
    typedef std::map<TPluginPtr, SDK::Plugin::IPluginFactory *> TPluginMapPtr;
    TPluginMapPtr m_CreatedPluginsPtr;

    /// Синхронизация создания/удаления плагина.
    mutable QRecursiveMutex m_AccessMutex;
};

} // namespace Plugin
} // namespace SDK

//------------------------------------------------------------------------------
