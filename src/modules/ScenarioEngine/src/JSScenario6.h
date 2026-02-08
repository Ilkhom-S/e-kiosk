/* @file Сценарий на основе javascript (Qt6 версия с QML QJSEngine). */

#pragma once

#include <QtCore/QSharedPointer>
#include <QtCore/QSignalMapper>
#include <QtCore/QTimer>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QtStateMachine/QStateMachine>

#include <ScenarioEngine/Scenario.h>

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
    void setDefaultTimeout(int aSeconds, const QJSValue &aHandler);

    /// Обработчик таймаута
    virtual void onTimeout();

private slots:
    void onEnterState(const QString &aState);
    void onExitState(const QString &aState);
    void onFinish();
    void onException(const QJSValue &aException);

private:
    /// Вызов функции внутри скрипта.
    QJSValue functionCall(const QString &aFunction,
                          const QVariantMap &aArguments,
                          const QString &aNameForLog);

    /// Подключение другого файла как модуля.
    static QJSValue includeScript(void *aContext, QJSEngine *aEngine, void *aScenario);

    /// Загрузка скрипта.
    bool
    loadScript(QJSEngine *aScriptEngine, const QString &aScenarioName, const QString &aScriptPath);

    /// Установка таймаута для состояния.
    void setStateTimeout(int aSeconds);

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
    QSharedPointer<QJSEngine> m_ScriptEngine;     /// Скриптовый движок для обработчиков.
    QSharedPointer<QStateMachine> m_StateMachine; /// Конечный автомат.
    TStateList m_States;                          /// Набор состояний.
    QSignalMapper m_EnterSignalMapper;
    QSignalMapper m_ExitSignalMapper;
    QString m_CurrentState;                    /// Имя текущего состояния.
    QString m_InitialState;                    /// Начальное состояние.
    QVariantMap m_SignalArguments;             /// Параметры сигналов.
    QVariantMap m_Context;                     /// Контекст.
    QString m_ScriptPath;                      /// Путь к файлу скрипты.
    QJSValue m_TimeoutHandler;                 /// Обработчик таймаута.
    QMultiMap<QString, QString> m_Transitions; /// Список возможных переходов для состояния.
    bool m_IsPaused;                           /// Флаг для пропуска переходов в сценариях "на паузе"
    int m_DefaultTimeout;                      /// Таймаут по умолчанию.
    QTimer m_TimeoutTimer;                     /// Таймер для таймаутов состояний.

    QList<Scenario::SExternalStateHook> m_Hooks;
};

} // namespace GUI

//---------------------------------------------------------------------------
