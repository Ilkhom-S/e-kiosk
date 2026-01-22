/* @file Реализация JSScenario для Qt6 с QML QJSEngine. */

// STL
#include <memory>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtGlobal>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QtStateMachine/QAbstractTransition>
#include <QtStateMachine/QFinalState>
#include <Common/QtHeadersEnd.h>

// Project
#include "JSScenario6.h"

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
        const char ParamSignalName[] = "signal";       /// Имя сигнала, при которому мы пришли в данное состояние.
        const char ParamResult[] = "result";           /// Результат работы сценария.
        const char ParamResultError[] = "resultError"; /// Ошибка, возвращаемая сценарием.
    } // namespace CJSScenario

    //---------------------------------------------------------------------------
    /// Событие сценария.
    class ScenarioEvent : public QEvent {
      public:
        /// Пользовательский тип.
        static const int Type = QEvent::User + 1;

        ScenarioEvent(const QString &aSignal) : QEvent(QEvent::Type(Type)), mSignal(aSignal) {
        }
        QString getSignal() const {
            return mSignal;
        }

      private:
        QString mSignal;
    };

    //---------------------------------------------------------------------------
    /// Переход между состояниями сценария.
    class ScenarioTransition : public QAbstractTransition {
      public:
        ScenarioTransition(const QString &aSignal) : mSignal(aSignal) {
        }

        virtual void onTransition(QEvent *) {
        }
        virtual bool eventTest(QEvent *aEvent) {
            if (aEvent->type() == ScenarioEvent::Type) {
                ScenarioEvent *se = static_cast<ScenarioEvent *>(aEvent);
                return mSignal == se->getSignal();
            }

            return false;
        }

      private:
        QString mSignal;
    };

    //---------------------------------------------------------------------------
    JSScenario::JSScenario(const QString &aName, const QString &aPath, const QString &aBasePath, ILog *aLog)
        : Scenario(aName, aLog), mPath(aPath), mBasePath(aBasePath), mIsPaused(true), mDefaultTimeout(0) {
        connect(&mEnterSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onEnterState(const QString &)));
        connect(&mExitSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onExitState(const QString &)));
        connect(&mTimeoutTimer, SIGNAL(timeout()), SLOT(onTimeout()));
    }

    //---------------------------------------------------------------------------
    JSScenario::~JSScenario() {
        // Stop timers and cleanup resources
        if (mTimeoutTimer.isActive()) {
            mTimeoutTimer.stop();
        }

        // Clear state machine
        if (mStateMachine) {
            mStateMachine->stop();
            // Note: QStateMachine doesn't have clear() method in Qt6
            // States are managed individually
        }

        // Clear states
        mStates.clear();
        mTransitions.clear();
        mHooks.clear();

        // Script engine will be automatically cleaned up by QSharedPointer
    }

    //---------------------------------------------------------------------------
    void JSScenario::start(const QVariantMap &aContext) {
        mContext = aContext;
        mCurrentState = mInitialState;

        mIsPaused = false;

        functionCall(CJSScenario::ScriptStartFunction, QVariantMap(), mName + ":" + CJSScenario::ScriptStartFunction);

        mSignalArguments.clear();
        mSignalArguments[CJSScenario::ParamSignalName] = "start";

        mStateMachine->setInitialState(mStates[mCurrentState].qstate);
        mStateMachine->start();
    }

    //---------------------------------------------------------------------------
    void JSScenario::pause() {
        mIsPaused = true;

        mTimeoutTimer.stop();
        mStateMachine->stop();

        functionCall(CJSScenario::ScriptPauseFunction, QVariantMap(), mName + ":" + CJSScenario::ScriptPauseFunction);
    }

    //---------------------------------------------------------------------------
    void JSScenario::resume(const QVariantMap &aContext) {
        // Очищаем контекст от совпадающих ключей
        foreach (QString key, aContext.keys()) {
            mContext.remove(key);
        }

        // Объединяем контексты
        mContext.insert(aContext);

        mIsPaused = false;

        functionCall(CJSScenario::ScriptResumeFunction, QVariantMap(), mName + ":" + CJSScenario::ScriptResumeFunction);

        mSignalArguments.clear();
        mSignalArguments[CJSScenario::ParamSignalName] = "resume";

        mStateMachine->setInitialState(mStates[mCurrentState].qstate);
        mStateMachine->start();
    }

    //---------------------------------------------------------------------------
    bool JSScenario::initialize(const QList<SScriptObject> &aScriptObjects) {
        mStateMachine = QSharedPointer<QStateMachine>(new QStateMachine);
        mScriptEngine = QSharedPointer<QJSEngine>(new QJSEngine);

        connect(mStateMachine.data(), SIGNAL(finished()), this, SLOT(onFinish()));

        // Add script objects to the engine
        foreach (const SScriptObject &object, aScriptObjects) {
            if (object.isType) {
                mScriptEngine->globalObject().setProperty(object.name,
                                                          mScriptEngine->newQMetaObject(object.metaObject));
            } else {
                mScriptEngine->globalObject().setProperty(object.name, mScriptEngine->newQObject(object.object));
            }
        }

        // Register this scenario object
        mScriptEngine->globalObject().setProperty(CJSScenario::ServiceName, mScriptEngine->newQObject(this));

        // Load base scenario if it exists
        if (!mBasePath.isEmpty()) {
            if (!loadScript(mScriptEngine.data(), mName, mBasePath)) {
                toLog(LogLevel::Error,
                      QString("Failed to load base scenario script %1 for scenario %2.").arg(mBasePath).arg(mName));
                return false;
            }

            // Call initialize function in the base script
            functionCall(CJSScenario::ScriptInitFunction, QVariantMap(), mName + ":" + CJSScenario::ScriptInitFunction);
        }

        // Load main script
        if (!loadScript(mScriptEngine.data(), mName, mPath)) {
            return false;
        }

        // Initialize the main script
        QJSValue result = functionCall(CJSScenario::ScriptInitFunction, QVariantMap(),
                                       QString("%1:%2").arg(mName).arg(CJSScenario::ScriptInitFunction));
        if (result.isError()) {
            toLog(LogLevel::Error, QString("Failed to initialize '%1' scenario: %2").arg(mName).arg(result.toString()));
            return false;
        }

        return true;
    }

    //---------------------------------------------------------------------------
    void JSScenario::signalTriggered(const QString &aSignal, const QVariantMap &aArguments) {
        // Если переход не зарегистрирован, не обрабатываем его.
        if (!mTransitions.contains(mCurrentState, aSignal)) {
            toLog(LogLevel::Debug,
                  QString("Transition from state '%1' by signal '%2' not found.").arg(mCurrentState).arg(aSignal));

            return;
        }

        // State-машина была активирована, но еще не вошла в рабочее состояние (срабатывание приведет к утечке памяти).
        if (!mStateMachine->isRunning()) {
            return;
        }

        // Переход не был выполнен.
        if (!mSignalArguments.isEmpty()) {
            toLog(LogLevel::Debug,
                  QString("Transition from state '%1' was not completed but new signal '%2' was happened.")
                      .arg(mCurrentState)
                      .arg(aSignal));

            return;
        }

        mSignalArguments = aArguments;
        mSignalArguments[CJSScenario::ParamSignalName] = aSignal;

        mStateMachine->postEvent(new ScenarioEvent(aSignal));
    }

    //---------------------------------------------------------------------------
    bool JSScenario::canStop() {
        QJSValue result = functionCall(CJSScenario::ScriptCanStopFunction, QVariantMap(),
                                       QString("%1:%2").arg(mName).arg(CJSScenario::ScriptCanStopFunction));

        if (result.isError()) {
            toLog(LogLevel::Error, QString("Failed to call '%1' scenario: %2").arg(mName).arg(result.toString()));
            return true;
        }

        return result.toBool();
    }

    //---------------------------------------------------------------------------
    QString JSScenario::getState() const {
        return mCurrentState;
    }

    //---------------------------------------------------------------------------
    void JSScenario::addState(const QString &aStateName, const QVariantMap &aParameters) {
        Q_ASSERT(!aStateName.isEmpty());

        SState state;
        state.name = aStateName;
        state.parameters = aParameters;

        if (state.name.isEmpty()) {
            toLog(LogLevel::Error, QString("Failed to add a state to '%1' scenario: name not specified.").arg(mName));
            return;
        }

        // Начальное, конечное или обычное состояние.
        if (aParameters.contains(CJSScenario::ParamFinal)) {
            state.qstate = new QFinalState();
        } else {
            if (aParameters.contains(CJSScenario::ParamInitial)) {
                mInitialState = state.name;
            }

            state.qstate = new QState();
        }

        mEnterSignalMapper.connect(state.qstate, SIGNAL(entered()), SLOT(map()));
        mEnterSignalMapper.setMapping(state.qstate, aStateName);

        mExitSignalMapper.connect(state.qstate, SIGNAL(exited()), SLOT(map()));
        mExitSignalMapper.setMapping(state.qstate, aStateName);

        mStates[aStateName] = state;
        mStateMachine->addState(state.qstate);
    }

    //---------------------------------------------------------------------------
    void JSScenario::addTransition(const QString &aSource, const QString &aTarget, const QString &aSignal) {
        Q_ASSERT(!aSignal.isEmpty());

        TStateList::iterator src = mStates.find(aSource);
        TStateList::iterator dst = mStates.find(aTarget);

        if (src == mStates.end() || dst == mStates.end()) {
            toLog(
                LogLevel::Error,
                QString(
                    "Failed to add '%1->%2' transition to '%3' scenario: either source or target state doesn't exist.")
                    .arg(aSource)
                    .arg(aTarget)
                    .arg(mName));
            return;
        }

        std::unique_ptr<ScenarioTransition> transition(new ScenarioTransition(aSignal));
        transition->setTargetState(dst->qstate);

        QState *state = dynamic_cast<QState *>(src->qstate);
        if (!state) {
            toLog(LogLevel::Error,
                  QString("Failed to add '%1->%2' transition to '%3' scenario: source state cannot have transitions.")
                      .arg(aSource)
                      .arg(aTarget)
                      .arg(mName));
            return;
        }

        state->addTransition(transition.release());
        mTransitions.insert(aSource, aSignal);
    }

    //---------------------------------------------------------------------------
    void JSScenario::setDefaultTimeout(int aSeconds, const QJSValue &aHandler) {
        mDefaultTimeout = aSeconds;
        mTimeoutHandler = aHandler;
    }

    //---------------------------------------------------------------------------
    void JSScenario::onTimeout() {
        bool handled = false;

        if (mTimeoutHandler.isCallable()) {
            QJSValue result = mTimeoutHandler.call(QJSValueList() << mScriptEngine->toScriptValue(mCurrentState));
            if (result.isError()) {
                toLog(LogLevel::Error, QString("An exception occured during executing '%1' timeout handler: %2.")
                                           .arg(mName)
                                           .arg(result.toString()));
            } else {
                handled = result.toBool();
            }
        }

        if (!handled) {
            mSignalArguments.clear();
            signalTriggered("timeout");
        }

        resetTimeout();
    }

    //---------------------------------------------------------------------------
    void JSScenario::setStateTimeout(int aSeconds) {
        mTimeoutTimer.stop();
        mTimeoutTimer.setSingleShot(true);
        mTimeoutTimer.setInterval(aSeconds * 1000); // Convert to milliseconds
        mTimeoutTimer.start();
    }

    //---------------------------------------------------------------------------
    void JSScenario::onEnterState(const QString &aState) {
        if (mIsPaused) {
            toLog(LogLevel::Warning, QString("Scenario %1 is paused. Skip state %2.").arg(mName).arg(aState));
            return;
        }

        TStateList::iterator s = mStates.find(aState);
        bool final = s->parameters.contains(CJSScenario::ParamFinal);

        toLog(LogLevel::Normal, QString("ENTER %1 %2state.").arg(aState).arg(final ? "final " : ""));

        QVariantMap::iterator t = s->parameters.find("timeout");
        int timeout = mDefaultTimeout;

        if (t != s->parameters.end()) {
            timeout = t->toInt();
        }

        if (timeout > 0) {
            setStateTimeout(timeout);
        }

        mCurrentState = aState;

        // Если достигли конечного состояния - копируем результат.
        if (final) {
            mContext[CJSScenario::ParamResult] = s->parameters[CJSScenario::ParamResult];
        }

        foreach (Scenario::SExternalStateHook hook, mHooks) {
            if (hook.targetScenario == getName() && hook.targetState == mCurrentState) {
                mSignalArguments = hook.hook(getContext(), mSignalArguments);
            }
        }

        QString handler = aState + (final ? CJSScenario::ScriptFinalHandler : CJSScenario::ScriptEnterHandler);
        functionCall(handler, mSignalArguments, QString("%1:%2:%3").arg(mName).arg(aState).arg(handler));

        mSignalArguments.clear();
    }

    //---------------------------------------------------------------------------
    void JSScenario::onExitState(const QString &aState) {
        if (mIsPaused) {
            toLog(LogLevel::Warning, QString("Scenario %1 in da pause. Skip state %2.").arg(mName).arg(aState));
            return;
        }

        QString handler = aState + CJSScenario::ScriptExitHandler;
        functionCall(handler, mSignalArguments, QString("%1:%2:%3").arg(mName).arg(aState).arg(handler));

        mTimeoutTimer.stop();
    }

    //---------------------------------------------------------------------------
    void JSScenario::onFinish() {
        mTimeoutTimer.stop();

        QJSValue resultError = functionCall(CJSScenario::ScriptStopFunction, QVariantMap(),
                                            QString("%1:%2").arg(mName).arg(CJSScenario::ScriptStopFunction));

        // Помещаем в контекст возвращаемое сценарием значение
        mContext[CJSScenario::ParamResultError] = resultError.toVariant();

        emit finished(mContext);
    }

    //---------------------------------------------------------------------------
    void JSScenario::onException(const QJSValue &aException) {
        if (aException.isError()) {
            toLog(LogLevel::Error, QString("QML Script error: %1").arg(aException.toString()));
        }
    }

    //---------------------------------------------------------------------------
    QJSValue JSScenario::functionCall(const QString &aFunction, const QVariantMap &aArguments,
                                      const QString &aNameForLog) {
        QJSValue function = mScriptEngine->globalObject().property(aFunction);

        if (!function.isCallable()) {
            toLog(LogLevel::Debug, QString("Function '%1' is not callable.").arg(aNameForLog));
            return QJSValue();
        }

        toLog(LogLevel::Normal, QString("CALL %1 function.").arg(aNameForLog));

        QJSValue result;
        if (aArguments.isEmpty()) {
            result = function.call();
        } else {
            // Convert QVariantMap to QJSValue
            QJSValue args = mScriptEngine->toScriptValue(aArguments);
            result = function.call(QJSValueList() << args);
        }

        // Check for errors
        if (result.isError()) {
            toLog(LogLevel::Error, QString("Exception in %1: %2").arg(aNameForLog).arg(result.toString()));
        }

        return result;
    }

    //---------------------------------------------------------------------------
    QJSValue JSScenario::includeScript(void *aContext, QJSEngine *aEngine, void *aScenario) {
        JSScenario *self = static_cast<JSScenario *>(aScenario);

        // For Qt6 QJSEngine, we implement a simpler include mechanism
        // We expect the first argument to be the file path to include
        // Note: Qt6 QJSEngine doesn't have the same context management as Qt5 QScriptEngine

        // This is a placeholder implementation for Qt6
        // A full implementation would require more complex JavaScript context management
        // Note: Cannot use toLog() here as this is a static method
        qWarning() << "includeScript called but not fully implemented in Qt6 QJSEngine";
        return QJSValue(false);
    }

    //---------------------------------------------------------------------------
    bool JSScenario::loadScript(QJSEngine *aScriptEngine, const QString &aScenarioName, const QString &aScriptPath) {
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
        return mContext;
    }

} // namespace GUI
