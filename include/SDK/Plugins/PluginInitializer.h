/* @file Класс для регистрации плагина в фабрике. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QVector>

// Plugin SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/PluginParameters.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Список плагинов в библиотеке. Каждая библиотека плагинов создаёт глобальный экземпляр
/// данного класса, который сохраняет путь к плагину в статическом списке. Затем этот список
/// используется фабрикой.

class PluginInitializer {
public:
    /// Конструктор плагина.
    typedef IPlugin *(*TConstructor)(IEnvironment *, const QString &);

    /// Список плагинов: имя -> конструктор, параметры.
    typedef QMap<QString, QPair<TConstructor, TParameterList>> TPluginList;

    /// Возвращает глобальный список плагинов.
    static TPluginList &getPluginList() {
        static TPluginList pluginList;
        return pluginList;
    }

    /// Конструктор, добавляет плагин в глобальный список, доступный фабрике.
    template <typename ParameterEnumerator>
    PluginInitializer(QString aPath,
                      TConstructor aConstructor,
                      ParameterEnumerator aParameterEnumerator) {
        getPluginList()[aPath] = qMakePair(aConstructor, aParameterEnumerator());
    }

    template <typename ParameterEnumerator>
    PluginInitializer *
    addPlugin(QString aPath, TConstructor aConstructor, ParameterEnumerator aParameterEnumerator) {
        getPluginList()[aPath] = qMakePair(aConstructor, aParameterEnumerator());

        return 0;
    }

    /// Регистрация плагина в статическом списке.
    template <typename ParameterEnumerator>
    static void registerPlugin(QString aPath,
                               TConstructor aConstructor,
                               ParameterEnumerator aParameterEnumerator) {
        getPluginList()[aPath] = qMakePair(aConstructor, aParameterEnumerator());
    }

    /// Пустой список параметров для плагинов без параметров.
    static TParameterList emptyParameterList() { return TParameterList(); }
};

//------------------------------------------------------------------------------
} // namespace Plugin
} // namespace SDK

//------------------------------------------------------------------------------

/// Вспомогательные методы для формирования пути плагина.
inline QString
makePath(const QString &aApplication, const QString &aComponent, const QString &aName) {
    return QString("%1.%2.%3").arg(aApplication).arg(aComponent).arg(aName);
}

inline QString makePath(const QString &aApplication,
                        const QString &aComponent,
                        const QString &aName,
                        const QString &aExtension) {
    return QString("%1.%2.%3.%4").arg(aApplication).arg(aComponent).arg(aName).arg(aExtension);
}

//------------------------------------------------------------------------------

/// Макросы для регистрации плагинов.

#define REGISTER_PLUGIN(path, constructor, parameters, unique_id)                                  \
    static void register_plugin_##unique_id() {                                                    \
        static bool registered = false;                                                            \
        if (!registered) {                                                                         \
            SDK::Plugin::PluginInitializer::registerPlugin(path, constructor, parameters);         \
            registered = true;                                                                     \
        }                                                                                          \
    }                                                                                              \
    static const bool dummy_##unique_id = (register_plugin_##unique_id(), true);

#define REGISTER_PLUGIN_WITH_PARAMETERS(path, constructor, parameters, unique_id)                  \
    static void register_plugin_##unique_id() {                                                    \
        static bool registered = false;                                                            \
        if (!registered) {                                                                         \
            SDK::Plugin::PluginInitializer::registerPlugin(path, constructor, parameters);         \
            registered = true;                                                                     \
        }                                                                                          \
    }                                                                                              \
    static const bool dummy_##unique_id = (register_plugin_##unique_id(), true);
