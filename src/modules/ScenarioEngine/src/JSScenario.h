/* @file Сценарий на основе javascript. */

#pragma once

#include <QtCore/QSignalMapper>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtStateMachine/QStateMachine>
#else
#include <QtCore/QStateMachine>
#endif
#include <QtCore/QSharedPointer>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#else
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#endif

#include "Scenario.h"

namespace GUI {

//---------------------------------------------------------------------------
/// Базовый класс сценария.
class JSScenario : public Scenario {
    Q_OBJECT

public:
    JSScenario(const QString &aName,
               const QString &aPath,
               const QString &aBasePath,
               ILog *aLog = 0);
    virtual ~JSScenario();

    /// Запуск сценария.
    virtual void start(const QVariantMap &aContext);

    /// Остановка сценария.
    virtual void stop() {};

    /// Приостановить сценарий с возможностью последующего возобновления.
    virtual void pause();

    /// Продолжение после паузы.
    virtual void resume(const QVariantMap &aContext);

    /// Инициализация сценария. Вызывается из скрипта.
    virtual bool initialize(const QList<SScriptObject> &aScriptObjects);

    virtual void setStateHook(const QList<SExternalStateHook> &aHook) { m_Hooks << aHook; }

    /// Обработка сигнала из активного состояния с дополнительными аргументами.
    virtual void signalTriggered(const QString &aSignal,
                                 const QVariantMap &aArguments = QVariantMap());

    /// Возвращает false, если сценарий не может быть остановлен в текущий момент.
    virtual bool canStop();

public slots:
    /// Текущее состояние.
    virtual QString getState() const;

    /// Добавить состояние.
    void addState(const QString &aStateName, const QVariantMap &aParameters);

    /// Добавить переход к сценарию.
    void addTransition(const QString &aSource, const QString &aTarget, const QString &aSignal);

/// Установка таймаута по-умолчанию.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void setDefaultTimeout(int aSeconds, const QJSValue &aHandler);
#else
    void setDefaultTimeout(int aSeconds, const QScriptValue &aHandler);
#endif

    /// Обработчик таймаута
    virtual void onTimeout();

private slots:
    void onEnterState(const QString &aState);
    void onExitState(const QString &aState);
    void onFinish();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void onException(const QJSValue &aException);
#else
    void onException(const QScriptValue &aException);
#endif

private:
/// Вызов функции внутри скрипта.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QJSValue functionCall(const QString &aFunction,
                          const QVariantMap &aArguments,
                          const QString &aNameForLog);
#else
    QScriptValue functionCall(const QString &aFunction,
                              const QVariantMap &aArguments,
                              const QString &aNameForLog);
#endif

/// Подключение другого файла как модуля.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static QJSValue includeScript(void *aContext, QJSEngine *aEngine, void *aScenario);
#else
    static QScriptValue
    includeScript(QScriptContext *aContext, QScriptEngine *aEngine, void *aScenario);
#endif

/// Загрузка скрипта.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool
    loadScript(QJSEngine *aScriptEngine, const QString &aScenarioName, const QString &aScriptPath);
#else
    bool loadScript(QScriptEngine *aScriptEngine,
                    const QString &aScenarioName,
                    const QString &aScriptPath);
#endif

    /// Контекст.
    QVariantMap getContext() const;

    /// Этап сценария (состояние конечного автомата).
    struct SState {
        QString name;           /// Название состояния.
        QAbstractState *qstate; /// Состояние для QStateMachine.
        QVariantMap parameters; /// Разные параметры состояния.
    };

    typedef QMap<QString, SState> TStateList;

private:
    QString m_Path;     /// Путь к сценарию.
    QString m_BasePath; /// Путь к скрипту родительского сценария (если применяется наследование).
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QSharedPointer<QJSEngine> m_ScriptEngine; /// Скриптовый движок для обработчиков.
#else
    QSharedPointer<QScriptEngine> m_ScriptEngine; /// Скриптовый движок для обработчиков.
#endif
    QSharedPointer<QStateMachine> m_StateMachine; /// Конечный автомат.
    TStateList m_States;                          /// Набор состояний.
    QSignalMapper m_EnterSignalMapper;
    QSignalMapper m_ExitSignalMapper;
    QString m_CurrentState;        /// Имя текущего состояния.
    QString m_InitialState;        /// Начальное состояние.
    QVariantMap m_SignalArguments; /// Параметры сигналов.
    QVariantMap m_Context;         /// Контекст.
    QString m_ScriptPath;          /// Путь к файлу скрипты.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QJSValue m_TimeoutHandler; /// Обработчик таймаута.
#else
    QScriptValue m_TimeoutHandler; /// Обработчик таймаута.
#endif
    QMultiMap<QString, QString> m_Transitions; /// Список возможных переходов для состояния.
    bool m_IsPaused;                           /// Флаг для пропуска переходов в сценариях "на паузе"

    QList<Scenario::SExternalStateHook> m_Hooks;
};

} // namespace GUI

//---------------------------------------------------------------------------
