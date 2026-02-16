/* @file Реализация фабрики плагинов. */

#include <QtCore/QCoreApplication>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QRecursiveMutex>
#include <QtCore/QTranslator>

#include <iostream>

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif

// Plugin SDK
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/PluginLoader.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
PluginLoader::PluginLoader(IKernel *aKernel) : m_Kernel(aKernel) {}

//------------------------------------------------------------------------------
PluginLoader::~PluginLoader() {
    foreach (QSharedPointer<QPluginLoader> library, m_Libraries) {
        m_Kernel->getLog()->write(
            LogLevel::Debug, QString("Plugin '%1' will be unloaded.").arg(library->fileName()));

        // Не делаем этого, т.к. из главного модуля могут быть ссылки на данные/функции внутри
        // библиотеки (а они есть - баг Qt). library->unload();
    }
}

//------------------------------------------------------------------------------
QStringList PluginLoader::getPluginList(const QRegularExpression &aFilter) const {
    QMutexLocker lock(&m_AccessMutex);

    getPluginPathList(aFilter);

    return QStringList(m_Plugins.keys()).filter(aFilter);
}

//------------------------------------------------------------------------------
QStringList PluginLoader::getPluginPathList(const QRegularExpression &aFilter) const {
    QMutexLocker lock(&m_AccessMutex);

    QStringList result;

    foreach (QSharedPointer<QPluginLoader> p, m_Libraries) {
        result << p->fileName();
    }

    return QStringList(result).filter(aFilter);
}

//------------------------------------------------------------------------------
QVariantMap PluginLoader::getPluginInstanceConfiguration(const QString &aInstancePath,
                                                         const QString &aConfigPath) {
    if (!m_Plugins.contains(aInstancePath)) {
        m_Kernel->getLog()->write(LogLevel::Error,
                                  QString("No such plugin %1.").arg(aInstancePath));
        return {};
    }

    QString configPath = aConfigPath.section(CPlugin::InstancePathSeparator, 0, 0);
    QString configPostfix = aConfigPath.section(CPlugin::InstancePathSeparator, 1, 1);

    return m_Plugins[aInstancePath]->getPluginInstanceConfiguration(configPath, configPostfix);
}

//------------------------------------------------------------------------------
IPlugin *PluginLoader::createPlugin(const QString &aInstancePath,
                                    const QString &aConfigInstancePath) {
    QMutexLocker lock(&m_AccessMutex);

    SDK::Plugin::IPlugin *plugin = nullptr;

    QString path = aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);

    if (m_Plugins.contains(path)) {
        QString configInstancePath =
            aConfigInstancePath.isEmpty() ? aInstancePath : aConfigInstancePath;
        plugin = m_Plugins[path]->createPlugin(aInstancePath, configInstancePath);

        if (plugin) {
            m_CreatedPlugins[plugin] = m_Plugins[path];
        }
    } else {
        m_Kernel->getLog()->write(LogLevel::Error,
                                  QString("No such plugin %1.").arg(aInstancePath));
    }

    return plugin;
}

//------------------------------------------------------------------------------
std::weak_ptr<IPlugin> PluginLoader::createPluginPtr(const QString &aInstancePath,
                                                     const QString &aConfigInstancePath) {
    QMutexLocker lock(&m_AccessMutex);

    QString path = aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);

    if (m_Plugins.contains(path)) {
        QString configInstancePath =
            aConfigInstancePath.isEmpty() ? aInstancePath : aConfigInstancePath;
        auto p = m_Plugins[path]->createPluginPtr(aInstancePath, configInstancePath);
        if (!p.expired()) {
            TPluginPtr plugin = p.lock();
            m_CreatedPluginsPtr[plugin] = m_Plugins[path];
            return p;
        }
    }

    m_Kernel->getLog()->write(LogLevel::Error, QString("No such plugin %1.").arg(aInstancePath));

    return {};
}

//------------------------------------------------------------------------------
bool PluginLoader::destroyPlugin(IPlugin *aPlugin) {
    QMutexLocker lock(&m_AccessMutex);

    if (m_CreatedPlugins.contains(aPlugin)) {
        m_CreatedPlugins[aPlugin]->destroyPlugin(aPlugin);
        m_CreatedPlugins.remove(aPlugin);

        return true;
    }

    m_Kernel->getLog()->write(
        LogLevel::Error,
        QString("Failed to destroy plugin 0x%1, doesn't exist.").arg(qlonglong(aPlugin), 0, 16));

    return false;
}

//------------------------------------------------------------------------------
bool PluginLoader::destroyPluginPtr(const std::weak_ptr<IPlugin> &aPlugin) {
    QMutexLocker lock(&m_AccessMutex);

    auto ptr = aPlugin.lock();

    if (m_CreatedPluginsPtr.find(ptr) != m_CreatedPluginsPtr.end()) {
        auto *fabric = m_CreatedPluginsPtr[ptr];
        m_CreatedPluginsPtr.erase(ptr);
        fabric->destroyPlugin(ptr);

        return true;
    }

    m_Kernel->getLog()->write(
        LogLevel::Error,
        QString("Failed to destroy plugin 0x%1, doesn't exist.").arg(qlonglong(ptr.get()), 0, 16));

    return false;
}

//------------------------------------------------------------------------------
int PluginLoader::addDirectory(const QString &aDirectory) {
    QMutexLocker lock(&m_AccessMutex);

    m_Directories << aDirectory;

    m_Kernel->getLog()->write(LogLevel::Debug,
                              QString("Scanning plugin directory: %1").arg(aDirectory));

    QStringList fileNameFilter;
#ifdef Q_OS_WIN
    fileNameFilter << "*.dll";
#elif defined(Q_OS_MAC)
    fileNameFilter << "*.dylib";
#else
    fileNameFilter << "*.so";
#endif

    // Debug: Log what file filter is used
    m_Kernel->getLog()->write(LogLevel::Debug,
                              QString("Plugin file filter: %1").arg(fileNameFilter.join(", ")));

    QDirIterator dirEntry(aDirectory, fileNameFilter, QDir::Files, QDirIterator::Subdirectories);

    // Загрузка библиотек
    while (dirEntry.hasNext()) {
        dirEntry.next();

        m_Kernel->getLog()->write(LogLevel::Debug,
                                  QString("Found plugin file: %1").arg(dirEntry.filePath()));

#ifdef Q_OS_WIN
        // Windows: Set DLL search path for dependent libraries
        SetDllDirectoryW(dirEntry.fileInfo().absolutePath().toStdWString().data());
#else
        // Unix/Linux/macOS: Add plugin directory to library search path
        // Note: On macOS, use DYLD_LIBRARY_PATH instead of LD_LIBRARY_PATH
        QString pluginDir = dirEntry.fileInfo().absolutePath();
        QString currentLibPath = qgetenv("DYLD_LIBRARY_PATH");

        if (!currentLibPath.contains(pluginDir)) {
            QString newLibPath = pluginDir + ":" + currentLibPath;
            qputenv("DYLD_LIBRARY_PATH", newLibPath.toUtf8());
        }
#endif

        QSharedPointer<QPluginLoader> library(new QPluginLoader(dirEntry.filePath()));
        QObject *rootObject = library->instance();

        if (rootObject) {
            SDK::Plugin::IPluginFactory *factory =
                qobject_cast<SDK::Plugin::IPluginFactory *>(rootObject);

            if (factory) {
                if (factory->initialize(m_Kernel, dirEntry.fileInfo().absolutePath())) {
                    m_Kernel->getLog()->write(
                        LogLevel::Normal,
                        QString("Loading %1. Name: %3. Author: %4. Version: %5.")
                            .arg(dirEntry.filePath())
                            .arg(factory->getName())
                            .arg(factory->getAuthor())
                            .arg(factory->getVersion()));

                    m_Libraries << library;

                    // Загрузка локализации
                    QString pluginBaseName = dirEntry.fileInfo().baseName();

                    // На macOS убираем префикс "lib" из имени библиотеки
                    if (pluginBaseName.startsWith("lib")) {
                        pluginBaseName = pluginBaseName.mid(3);
                    }

                    // Пытаемся найти файлы переводов с текущим именем
                    QDir translations(dirEntry.fileInfo().absolutePath(),
                                      QString("%1_*.qm").arg(pluginBaseName));

                    // Если не найдены и имя заканчивается на "d", пробуем без него
                    // (debug builds добавляют суффикс "d", но файлы переводов без него)
                    if (translations.count() == 0 && pluginBaseName.endsWith("d")) {
                        QString baseNameWithoutDebug = pluginBaseName;
                        baseNameWithoutDebug.chop(1);
                        translations = QDir(dirEntry.fileInfo().absolutePath(),
                                            QString("%1_*.qm").arg(baseNameWithoutDebug));
                        if (translations.count() != 0) {
                            pluginBaseName = baseNameWithoutDebug;
                        }
                    }

                    if (translations.count() != 0) {
                        // Получаем текущий язык приложения
                        QString currentLanguage = m_Kernel->getLanguage();

                        // Ищем файл перевода для текущего языка
                        QString targetTranslation =
                            QString("%1_%2.qm").arg(pluginBaseName, currentLanguage);
                        QString translationPath = QDir(dirEntry.fileInfo().absolutePath())
                                                      .absoluteFilePath(targetTranslation);

                        // Если файл для текущего языка не найден, пробуем английский
                        if (!QFile::exists(translationPath) && currentLanguage != "en") {
                            targetTranslation = QString("%1_en.qm").arg(pluginBaseName);
                            translationPath = QDir(dirEntry.fileInfo().absolutePath())
                                                  .absoluteFilePath(targetTranslation);

                            if (!QFile::exists(translationPath)) {
                                // Если и английский не найден, берем первый доступный
                                translationPath =
                                    translations.entryInfoList().first().absoluteFilePath();
                            }
                        }

                        std::unique_ptr<QTranslator> translator(new QTranslator(qApp));

                        if (translator->load(translationPath)) {
                            qApp->installTranslator(translator.release());

                            m_Kernel->getLog()->write(LogLevel::Normal,
                                                      QString("Translation %1 for %2 loaded.")
                                                          .arg(translationPath)
                                                          .arg(factory->getName()));
                        }
                    }

                    factory->translateParameters();

                    // Загрузка информации о плагинах
                    foreach (QString path, factory->getPluginList()) {
                        if (m_Plugins.contains(path)) {
                            m_Kernel->getLog()->write(
                                LogLevel::Warning,
                                QString(
                                    "Plugin %1 is already registered in %2, ignoring this one...")
                                    .arg(path)
                                    .arg(m_Plugins[path]->getName()));
                        } else {
                            m_Plugins[path] = factory;
                        }
                    }
                } else {
                    m_Kernel->getLog()->write(LogLevel::Warning,
                                              QString("Failed to initialize plugin factory in %1.")
                                                  .arg(dirEntry.filePath()));
                    library->unload();
                }
            } else {
                m_Kernel->getLog()->write(
                    LogLevel::Warning,
                    QString("%1 doesn't support base plugin interface.").arg(dirEntry.filePath()));
                library->unload();
            }
        } else {
            m_Kernel->getLog()->write(
                LogLevel::Warning,
                QString("Skipping %1: %2").arg(dirEntry.filePath()).arg(library->errorString()));
        }
    }

#ifdef Q_OS_WIN
    // Windows: Reset DLL search path
    SetDllDirectory(0);
#else
    // Unix/Linux/macOS: LD_LIBRARY_PATH modification persists for the process
    // No explicit reset needed as it's an environment variable
#endif

    // Qt6: QMap::count() возвращает qsizetype (long long), явное приведение к int безопасно,
    // т.к. количество плагинов не превысит INT_MAX
    return static_cast<int>(m_Plugins.count());
}

//------------------------------------------------------------------------------
TParameterList PluginLoader::getPluginParametersDescription(const QString &aPath) const {
    QMutexLocker lock(&m_AccessMutex);

    if (m_Plugins.contains(aPath)) {
        return m_Plugins.value(aPath)->getPluginParametersDescription(aPath);
    }

    return {};
}

//------------------------------------------------------------------------------
} // namespace Plugin
} // namespace SDK
