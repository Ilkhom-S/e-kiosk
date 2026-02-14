/* @file Реализация JSScenario для Qt6 с QML QJSEngine. */

#include "JSScenario6.h"

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtGlobal>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QtStateMachine/QAbstractTransition>
#include <QtStateMachine/QFinalState>

#include <memory>
#include <utility>

namespace GUI {

//---------------------------------------------------------------------------
namespace CJSScenario {
/// Название сервиса для Core API.
const char ServiceName[] = "ScenarioEngine";

/// Функция импорта другого файла из скрипта.
const char ScriptIncludeFunction[] = "include";

/// Javascript-функции сценария.
const char ScriptInitFunction[] = "initialize";   /// Инициализация.
const char ScriptStartFunction[] = "onStart";     /// Запуск сценария.
const char ScriptStopFunction[] = "onStop";       /// Завершение.
const char ScriptTimeoutFunction[] = "onTimeout"; /// Таймаут.
const char ScriptPauseFunction[] = "onPause";     /// Прерывание выполнения сценария.
const char ScriptResumeFunction[] = "onResume";   /// Возобновление прерванного сценария.
const char ScriptFinalHandler[] = "Handler";      /// Суффикс обработчика конечного состояния.
const char ScriptEnterHandler[] = "EnterHandler"; /// Суффикс обработчика входа в состояние.
const char ScriptExitHandler[] = "ExitHandler";   /// Суффикс обработчика выхода в состояние.
const char ScriptCanStopFunction[] =
    "canStop"; /// Возвращает false, если сценарий не может быть остановлен в текущий момент.

/// Параметры состояний.
const char ParamInitial[] = "initial";                 /// Начальное состояние.
const char ParamFinal[] = "final";                     /// Конечное состояние.
const char ParamTimeout[] = "timeout";                 /// Таймаут состояния.
const char ParamUserActivity[] = "ignoreUserActivity"; /// Игнорировать активность пользователя.
const char ParamSignalName[] = "signal"; /// Имя сигнала, при которому мы пришли в данное состояние.
const char ParamResult[] = "result";     /// Результат работы сценария.
const char ParamResultError[] = "resultError"; /// Ошибка, возвращаемая сценарием.
} // namespace CJSScenario

//---------------------------------------------------------------------------
/// Событие сценария.
class ScenarioEvent : public QEvent {
public:
    /// Пользовательский тип.
    static const int Type = QEvent::User + 1;

    ScenarioEvent(QString aSignal) : QEvent(QEvent::Type(Type)), m_signal(std::move(aSignal)) {}
    QString getSignal() const { return m_signal; }

private:
    QString m_signal;
};

//---------------------------------------------------------------------------
/// Переход между состояниями сценария.
class ScenarioTransition : public QAbstractTransition {
public:
    ScenarioTransition(QString aSignal) : m_signal(std::move(aSignal)) {}

    void onTransition(QEvent * /*event*/) override {}
    bool eventTest(QEvent *aEvent) override {
        if (aEvent->type() == ScenarioEvent::Type) {
            auto *se = dynamic_cast<ScenarioEvent *>(aEvent);
            return m_signal == se->getSignal();
        }

        return false;
    }

private:
    QString m_signal;
};

//---------------------------------------------------------------------------
JSScenario::JSScenario(const QString &aName, QString aPath, QString aBasePath, ILog *aLog)
    : Scenario(aName, aLog), m_Path(std::move(aPath)), m_BasePath(std::move(aBasePath)),
      m_IsPaused(true), m_DefaultTimeout(0) {
    connect(
        &m_EnterSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onEnterState(const QString &)));
    connect(
        &m_ExitSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onExitState(const QString &)));
    connect(&m_TimeoutTimer, SIGNAL(timeout()), SLOT(onTimeout()));
}

//---------------------------------------------------------------------------
JSScenario::~JSScenario() {
    // Stop timers and cleanup resources
    if (m_TimeoutTimer.isActive()) {
        m_TimeoutTimer.stop();
    }

    // Clear state machine
    if (m_StateMachine) {
        m_StateMachine->stop();
        // Note: QStateMachine doesn't have clear() method in Qt6
        // States are managed individually
    }

    // Clear states
    m_States.clear();
    m_Transitions.clear();
    m_Hooks.clear();

    // Script engine will be automatically cleaned up by QSharedPointer
}

//---------------------------------------------------------------------------
void JSScenario::start(const QVariantMap &aContext) {
    m_Context = aContext;
    m_CurrentState = m_InitialState;

    m_IsPaused = false;

    functionCall(CJSScenario::ScriptStartFunction,
                 QVariantMap(),
                 m_Name + ":" + CJSScenario::ScriptStartFunction);

    m_SignalArguments.clear();
    m_SignalArguments[CJSScenario::ParamSignalName] = "start";

    m_StateMachine->setInitialState(m_States[m_CurrentState].qstate);
    m_StateMachine->start();
}

//---------------------------------------------------------------------------
void JSScenario::pause() {
    m_IsPaused = true;

    m_TimeoutTimer.stop();
    m_StateMachine->stop();

    functionCall(CJSScenario::ScriptPauseFunction,
                 QVariantMap(),
                 m_Name + ":" + CJSScenario::ScriptPauseFunction);
}

//---------------------------------------------------------------------------
void JSScenario::resume(const QVariantMap &aContext) {
    // Очищаем контекст от совпадающих ключей
    foreach (QString key, aContext.keys()) {
        m_Context.remove(key);
    }

    // Объединяем контексты
    m_Context.insert(aContext);

    m_IsPaused = false;

    functionCall(CJSScenario::ScriptResumeFunction,
                 QVariantMap(),
                 m_Name + ":" + CJSScenario::ScriptResumeFunction);

    m_SignalArguments.clear();
    m_SignalArguments[CJSScenario::ParamSignalName] = "resume";

    m_StateMachine->setInitialState(m_States[m_CurrentState].qstate);
    m_StateMachine->start();
}

//---------------------------------------------------------------------------
bool JSScenario::initialize(const QList<SScriptObject> &aScriptObjects) {
    m_StateMachine = QSharedPointer<QStateMachine>(new QStateMachine);
    m_ScriptEngine = QSharedPointer<QJSEngine>(new QJSEngine);

    connect(m_StateMachine.data(), SIGNAL(finished()), this, SLOT(onFinish()));

    // Add script objects to the engine
    foreach (const SScriptObject &object, aScriptObjects) {
        if (object.isType) {
            m_ScriptEngine->globalObject().setProperty(
                object.name, m_ScriptEngine->newQMetaObject(object.metaObject));
        } else {
            m_ScriptEngine->globalObject().setProperty(object.name,
                                                       m_ScriptEngine->newQObject(object.object));
        }
    }

    // Register this scenario object
    m_ScriptEngine->globalObject().setProperty(CJSScenario::ServiceName,
                                               m_ScriptEngine->newQObject(this));

    // Load base scenario if it exists
    if (!m_BasePath.isEmpty()) {
        if (!loadScript(m_ScriptEngine.data(), m_Name, m_BasePath)) {
            toLog(LogLevel::Error,
                  QString("Failed to load base scenario script %1 for scenario %2.")
                      .arg(m_BasePath)
                      .arg(m_Name));
            return false;
        }

        // Call initialize function in the base script
        functionCall(CJSScenario::ScriptInitFunction,
                     QVariantMap(),
                     m_Name + ":" + CJSScenario::ScriptInitFunction);
    }

    // Load main script
    if (!loadScript(m_ScriptEngine.data(), m_Name, m_Path)) {
        return false;
    }

    // Initialize the main script
    QJSValue result =
        functionCall(CJSScenario::ScriptInitFunction,
                     QVariantMap(),
                     QString("%1:%2").arg(m_Name).arg(CJSScenario::ScriptInitFunction));
    if (result.isError()) {
        toLog(LogLevel::Error,
              QString("Failed to initialize '%1' scenario: %2").arg(m_Name).arg(result.toString()));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void JSScenario::signalTriggered(const QString &aSignal, const QVariantMap &aArguments) {
    // Если переход не зарегистрирован, не обрабатываем его.
    if (!m_Transitions.contains(m_CurrentState, aSignal)) {
        toLog(LogLevel::Debug,
              QString("Transition from state '%1' by signal '%2' not found.")
                  .arg(m_CurrentState)
                  .arg(aSignal));

        return;
    }

    // State-машина была активирована, но еще не вошла в рабочее состояние (срабатывание приведет к
    // утечке памяти).
    if (!m_StateMachine->isRunning()) {
        return;
    }

    // Переход не был выполнен.
    if (!m_SignalArguments.isEmpty()) {
        toLog(LogLevel::Debug,
              QString(
                  "Transition from state '%1' was not completed but new signal '%2' was happened.")
                  .arg(m_CurrentState)
                  .arg(aSignal));

        return;
    }

    m_SignalArguments = aArguments;
    m_SignalArguments[CJSScenario::ParamSignalName] = aSignal;

    m_StateMachine->postEvent(new ScenarioEvent(aSignal));
}

//---------------------------------------------------------------------------
bool JSScenario::canStop() {
    QJSValue result =
        functionCall(CJSScenario::ScriptCanStopFunction,
                     QVariantMap(),
                     QString("%1:%2").arg(m_Name).arg(CJSScenario::ScriptCanStopFunction));

    if (result.isError()) {
        toLog(LogLevel::Error,
              QString("Failed to call '%1' scenario: %2").arg(m_Name).arg(result.toString()));
        return true;
    }

    return result.toBool();
}

//---------------------------------------------------------------------------
QString JSScenario::getState() const {
    return m_CurrentState;
}

//---------------------------------------------------------------------------
void JSScenario::addState(const QString &aStateName, const QVariantMap &aParameters) {
    Q_ASSERT(!aStateName.isEmpty());

    SState state;
    state.name = aStateName;
    state.parameters = aParameters;

    if (state.name.isEmpty()) {
        toLog(LogLevel::Error,
              QString("Failed to add a state to '%1' scenario: name not specified.").arg(m_Name));
        return;
    }

    // Начальное, конечное или обычное состояние.
    if (aParameters.contains(CJSScenario::ParamFinal)) {
        state.qstate = new QFinalState();
    } else {
        if (aParameters.contains(CJSScenario::ParamInitial)) {
            m_InitialState = state.name;
        }

        state.qstate = new QState();
    }

    m_EnterSignalMapper.connect(state.qstate, SIGNAL(entered()), SLOT(map()));
    m_EnterSignalMapper.setMapping(state.qstate, aStateName);

    m_ExitSignalMapper.connect(state.qstate, SIGNAL(exited()), SLOT(map()));
    m_ExitSignalMapper.setMapping(state.qstate, aStateName);

    m_States[aStateName] = state;
    m_StateMachine->addState(state.qstate);
}

//---------------------------------------------------------------------------
void JSScenario::addTransition(const QString &aSource,
                               const QString &aTarget,
                               const QString &aSignal) {
    Q_ASSERT(!aSignal.isEmpty());

    TStateList::iterator src = m_States.find(aSource);
    TStateList::iterator dst = m_States.find(aTarget);

    if (src == m_States.end() || dst == m_States.end()) {
        toLog(LogLevel::Error,
              QString("Failed to add '%1->%2' transition to '%3' scenario: either source or target "
                      "state doesn't exist.")
                  .arg(aSource)
                  .arg(aTarget)
                  .arg(m_Name));
        return;
    }

    std::unique_ptr<ScenarioTransition> transition(new ScenarioTransition(aSignal));
    transition->setTargetState(dst->qstate);

    auto *state = dynamic_cast<QState *>(src->qstate);
    if (!state) {
        toLog(LogLevel::Error,
              QString("Failed to add '%1->%2' transition to '%3' scenario: source state cannot "
                      "have transitions.")
                  .arg(aSource)
                  .arg(aTarget)
                  .arg(m_Name));
        return;
    }

    state->addTransition(transition.release());
    m_Transitions.insert(aSource, aSignal);
}

//---------------------------------------------------------------------------
void JSScenario::setDefaultTimeout(int aSeconds, const QJSValue &aHandler) {
    m_DefaultTimeout = aSeconds;
    m_TimeoutHandler = aHandler;
}

//---------------------------------------------------------------------------
void JSScenario::onTimeout() {
    bool handled = false;

    if (m_TimeoutHandler.isCallable()) {
        QJSValue result =
            m_TimeoutHandler.call(QJSValueList() << m_ScriptEngine->toScriptValue(m_CurrentState));
        if (result.isError()) {
            toLog(LogLevel::Error,
                  QString("An exception occured during executing '%1' timeout handler: %2.")
                      .arg(m_Name)
                      .arg(result.toString()));
        } else {
            handled = result.toBool();
        }
    }

    if (!handled) {
        m_SignalArguments.clear();
        signalTriggered("timeout");
    }

    resetTimeout();
}

//---------------------------------------------------------------------------
void JSScenario::setStateTimeout(int aSeconds) {
    m_TimeoutTimer.stop();
    m_TimeoutTimer.setSingleShot(true);
    m_TimeoutTimer.setInterval(aSeconds * 1000); // Convert to milliseconds
    m_TimeoutTimer.start();
}

//---------------------------------------------------------------------------
void JSScenario::onEnterState(const QString &aState) {
    if (m_IsPaused) {
        toLog(LogLevel::Warning,
              QString("Scenario %1 is paused. Skip state %2.").arg(m_Name).arg(aState));
        return;
    }

    TStateList::iterator s = m_States.find(aState);
    bool final = s->parameters.contains(CJSScenario::ParamFinal);

    toLog(LogLevel::Normal, QString("ENTER %1 %2state.").arg(aState).arg(final ? "final " : ""));

    QVariantMap::iterator t = s->parameters.find("timeout");
    int timeout = m_DefaultTimeout;

    if (t != s->parameters.end()) {
        timeout = t->toInt();
    }

    if (timeout > 0) {
        setStateTimeout(timeout);
    }

    m_CurrentState = aState;

    // Если достигли конечного состояния - копируем результат.
    if (final) {
        m_Context[CJSScenario::ParamResult] = s->parameters[CJSScenario::ParamResult];
    }

    foreach (Scenario::SExternalStateHook hook, m_Hooks) {
        if (hook.targetScenario == getName() && hook.targetState == m_CurrentState) {
            m_SignalArguments = hook.hook(getContext(), m_SignalArguments);
        }
    }

    QString handler =
        aState + (final ? CJSScenario::ScriptFinalHandler : CJSScenario::ScriptEnterHandler);
    functionCall(
        handler, m_SignalArguments, QString("%1:%2:%3").arg(m_Name).arg(aState).arg(handler));

    m_SignalArguments.clear();
}

//---------------------------------------------------------------------------
void JSScenario::onExitState(const QString &aState) {
    if (m_IsPaused) {
        toLog(LogLevel::Warning,
              QString("Scenario %1 in da pause. Skip state %2.").arg(m_Name).arg(aState));
        return;
    }

    QString handler = aState + CJSScenario::ScriptExitHandler;
    functionCall(
        handler, m_SignalArguments, QString("%1:%2:%3").arg(m_Name).arg(aState).arg(handler));

    m_TimeoutTimer.stop();
}

//---------------------------------------------------------------------------
void JSScenario::onFinish() {
    m_TimeoutTimer.stop();

    QJSValue resultError =
        functionCall(CJSScenario::ScriptStopFunction,
                     QVariantMap(),
                     QString("%1:%2").arg(m_Name).arg(CJSScenario::ScriptStopFunction));

    // Помещаем в контекст возвращаемое сценарием значение
    m_Context[CJSScenario::ParamResultError] = resultError.toVariant();

    emit finished(m_Context);
}

//---------------------------------------------------------------------------
void JSScenario::onException(const QJSValue &aException) {
    if (aException.isError()) {
        toLog(LogLevel::Error, QString("QML Script error: %1").arg(aException.toString()));
    }
}

//---------------------------------------------------------------------------
QJSValue JSScenario::functionCall(const QString &aFunction,
                                  const QVariantMap &aArguments,
                                  const QString &aNameForLog) {
    QJSValue function = m_ScriptEngine->globalObject().property(aFunction);

    if (!function.isCallable()) {
        toLog(LogLevel::Debug, QString("Function '%1' is not callable.").arg(aNameForLog));
        return {};
    }

    toLog(LogLevel::Normal, QString("CALL %1 function.").arg(aNameForLog));

    QJSValue result;
    if (aArguments.isEmpty()) {
        result = function.call();
    } else {
        // Convert QVariantMap to QJSValue
        QJSValue args = m_ScriptEngine->toScriptValue(aArguments);
        result = function.call(QJSValueList() << args);
    }

    // Check for errors
    if (result.isError()) {
        toLog(LogLevel::Error,
              QString("Exception in %1: %2").arg(aNameForLog).arg(result.toString()));
    }

    return result;
}

//---------------------------------------------------------------------------
QJSValue JSScenario::includeScript(void *aContext, QJSEngine *aEngine, void *aScenario) {
    auto *self = static_cast<JSScenario *>(aScenario);

    // For Qt6 QJSEngine, we implement a simpler include mechanism
    // We expect the first argument to be the file path to include
    // Note: Qt6 QJSEngine doesn't have the same context management as Qt5 QScriptEngine

    // This is a placeholder implementation for Qt6
    // A full implementation would require more complex JavaScript context management
    // Note: Cannot use toLog() here as this is a static method
    qWarning() << "includeScript called but not fully implemented in Qt6 QJSEngine";
    return {false};
}

//---------------------------------------------------------------------------
bool JSScenario::loadScript(QJSEngine *aScriptEngine,
                            const QString &aScenarioName,
                            const QString &aScriptPath) {
    QFile scriptFile(aScriptPath);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        toLog(LogLevel::Error, QString("Cannot open script file: %1").arg(aScriptPath));
        return false;
    }

    QTextStream stream(&scriptFile);
    QString script = stream.readAll();
    scriptFile.close();

    QJSValue result = aScriptEngine->evaluate(script, aScriptPath);
    if (result.isError()) {
        toLog(LogLevel::Error, QString("Script evaluation error: %1").arg(result.toString()));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
QVariantMap JSScenario::getContext() const {
    return m_Context;
}

} // namespace GUI
