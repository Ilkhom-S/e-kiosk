/* @file Реализация фабрики плагинов. */

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QUuid>

#include <Common/ExceptionFilter.h>
#include <Common/PluginConstants.h>
#include <Common/Version.h>

#include <SDK/Plugins/PluginFactory.h>
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/PluginLoader.h>

#include <iostream>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Список имен параметров общих для любого плагина.
namespace Parameters {
const char Debug[] = "debug";         /// Режим отладки
const char Singleton[] = "singleton"; /// Плагин - единоличник
} // namespace Parameters

//------------------------------------------------------------------------------
namespace CPluginFactory {
/// Папка для сохранения конфигураций плагинов.
const char ConfigurationDirectory[] = "plugins";
} // namespace CPluginFactory

//------------------------------------------------------------------------------
PluginFactory::PluginFactory() : m_Kernel(nullptr), m_Initialized(false) {}

//------------------------------------------------------------------------------
PluginFactory::~PluginFactory() {
    if (m_Initialized) {
        // Вызов виртуального метода в деструкторе намеренный.
        // Виртуальная диспетчеризация здесь не используется, вызывается только базовая версия.
        shutdown();
    }
}

//------------------------------------------------------------------------------
bool PluginFactory::initialize(IKernel *aKernel, const QString &aDirectory) {
    m_Initialized = false;

    if (!aKernel) {
        Q_ASSERT(aKernel);
        return m_Initialized;
    }

    m_Kernel = aKernel;
    m_Directory = aDirectory;

    m_Kernel->getLog()->write(LogLevel::Normal,
                              QString("Initializing plugin library \"%1\". Core: %2.")
                                  .arg(getName())
                                  .arg(Humo::getVersion()));

    if (m_Kernel->getVersion() != Humo::getVersion()) {
        m_Kernel->getLog()->write(LogLevel::Warning,
                                  QString("Plugin library \"%1\" compiled with a different "
                                          "core versions. Compiled:[%2] Current core:[%3]")
                                      .arg(getName())
                                      .arg(Humo::getVersion())
                                      .arg(m_Kernel->getVersion()));
    }

    // Отключение плагинов с пустым путём или конструктором
    for (PluginInitializer::TPluginList::iterator i = PluginInitializer::getPluginList().begin();
         i != PluginInitializer::getPluginList().end();
         ++i) {
        if (i.key().isEmpty() || !i.value().first) {
            m_Kernel->getLog()->write(
                LogLevel::Warning,
                QString("Disabling broken plugin %1: no path or constructor.").arg(i.key()));
        }
    }

    // Загрузка конфигураций (в одном файле хранятся ВСЕ конфигурации ВСЕХ плагинов данной
    // библиотеки)
    QFileInfo file(QDir::toNativeSeparators(QDir::cleanPath(
        m_Kernel->getDataDirectory() + QDir::separator() + CPluginFactory::ConfigurationDirectory +
        QDir::separator() + m_ModuleName + ".ini")));

    if (file.exists()) {
        m_Kernel->getLog()->write(
            LogLevel::Normal,
            QString("Configuration file %1 found, loading.").arg(file.absoluteFilePath()));

        QSettings config(file.absoluteFilePath(), QSettings::IniFormat);
        // В Qt6 метод setIniCodec() удален, UTF-8 используется по умолчанию
        // В Qt5 необходимо явно установить кодировку UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        config.setIniCodec("utf-8");
#endif

        foreach (QString group, config.childGroups()) {
            QVariantMap parameters;
            config.beginGroup(group);

            foreach (QString key, config.allKeys()) {
                parameters[key] = config.value(key);
            }

            config.endGroup();
            m_PersistentConfigurations[group] = parameters;
        }
    }

    m_Initialized = true;

    return m_Initialized;
}

//------------------------------------------------------------------------------
void PluginFactory::shutdown() {
    if (m_Kernel) {
        // Don't log during Qt shutdown to avoid crashes in test framework
        if (QCoreApplication::instance() != nullptr) {
            m_Kernel->getLog()->write(
                LogLevel::Normal, QString("Shutting down plugin library \"%1\".").arg(getName()));
        }

        foreach (IPlugin *plugin, m_CreatedPlugins.keys()) {
            delete plugin;
        }
    }

    m_CreatedPlugins.clear();
    m_Initialized = false;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
QString PluginFactory::getName() const {
    return m_Name;
}

//------------------------------------------------------------------------------
QString PluginFactory::getDescription() const {
    return m_Description;
}

//------------------------------------------------------------------------------
QString PluginFactory::getAuthor() const {
    return m_Author;
}

//------------------------------------------------------------------------------
QString PluginFactory::getVersion() const {
    return m_Version;
}

//------------------------------------------------------------------------------

QString PluginFactory::getModuleName() const {
    return m_ModuleName;
}

//------------------------------------------------------------------------------

QStringList PluginFactory::getPluginList() const {
    return PluginInitializer::getPluginList().keys();
}

// Force vtable instantiation
static PluginFactory *dummy = nullptr;

//------------------------------------------------------------------------------
IPlugin *PluginFactory::createPlugin(const QString &aInstancePath,
                                     const QString &aConfigInstancePath) {
    m_Kernel->getLog()->write(LogLevel::Normal, QString("Creating plugin %1.").arg(aInstancePath));

    IPlugin *plugin = nullptr;

    // Разделим на путь и конфигурацию
    QString path = aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);
    QString instance = aInstancePath.section(CPlugin::InstancePathSeparator, 1, 1);

    // Найдём нужный класс
    PluginInitializer::TPluginList::iterator pluginInfo =
        PluginInitializer::getPluginList().find(path);

    // Сгенерируем имя конфигурации, если отсутствует
    if (instance.isEmpty()) {
        auto singleton = findParameter(SDK::Plugin::Parameters::Singleton, pluginInfo->second);

        if (singleton.isValid() && singleton.defaultValue.toBool()) {
            instance = SDK::Plugin::Parameters::Singleton;
        } else {
            instance = QUuid::createUuid().toString();
            instance.remove('{');
            instance.remove('}');
        }
    }

    if (pluginInfo != PluginInitializer::getPluginList().end()) {
        try {
            QString realInstancePath(path + CPlugin::InstancePathSeparator + instance);

            auto pluginConstructor = pluginInfo.value().first;
            plugin = pluginConstructor(this, realInstancePath);

            if (plugin && plugin->isReady()) {
                // Инициализируем и конфигурируем плагин
                m_CreatedPlugins[plugin] = realInstancePath;
                QString configPath =
                    aConfigInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);
                QString configInstance =
                    aConfigInstancePath.section(CPlugin::InstancePathSeparator, 1, 1);

                QVariantMap configuration = getPluginInstanceConfiguration(
                    configPath, configInstance.isEmpty() ? instance : configInstance);

                // Синхронизируем настройки конфига и плагина, если плагин новее, чем конфиг
                TParameterList &parameterList = m_TranslatedParameters[path];

                SPluginParameter modifiedKeys = findParameter(CPlugin::ModifiedKeys, parameterList);

                if (modifiedKeys.isValid()) {
                    for (auto it = modifiedKeys.possibleValues.begin();
                         it != modifiedKeys.possibleValues.end();
                         ++it) {
                        const QString &key = it.key();
                        QString newKey = it.value().toString();

                        if (configuration.contains(key) &&
                            (!configuration.contains(newKey) || configuration[newKey].isNull() ||
                             !configuration[newKey].isValid())) {
                            QVariant value = configuration[key];
                            configuration.remove(key);
                            configuration.insert(newKey, value);
                        }
                    }
                }

                IPluginLoader *pluginLoader = getPluginLoader();

                for (auto it = parameterList.begin(); it != parameterList.end(); ++it) {
                    bool isSet = !it->possibleValues.isEmpty();

                    for (auto jt = it->possibleValues.begin(); jt != it->possibleValues.end();
                         ++jt) {
                        isSet = isSet && (jt.key() == jt.value());
                    }

                    bool notContains = !configuration.contains(it->name);
                    QVariant configValue = configuration[it->name];

                    if (notContains || (isSet && !it->defaultValue.toString().isEmpty() &&
                                        !it->possibleValues.values().contains(configValue))) {
                        QString newValue = it->defaultValue.toString();

                        if ((pluginLoader != nullptr) && !notContains &&
                            (it->name != CPlugin::ModifiedValues)) {
                            for (auto &jt : parameterList) {
                                if (jt.isValid() && (jt.name == CPlugin::ModifiedValues) &&
                                    (jt.title.isEmpty() || (jt.title == it->name))) {
                                    QString modifiedValue =
                                        jt.possibleValues.value(configValue.toString()).toString();

                                    if (!modifiedValue.isEmpty()) {
                                        newValue = modifiedValue;
                                    }
                                }
                            }
                        }

                        configuration.insert(it->name, newValue);
                    }
                }

                /*
                //в случае изменения пути плагина с одновременным изменением настроек появятся 2
                разные конфигурации одного устройства if (oldConfiguration != configuration)
                {
                        QString realConfigPath(configPath + CPlugin::InstancePathSeparator +
                configInstance); saveConfiguration(realInstancePath, configuration);
                }
                */

                // Добавляем версию PP и устанавливаем настройки плагина
                configuration.insert(CPluginParameters::PPVersion, m_Kernel->getVersion());
                plugin->setConfiguration(configuration);
            } else {
                m_Kernel->getLog()->write(LogLevel::Error,
                                          QString("Failed to create plugin %1.").arg(path));

                if (plugin) {
                    delete plugin;
                    plugin = nullptr;
                }
            }
        } catch (...) {
            EXCEPTION_FILTER_NO_THROW(m_Kernel->getLog());

            if (plugin) {
                delete plugin;
                plugin = nullptr;
            }
        }
    } else {
        m_Kernel->getLog()->write(LogLevel::Error,
                                  QString("No plugin found for %1.").arg(aInstancePath));
    }

    return plugin;
}

//------------------------------------------------------------------------------
bool PluginFactory::destroyPlugin(IPlugin *aPlugin) {
    Q_ASSERT(aPlugin);

    if (aPlugin) {
        m_Kernel->getLog()->write(
            LogLevel::Normal, QString("Destroying plugin \"%1\".").arg(aPlugin->getPluginName()));

        if (m_CreatedPlugins.contains(aPlugin)) {
            m_CreatedPlugins.remove(aPlugin);
            delete aPlugin;

            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
std::weak_ptr<IPlugin> PluginFactory::createPluginPtr(const QString &aInstancePath,
                                                      const QString &aConfigPath) {
    IPlugin *plugin = createPlugin(aInstancePath, aConfigPath);

    if (!plugin) {
        return {};
    }

    auto pluginPtr = std::shared_ptr<IPlugin>(plugin, SDK::Plugin::PluginDeleter());

    // createPlugin помещает указатель на плагин в отдельную мапу. Поместим плагин в отдельный
    // контейнер.
    m_CreatedPluginsPtr.insert(pluginPtr, m_CreatedPlugins[plugin]);
    m_CreatedPlugins.remove(plugin);

    return pluginPtr;
}

//------------------------------------------------------------------------------
bool PluginFactory::destroyPlugin(const std::weak_ptr<IPlugin> &aPlugin) {
    m_Kernel->getLog()->write(
        LogLevel::Normal,
        QString("Destroying plugin \"%1\".").arg(aPlugin.lock()->getPluginName()));

    if (m_CreatedPluginsPtr.contains(aPlugin.lock())) {
        m_CreatedPluginsPtr.remove(aPlugin.lock());
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
void PluginFactory::translateParameters() {
    auto pluginList = PluginInitializer::getPluginList();

    foreach (auto pluginPath, pluginList.keys()) {
        auto parameters = pluginList.value(pluginPath).second;
        auto pluginName = pluginPath.section(".", -1, -1).toLatin1();

        TParameterList result;

        auto translateItem = [&](QString &aItem) {
            QStringList elements = aItem.split(" ");

            for (auto &element : elements) {
                // Контекст для перевода может содержаться в самой строке с переводом.
                auto context = element.section("#", 0, 0).toLatin1();
                element =
                    qApp->translate(context.isEmpty() ? pluginName : context, element.toLatin1());
            }

            aItem = elements.join(" ");
        };

        // Производим локализацию названий и описаний параметров.
        foreach (auto parameter, parameters) {
            translateItem(parameter.title);
            translateItem(parameter.description);

            result.append(parameter);
        }

        m_TranslatedParameters.insert(pluginPath, result);
    }
}

//------------------------------------------------------------------------------
IExternalInterface *PluginFactory::getInterface(const QString &aInterface) {
    if (!m_Kernel) {
        return nullptr;
    }
    return m_Kernel->getInterface(aInterface);
}

//------------------------------------------------------------------------------
QString PluginFactory::getKernelVersion() const {
    return m_Kernel->getVersion();
}

//------------------------------------------------------------------------------
QString PluginFactory::getKernelDirectory() const {
    return m_Kernel->getDirectory();
}

//------------------------------------------------------------------------------
QString PluginFactory::getKernelDataDirectory() const {
    return m_Kernel->getDataDirectory();
}

//------------------------------------------------------------------------------
QString PluginFactory::getKernelLogsDirectory() const {
    return m_Kernel->getLogsDirectory();
}

//------------------------------------------------------------------------------
const QString &PluginFactory::getPluginDirectory() const {
    return m_Directory;
}

//------------------------------------------------------------------------------
ILog *PluginFactory::getLog(const QString &aName) {
    return m_Kernel->getLog(aName);
}

//------------------------------------------------------------------------------
TParameterList PluginFactory::getPluginParametersDescription(const QString &aPath) const {
    PluginInitializer::TPluginList::Iterator plugin =
        PluginInitializer::getPluginList().find(aPath);

    if (plugin != PluginInitializer::getPluginList().end()) {
        return m_TranslatedParameters.contains(aPath) ? m_TranslatedParameters.value(aPath)
                                                      : plugin->second;
    }

    return {};
}

//------------------------------------------------------------------------------
QStringList PluginFactory::getRuntimeConfigurations(const QString &aPathFilter) const {
    // Немного оптимизации
    if (aPathFilter.isEmpty()) {
        return m_CreatedPlugins.values();
    }
    QRegularExpression regex(QString("^%1").arg(QRegularExpression::escape(aPathFilter)));
    return QStringList(m_CreatedPlugins.values()).filter(regex);
}

//------------------------------------------------------------------------------
QStringList PluginFactory::getPersistentConfigurations(const QString &aPathFilter) const {
    // Немного оптимизации
    if (aPathFilter.isEmpty()) {
        return m_PersistentConfigurations.keys();
    }
    QRegularExpression regex(QString("^%1").arg(QRegularExpression::escape(aPathFilter)));
    return QStringList(m_PersistentConfigurations.keys()).filter(regex);
}

//------------------------------------------------------------------------------
bool isContainContent(const QVariantMap &aStorage, const QVariantMap &aContent) {
    foreach (auto key, aContent.keys()) {
        if (!aStorage.contains(key) || aStorage.value(key) != aContent.value(key)) {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------
bool PluginFactory::saveConfiguration(const QString &aInstancePath,
                                      const QVariantMap &aParameters) {
    bool result = false;

    // Сохраняем во внешнем хранилище, если можно
    if (m_Kernel->canSavePluginConfiguration(aInstancePath)) {
        m_Kernel->getLog()->write(
            LogLevel::Error,
            QString("Saving configuration %1 to external storage.").arg(aInstancePath));

        // Фильтруем параметры - сохраняем только те, которые есть в описании плагина.
        TParameterList parameterDefinitions = getPluginParametersDescription(
            aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0));
        QVariantMap parameters;

        foreach (QString key, aParameters.keys()) {
            if (findParameter(key, parameterDefinitions).isValid() && aParameters[key].isValid()) {
                parameters.insert(key, aParameters[key]);
            }
        }

        result = m_Kernel->savePluginConfiguration(aInstancePath, parameters);
    } else {
        // Формируем список параметров для сохранения в конфиг
        QVariantMap saveParameters;
        TParameterList parameters = getPluginParametersDescription(
            aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0));

        foreach (QString key, aParameters.keys()) {
            if (findParameter(key, parameters).isValid() && aParameters[key].isValid()) {
                saveParameters[key] = aParameters[key];
            }
        }

        // Проверяем, нужно ли сохранять
        if (isContainContent(m_PersistentConfigurations[aInstancePath], saveParameters)) {
            m_Kernel->getLog()->write(
                LogLevel::Normal,
                QString("Skip saving configuration %1. Nothing changes.").arg(aInstancePath));
        } else {
            // Иначе в конфигурационный файл
            QString fileName = m_Kernel->getDataDirectory() + QDir::separator() +
                               CPluginFactory::ConfigurationDirectory + QDir::separator() +
                               m_ModuleName + ".ini";
            QSettings config(QDir::toNativeSeparators(QDir::cleanPath(fileName)),
                             QSettings::IniFormat);
            // В Qt6 метод setIniCodec() удален, UTF-8 используется по умолчанию
            // В Qt5 необходимо явно установить кодировку UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            config.setIniCodec("utf-8");
#endif

            result = config.isWritable();

            if (result) {
                m_Kernel->getLog()->write(
                    LogLevel::Normal,
                    QString("Saving configuration %1 to file.").arg(config.fileName()));

                config.beginGroup(aInstancePath);

                foreach (QString key, saveParameters.keys()) {
                    config.setValue(key, saveParameters[key]);
                }

                config.endGroup();
                config.sync();

                m_PersistentConfigurations[aInstancePath] = aParameters;
            } else {
                m_Kernel->getLog()->write(
                    LogLevel::Error,
                    QString("Failed to save configuration %1: %2 is not writable.")
                        .arg(aInstancePath)
                        .arg(config.fileName()));
            }
        }
    }

    return result;
}

//------------------------------------------------------------------------------
QVariantMap PluginFactory::getPluginInstanceConfiguration(const QString &aPath,
                                                          const QString &aInstance) {
    QString instancePath = aPath + CPlugin::InstancePathSeparator + aInstance;

    // Если конфигурация есть у приложения, берём её
    if (m_Kernel->canConfigurePlugin(instancePath)) {
        return m_Kernel->getPluginConfiguration(instancePath);
    } // Иначе из конфигурационного файла
    if (m_PersistentConfigurations.contains(instancePath)) {
        return m_PersistentConfigurations[instancePath];
    }
    if (m_PersistentConfigurations.contains(aPath)) {
        // Иначе из конфигурационного файла конфигурацию по умолчанию
        return m_PersistentConfigurations[aPath];
    }

    // Иначе пустую конфигурацию
    return {};
}

//------------------------------------------------------------------------------
IPluginLoader *PluginFactory::getPluginLoader() const {
    return m_Kernel->getPluginLoader();
}

//------------------------------------------------------------------------------
} // namespace Plugin
} // namespace SDK
