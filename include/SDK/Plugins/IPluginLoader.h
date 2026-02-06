/* @file Интерфейс загрузчика плагинов. */

#pragma once

#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <functional>
#include <memory>

// Plugin SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/PluginParameters.h>

namespace SDK {

namespace Plugin {

//------------------------------------------------------------------------------
/// Интерфейс загрузчика плагинов.
class IPluginLoader {
public:
    /// Добавляет каталог с плагинами.
    virtual int addDirectory(const QString &aDirectory) = 0;

    /// Возвращает список доступных плагинов.
    virtual QStringList getPluginList(const QRegularExpression &aFilter) const = 0;

    /// Возвращает список полных путей для загруженных плагинов.
    virtual QStringList getPluginPathList(const QRegularExpression &aFilter) const = 0;

    /// Создаёт плагин по заданному пути.
    virtual IPlugin *createPlugin(const QString &aInstancePath,
                                  const QString &aConfigPath = "") = 0;

    /// Создаёт плагин по заданному пути.
    virtual std::weak_ptr<IPlugin> createPluginPtr(const QString &aInstancePath,
                                                   const QString &aConfigPath = "") = 0;

    /// Возвращает параметры для данного плагина, загруженные из
    /// конфигурационного файла или внешнего хранилища.
    virtual QVariantMap getPluginInstanceConfiguration(const QString &aInstancePath,
                                                       const QString &aConfigPath) = 0;

    /// Получить описание параметров плагина.
    virtual TParameterList getPluginParametersDescription(const QString &aPath) const = 0;

    /// Удаляет плагин.
    virtual bool destroyPlugin(IPlugin *aPlugin) = 0;

    /// Удаляет плагин.
    virtual bool destroyPluginPtr(const std::weak_ptr<IPlugin> &aPlugin) = 0;

protected:
    virtual ~IPluginLoader() {}
};

//------------------------------------------------------------------------------
} // namespace Plugin
} // namespace SDK

//------------------------------------------------------------------------------
