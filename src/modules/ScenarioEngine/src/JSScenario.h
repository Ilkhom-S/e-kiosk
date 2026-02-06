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

    Q_PROPERTY(QVariantMap context READ getContext)

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

    virtual void setStateHook(const QList<SExternalStateHook> &aHook) { mHooks << aHook; }

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
    QString mPath;     /// Путь к сценарию.
    QString mBasePath; /// Путь к скрипту родительского сценария (если применяется наследование).
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QSharedPointer<QJSEngine> mScriptEngine; /// Скриптовый движок для обработчиков.
#else
    QSharedPointer<QScriptEngine> mScriptEngine; /// Скриптовый движок для обработчиков.
#endif
    QSharedPointer<QStateMachine> mStateMachine; /// Конечный автомат.
    TStateList mStates;                          /// Набор состояний.
    QSignalMapper mEnterSignalMapper;
    QSignalMapper mExitSignalMapper;
    QString mCurrentState;        /// Имя текущего состояния.
    QString mInitialState;        /// Начальное состояние.
    QVariantMap mSignalArguments; /// Параметры сигналов.
    QVariantMap mContext;         /// Контекст.
    QString mScriptPath;          /// Путь к файлу скрипты.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QJSValue mTimeoutHandler; /// Обработчик таймаута.
#else
    QScriptValue mTimeoutHandler; /// Обработчик таймаута.
#endif
    QMultiMap<QString, QString> mTransitions; /// Список возможных переходов для состояния.
    bool mIsPaused;                           /// Флаг для пропуска переходов в сценариях "на паузе"

    QList<Scenario::SExternalStateHook> mHooks;
};

} // namespace GUI

//---------------------------------------------------------------------------
