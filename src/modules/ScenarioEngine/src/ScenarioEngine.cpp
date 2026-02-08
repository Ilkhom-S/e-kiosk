/* @file Движок сценариев. */

#include <QtCore/QDirIterator>
#include <QtCore/QSettings>

#include <algorithm>

#if QT_VERSION_MAJOR >= 6
#include "JSScenario6.h"
#else
#include "JSScenario5.h"
#endif
#include "ScenarioEngine.h"

namespace GUI {

//---------------------------------------------------------------------------
namespace CScenarioEngine {
/// Название лога.
const char LogName[] = "Interface";

/// Маска поиска файлов сценариев.
const char ScenarioFileMask[] = "*.ini";

/// Название раздела с названием сценария в файле.
const char ScenarioGroup[] = "scenario";

/// Ключи конфигурационного файла.
const char NameKey[] = "name";         /// Название сценария/состояния.
const char ScriptKey[] = "script";     /// Пусть к скрипту.
const char BaseScenarioKey[] = "base"; /// Базовый сценарий.
} // namespace CScenarioEngine

//---------------------------------------------------------------------------
ScenarioEngine::ScenarioEngine() : ILogable(CScenarioEngine::LogName), m_LogPadding(0) {}

//---------------------------------------------------------------------------
ScenarioEngine::~ScenarioEngine() = default;

//---------------------------------------------------------------------------
void ScenarioEngine::addDirectory(const QString &aDirectory) {
    // Ищем в данном каталоге сценарии рекурсивно
    QDirIterator dirEntry(aDirectory,
                          QStringList(CScenarioEngine::ScenarioFileMask),
                          QDir::Files,
                          QDirIterator::Subdirectories);

    QMap<QString, QPair<QString, QString>> scenarios;

    while (dirEntry.hasNext()) {
        dirEntry.next();

        QSettings file(dirEntry.fileInfo().absoluteFilePath(), QSettings::IniFormat);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Qt6: QSettings uses UTF-8 by default, no need to set codec
#else
        file.setIniCodec("UTF-8");
#endif

        // не путаем с другими файлами
        if (file.childGroups().contains(CScenarioEngine::ScenarioGroup)) {
            file.beginGroup(CScenarioEngine::ScenarioGroup);
            scenarios[file.value(CScenarioEngine::NameKey).toString()] =
                qMakePair(dirEntry.fileInfo().absolutePath() + "/" +
                              file.value(CScenarioEngine::ScriptKey).toString(),
                          file.value(CScenarioEngine::BaseScenarioKey).toString());
        }
    }

    foreach (QString key, scenarios.keys()) {
        QPair<QString, QString> params;
        QString basePath;

        if (!params.second.isEmpty()) {
            if (scenarios.contains(params.second)) {
                basePath = scenarios[params.second].first;
            } else {
                toLog(LogLevel::Error, QString("Failed to find base scenario for %1.").arg(key));
                continue;
            }
        }

        m_Scenarios.insert(key, SScenarioDescriptor(key, scenarios[key].first, basePath));
    }
}

//---------------------------------------------------------------------------
void ScenarioEngine::addScenario(Scenario *aScenario) {
    connect(aScenario,
            SIGNAL(finished(const QVariantMap &)),
            SLOT(finished(const QVariantMap &)),
            Qt::UniqueConnection);

    // Устанавливаем лог, если он не был установлен отдельно.
    aScenario->setLog(getLog());

    m_Scenarios.insert(
        aScenario->getName(),
        SScenarioDescriptor(aScenario->getName(), aScenario->getName(), QString(), aScenario));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_ScenarioStorage.insert(aScenario->getName(), QSharedPointer<Scenario>(aScenario));
#else
    m_ScenarioStorage.insertMulti(aScenario->getName(), QSharedPointer<Scenario>(aScenario));
#endif

    toLog(LogLevel::Normal, QString("Added scenario: %1.").arg(aScenario->getName()));
}

//---------------------------------------------------------------------------
bool ScenarioEngine::initialize() const {
    return !m_ScenarioStorage.isEmpty();
}

//---------------------------------------------------------------------------
void ScenarioEngine::finalize() {
    m_ScenarioStorage.clear();
    m_ScenarioStack.clear();
    m_Scenarios.clear();
    m_ScriptObjects.clear();
}

//---------------------------------------------------------------------------
void ScenarioEngine::resetTimeout() {
    if (!m_ScenarioStack.isEmpty()) {
        m_ScenarioStack.top()->resetTimeout();
    }
}

//---------------------------------------------------------------------------
void ScenarioEngine::signalTriggered(const QString &aSignal, const QVariantMap &aArguments) {
    if (m_ScenarioStack.isEmpty()) {
        toLog(LogLevel::Warning, QString("Cannot handle '%1' signal, no scenario is running."));
    } else {
        m_ScenarioStack.top()->signalTriggered(aSignal, aArguments);
    }
}

//---------------------------------------------------------------------------
bool ScenarioEngine::startScenario(const QString &aScenario, const QVariantMap &aParameters) {
    TScenarioDescriptorMap::Iterator sc = m_Scenarios.find(aScenario);

    if (sc != m_Scenarios.end()) {
        if (!m_ScenarioStack.isEmpty()) {
            auto *prevScenario = m_ScenarioStack.top();

            if (prevScenario->getName() == aScenario) {
                return true;
            }

            // Приостанавливаем текущий сценарий.
            toLog(LogLevel::Normal, QString("SUSPEND %1 scenario.").arg(prevScenario->getName()));
            prevScenario->pause();
        }

        Scenario *scenario = nullptr;

        // В стеке уже есть такой запущенный сценарий или
        // Сценарий еще не создан
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if ((sc->instance == nullptr) ||
            std::find(m_ScenarioStack.begin(), m_ScenarioStack.end(), sc->instance) !=
                m_ScenarioStack.end()) {
#else
        if (!sc->instance || qFind(m_ScenarioStack.begin(), m_ScenarioStack.end(), sc->instance) !=
                                 m_ScenarioStack.end()) {
#endif
            // Запускаем новый экземпляр сценария.
            scenario = new JSScenario(sc->name, sc->path, sc->basePath, getLog());

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            m_ScenarioStorage.insert(aScenario, QSharedPointer<Scenario>(scenario));
#else
            m_ScenarioStorage.insertMulti(aScenario, QSharedPointer<Scenario>(scenario));
#endif

            if (scenario->initialize(m_ScriptObjects)) {
                connect(scenario,
                        SIGNAL(finished(const QVariantMap &)),
                        SLOT(finished(const QVariantMap &)),
                        Qt::UniqueConnection);
                scenario->setLog(getLog());

                sc->instance = scenario;
                m_Scenarios.insert(aScenario, *sc);

                foreach (QSharedPointer<Scenario> s, m_ScenarioStorage) {
                    if (s->getStateHook().isEmpty()) {
                        continue;
                    }

                    QList<Scenario::SExternalStateHook> hooks;

                    foreach (Scenario::SExternalStateHook hook, s->getStateHook()) {
                        if (hook.targetScenario == scenario->getName()) {
                            hooks << hook;
                        }
                    }

                    scenario->setStateHook(hooks);
                }
            } else {
                toLog(LogLevel::Error, QString("Skipping invalid scenario %1.").arg(sc->name));
                return false;
            }
        } else {
            scenario = sc->instance;
        }

        m_ScenarioStack.push(scenario);
        emit scenarioChanged(aScenario);

        toLog(LogLevel::Normal, QString("START %1 scenario.").arg(aScenario));
        getLog()->adjustPadding(1);

        scenario->start(aParameters);
    } else {
        toLog(LogLevel::Error, QString("Failed to start '%1' scenario: not found").arg(aScenario));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void ScenarioEngine::finished(const QVariantMap &aResult) {
    if (m_ScenarioStack.isEmpty()) {
        toLog(LogLevel::Error, "Some scenario has finished but the stack is empty.");
        return;
    }

    getLog()->adjustPadding(-1);

    auto *sourceScenario = dynamic_cast<GUI::Scenario *>(sender());
    if ((sourceScenario != nullptr) &&
        sourceScenario->getName() != m_ScenarioStack.top()->getName()) {
        toLog(LogLevel::Warning,
              QString("Received a signal from the scenario '%1', but '%2' is active.")
                  .arg(sourceScenario->getName())
                  .arg(m_ScenarioStack.top()->getName()));
    } else {
        toLog(LogLevel::Normal, QString("STOP %1 scenario.").arg(m_ScenarioStack.top()->getName()));
        m_ScenarioStack.top()->stop();

        m_ScenarioStack.pop();

        if (!m_ScenarioStack.isEmpty()) {
            toLog(LogLevel::Normal,
                  QString("RESUME %1 scenario.").arg(m_ScenarioStack.top()->getName()));
            m_ScenarioStack.top()->resume(aResult);
            emit scenarioChanged(m_ScenarioStack.top()->getName());
        }
    }
}

//---------------------------------------------------------------------------
bool ScenarioEngine::canStop() const {
    bool result = true;

    foreach (auto scenario, m_ScenarioStack) {
        if (!scenario->canStop()) {
            result = false;
        }
    }

    return result;
}

//---------------------------------------------------------------------------

} // namespace GUI
