/* @file Движок сценариев. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QStack>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

#include <Common/ILogable.h>

#include <ScenarioEngine/Scenario.h>

namespace GUI {

//---------------------------------------------------------------------------
/// Движок сценариев графического интерфейса.
class ScenarioEngine : public QObject, protected ILogable {
    Q_OBJECT

public:
    ScenarioEngine();
    ~ScenarioEngine();

    /// Инициализирует и запускает движок.
    bool initialize() const;

    /// Останавливает работу движка, освобождает ресурсы.
    void finalize();

    /// Добавление директории со сценариями.
    void addDirectory(const QString &aDirectory);

    /// Добавляет ранее созданный сценарий.
    void addScenario(Scenario *aScenario);

    struct SScenarioDescriptor {
        QString name;
        QString path;
        QString basePath;
        Scenario *instance;

        SScenarioDescriptor() : instance(nullptr) {}

        SScenarioDescriptor(const QString &aName,
                            const QString &aPath,
                            const QString &aBasePath,
                            Scenario *aInstance = nullptr)
            : name(aName), path(aPath), basePath(aBasePath), instance(aInstance) {}
    };

    /// Экспортирует объект во все вновь открываемые сценарии. Движок не берёт
    /// на себя обязанности по очистке памяти после использования объекта.
    void injectScriptObject(const QString &aName, QObject *aObject) {
        m_ScriptObjects << SScriptObject(aName, aObject);
    }

    /// Экспортирует типовой объект во все вновь открываемые сценарии.
    template <typename T> void injectScriptObject(const QString &aName) {
        m_ScriptObjects << SScriptObject(aName, &T::staticMetaObject);
    }

    /// Возвращает false, если какой либо из сценариев в стеке не может быть остановлен в текущий
    /// момент.
    virtual bool canStop() const;

public slots:
    /// Запускает сценарий.
    bool startScenario(const QString &aScenario, const QVariantMap &aParameters = QVariantMap());

    /// Останавливает текущий сценарий.
    void stopScenario() { /*TODO*/ }

    /// Перезапускает таймер.
    void resetTimeout();

    /// Обработка сигнала из активного состояния с дополнительными аргументами.
    void signalTriggered(const QString &aSignal, const QVariantMap &aArguments = QVariantMap());

signals:
    /// Срабатывает при переключении сценариев.
    void scenarioChanged(const QString &aCurrentScenario);

private slots:
    void finished(const QVariantMap &aResult);

private:
    typedef QMap<QString, QSharedPointer<Scenario>> TScenarioStorageMap;
    typedef QMap<QString, SScenarioDescriptor> TScenarioDescriptorMap;

    /// Загруженные сценарии-объекты.
    TScenarioStorageMap m_ScenarioStorage;

    /// Описатели сценариев
    TScenarioDescriptorMap m_Scenarios;

    /// Стек сценариев.
    QStack<Scenario *> m_ScenarioStack;

    QList<SScriptObject> m_ScriptObjects;
    int m_LogPadding;
};

} // namespace GUI

//---------------------------------------------------------------------------
