/* @file Сценарий на основе javascript. */

#include <QtGlobal>

#include <memory>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtStateMachine/QAbstractTransition>
#include <QtStateMachine/QFinalState>
#else
#include <QtCore/QAbstractTransition>
#include <QtCore/QFinalState>
#endif
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#else
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#endif

#include "JSScenario.h"

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
const char Param_Initial[] = "initial";                 /// Начальное состояние.
const char Param_Final[] = "final";                     /// Конечное состояние.
const char Param_Timeout[] = "timeout";                 /// Таймаут состояния.
const char Param_UserActivity[] = "ignoreUserActivity"; /// Игнорировать активность пользователя.
const char Param_SignalName[] = "signal"; /// Имя сигнала, при которому мы пришли в данное состояние.
const char Param_Result[] = "result";     /// Результат работы сценария.
const char Param_ResultError[] = "resultError"; /// Ошибка, возвращаемая сценарием.
} // namespace CJSScenario

//---------------------------------------------------------------------------
/// Событие сценария.
class ScenarioEvent : public QEvent {
public:
    /// Пользовательский тип.
    static const int Type = QEvent::User + 1;

    ScenarioEvent(const QString &aSignal) : QEvent(QEvent::Type(Type)), m_Signal(aSignal) {}
    QString getSignal() const { return m_Signal; }

private:
    QString m_Signal;
};

//---------------------------------------------------------------------------
/// Переход между состояниями сценария.
class ScenarioTransition : public QAbstractTransition {
public:
    ScenarioTransition(const QString &aSignal) : m_Signal(aSignal) {}

    virtual void onTransition(QEvent *) {}
    virtual bool eventTest(QEvent *aEvent) {
        if (aEvent->type() == ScenarioEvent::Type) {
            ScenarioEvent *se = static_cast<ScenarioEvent *>(aEvent);
            return m_Signal == se->getSignal();
        }

        return false;
    }

private:
    QString m_Signal;
};

//---------------------------------------------------------------------------
JSScenario::JSScenario(const QString &aName,
                       const QString &aPath,
                       const QString &aBasePath,
                       ILog *aLog)
    : Scenario(aName, aLog), m_BasePath(aBasePath), m_Path(aPath), m_IsPaused(true) {
    connect(
        &m_EnterSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onEnterState(const QString &)));
    connect(
        &m_ExitSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onExitState(const QString &)));
}

//---------------------------------------------------------------------------
JSScenario::~JSScenario() {}

//---------------------------------------------------------------------------
void JSScenario::start(const QVariantMap &aContext) {
    m_Context = aContext;
    m_CurrentState = m_InitialState;

    m_IsPaused = false;

    functionCall(CJSScenario::ScriptStartFunction,
                 QVariantMap(),
                 m_Name + ":" + CJSScenario::ScriptStartFunction);

    m_SignalArguments.clear();
    m_SignalArguments[CJSScenario::Param_SignalName] = "start";

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

    // Объединяем контексты: в Qt6 метод unite() был удален, используем insert()
    // Qt5: unite() - объединяет карты, перезаписывая существующие ключи
    // Qt6: insert() - аналогичная функциональность после удаления unite()
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_Context.insert(aContext);
#else
    m_Context.unite(aContext);
#endif

    m_IsPaused = false;

    functionCall(CJSScenario::ScriptResumeFunction,
                 QVariantMap(),
                 m_Name + ":" + CJSScenario::ScriptResumeFunction);

    m_SignalArguments.clear();
    m_SignalArguments[CJSScenario::Param_SignalName] = "resume";

    m_StateMachine->setInitialState(m_States[m_CurrentState].qstate);
    m_StateMachine->start();
}

//---------------------------------------------------------------------------
bool JSScenario::initialize(const QList<SScriptObject> &aScriptObjects) {
    m_StateMachine = QSharedPointer<QStateMachine>(new QStateMachine);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_ScriptEngine = QSharedPointer<QJSEngine>(new QJSEngine);
#else
    m_ScriptEngine = QSharedPointer<QScriptEngine>(new QScriptEngine);
#endif

    connect(m_StateMachine.data(), SIGNAL(finished()), this, SLOT(onFinish()));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6: QJSEngine does not have signalHandlerException
#else
    connect(m_ScriptEngine.data(),
            SIGNAL(signalHandlerException(const QScriptValue &)),
            this,
            SLOT(onException(const QScriptValue &)));
#endif

    // Добавляем в скрипты внешние объекты.
    foreach (const SScriptObject &object, aScriptObjects) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_ScriptEngine->globalObject().setProperty(
            object.name,
            object.isType ? m_ScriptEngine->newQMetaObject(object.metaObject)
                          : m_ScriptEngine->newQObject(object.object));
#else
        m_ScriptEngine->globalObject().setProperty(
            object.name,
            object.isType ? m_ScriptEngine->newQMetaObject(object.metaObject)
                          : m_ScriptEngine->newQObject(object.object));
#endif
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_ScriptEngine->globalObject().setProperty(CJSScenario::ServiceName,
                                              m_ScriptEngine->newQObject(this));
    // Qt6: QJSEngine does not support newFunction, so includeScript must be handled differently
    // Placeholder: migration logic for includeScript registration
#else
    m_ScriptEngine->globalObject().setProperty(CJSScenario::ServiceName,
                                              m_ScriptEngine->newQObject(this));
    m_ScriptEngine->globalObject().setProperty(
        CJSScenario::ScriptIncludeFunction,
        m_ScriptEngine->newFunction(&JSScenario::includeScript, this));
    m_ScriptEngine->installTranslatorFunctions();
#endif

    // Загружаем базовый сценарий, если такой имеется.
    if (!m_BasePath.isEmpty()) {
        if (!loadScript(m_ScriptEngine.data(), m_Name, m_BasePath)) {
            toLog(LogLevel::Error,
                  QString("Failed to load base scenario script %1 for scenario %2.")
                      .arg(m_BasePath)
                      .arg(m_Name));
            return false;
        }

        // Делаем вызов функции initialize в скрипте.
        functionCall(CJSScenario::ScriptInitFunction,
                     QVariantMap(),
                     m_Name + ":" + CJSScenario::ScriptInitFunction);
    }

    if (!loadScript(m_ScriptEngine.data(), m_Name, m_Path)) {
        return false;
    }

    // Инициализируем скрипт сценария.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QJSValue result =
        functionCall(CJSScenario::ScriptInitFunction,
                     QVariantMap(),
                     QString("%1:%2").arg(m_Name).arg(CJSScenario::ScriptInitFunction));
    if (result.isError()) {
        toLog(LogLevel::Error,
              QString("Failed to initialize '%1' scenario: %2").arg(m_Name).arg(result.toString()));
        return false;
    }
#else
    QScriptValue result =
        functionCall(CJSScenario::ScriptInitFunction,
                     QVariantMap(),
                     QString("%1:%2").arg(m_Name).arg(CJSScenario::ScriptInitFunction));
    if (result.isError()) {
        toLog(LogLevel::Error,
              QString("Failed to initialize '%1' scenario: %2").arg(m_Name).arg(result.toString()));
        return false;
    }
#endif
    return true;
}

//---------------------------------------------------------------------------
bool JSScenario::canStop() {
    QScriptValue result =
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
    m_SignalArguments[CJSScenario::Param_SignalName] = aSignal;

    m_StateMachine->postEvent(new ScenarioEvent(aSignal));
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
    if (aParameters.contains(CJSScenario::Param_Final)) {
        state.qstate = new QFinalState();
    } else {
        if (aParameters.contains(CJSScenario::Param_Initial)) {
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

    QState *state = dynamic_cast<QState *>(src->qstate);
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
void JSScenario::setDefaultTimeout(int aSeconds, const QScriptValue &aHandler) {
    m_DefaultTimeout = aSeconds;
    m_TimeoutHandler = aHandler;
}

//---------------------------------------------------------------------------
void JSScenario::onEnterState(const QString &aState) {
    if (m_IsPaused) {
        toLog(LogLevel::Warning,
              QString("Scenario %1 is paused. Skip state %2.").arg(m_Name).arg(aState));
        return;
    }

    TStateList::iterator s = m_States.find(aState);
    bool final = s->parameters.contains(CJSScenario::Param_Final);

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
        m_Context[CJSScenario::Param_Result] = s->parameters[CJSScenario::Param_Result];
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
void JSScenario::onTimeout() {
    bool handled = false;

    if (m_TimeoutHandler.isFunction()) {
        QScriptValue result =
            m_TimeoutHandler.call(QScriptValue(), QScriptValueList() << m_CurrentState);
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
void JSScenario::onFinish() {
    m_TimeoutTimer.stop();

    QScriptValue resultError =
        functionCall(CJSScenario::ScriptStopFunction,
                     QVariantMap(),
                     QString("%1:%2").arg(m_Name).arg(CJSScenario::ScriptStopFunction));

    // Помещаем в контекст возвращаемое сценарием значение
    m_Context[CJSScenario::Param_ResultError] = resultError.toVariant();

    emit finished(m_Context);
}

//---------------------------------------------------------------------------
QString JSScenario::getState() const {
    return m_CurrentState;
}

//---------------------------------------------------------------------------
QVariantMap JSScenario::getContext() const {
    return m_Context;
}

//---------------------------------------------------------------------------
QScriptValue JSScenario::functionCall(const QString &aFunction,
                                      const QVariantMap &aArguments,
                                      const QString &aNameForLog) {
    QScriptValue function = m_ScriptEngine->globalObject().property(aFunction);

    QScriptValue result;

    if (function.isValid()) {
        toLog(LogLevel::Normal, QString("CALL %1 function.").arg(aNameForLog));

        if (aArguments.isEmpty()) {
            result = function.call();
        } else {
            QScriptValueList arguments;
            arguments << function.engine()->toScriptValue<QVariantMap>(aArguments);

            result = function.call(QScriptValue(), arguments);
        }

        if (function.engine()->hasUncaughtException()) {
            toLog(LogLevel::Error,
                  QString("An exception occured while calling %1(line %2): %3\nBacktrace:\n%4.")
                      .arg(aNameForLog)
                      .arg(function.engine()->uncaughtExceptionLineNumber())
                      .arg(function.engine()->uncaughtException().toString())
                      .arg(function.engine()->uncaughtExceptionBacktrace().join("\n")));
        } else if (!result.isValid()) {
            toLog(LogLevel::Error, QString("%1 is not a function.").arg(aNameForLog));
        }
    } else {
        toLog(LogLevel::Debug,
              QString("Failed to call %1, not a valid script object.").arg(aNameForLog));
    }

    return result;
}

//---------------------------------------------------------------------------
bool JSScenario::loadScript(QScriptEngine *aScriptEngine,
                            const QString &aScenarioName,
                            const QString &aScriptPath) {
    // Загружаем скрипт сценария.
    QFile script(aScriptPath);
    if (!script.open(QIODevice::ReadOnly | QIODevice::Text)) {
        toLog(LogLevel::Error,
              QString("Failed to open '%1' scenario script %2: %3")
                  .arg(aScenarioName)
                  .arg(aScriptPath)
                  .arg(script.errorString()));
        return false;
    }

    QTextStream stream(&script);
    QString program = stream.readAll();

    // Проверка синтаксиса скрипта.
    QScriptSyntaxCheckResult syntax = QScriptEngine::checkSyntax(program);
    if (syntax.state() == QScriptSyntaxCheckResult::Error) {
        toLog(LogLevel::Error,
              QString("Failed to execute '%1'. Syntax error at line %2 column %3. %4.")
                  .arg(aScriptPath)
                  .arg(syntax.errorLineNumber())
                  .arg(syntax.errorColumnNumber())
                  .arg(syntax.errorMessage()));

        return false;
    }

    QScriptValue result = aScriptEngine->evaluate(program);
    if (result.isError()) {
        toLog(LogLevel::Error,
              QString("Failed to execute '%1' scenario script: %2")
                  .arg(aScenarioName)
                  .arg(result.toString()));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
QScriptValue
JSScenario::includeScript(QScriptContext *aContext, QScriptEngine *aEngine, void *aScenario) {
    JSScenario *self = static_cast<JSScenario *>(aScenario);

    QString includeFilePath =
        self->m_Path.section("/", 0, -2) + "/" + aContext->argument(0).toString();
    QString namespaceName = aContext->argument(1).toString();

    if (!namespaceName.isEmpty()) {
        aEngine->pushContext();

        // Добавляем другой файл в текущий контекст.
        if (!self->loadScript(aEngine, self->m_Name, includeFilePath)) {
            aEngine->popContext();
            return QScriptValue(false);
        }

        // Сохраняем контекст активации.
        QScriptValue vars = aEngine->currentContext()->activationObject();
        aEngine->popContext();

        // И добавляем его под другим именем.
        aEngine->globalObject().setProperty(namespaceName, vars);

        return QScriptValue(true);
    } else {
        // Устанавливаем объект активации от внешнего контекста.
        aContext->setActivationObject(aContext->parentContext()->activationObject());

        // Добавляем другой скрипт в текущий контекст.
        if (self->loadScript(aEngine, self->m_Name, includeFilePath)) {
            return QScriptValue(true);
        }

        return QScriptValue(false);
    }
}

//---------------------------------------------------------------------------
void JSScenario::onException(const QScriptValue &aException) {
    toLog(LogLevel::Error,
          QString("An exception occured in scenario %1: %2")
              .arg(getName())
              .arg(aException.toString()));
}

} // namespace GUI

//---------------------------------------------------------------------------
