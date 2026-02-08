/* @file Сценарий простоя. */

#pragma once

#include <SDK/PaymentProcessor/Core/Event.h>

// modules
#include <ScenarioEngine/Scenario.h>

class IApplication;

//---------------------------------------------------------------------------
namespace CGUISignals {
extern const char StartGUI[];
extern const char StopGUI[];
extern const char UpdateGUI[];
extern const char ScreenPasswordUpdated[];
} // namespace CGUISignals

//---------------------------------------------------------------------------
class IdleScenario : public GUI::Scenario {
    Q_OBJECT
    Q_ENUMS(Command)

public:
    enum Command { None, Autoencashment };

public:
    IdleScenario(IApplication *m_Application);
    virtual ~IdleScenario();

    /// Запуск сценария.
    virtual void start(const QVariantMap &aContext);

    /// Остановка сценария.
    virtual void stop() {};

    /// Приостановить сценарий с возможностью последующего возобновления.
    virtual void pause();

    /// Продолжение после паузы.
    virtual void resume(const QVariantMap &aContext);

    /// Инициализация сценария.
    virtual bool initialize(const QList<GUI::SScriptObject> &aScriptObjects);

    /// Обработка сигнала из активного состояния с дополнительными аргументами.
    virtual void signalTriggered(const QString &aSignal,
                                 const QVariantMap &aArguments = QVariantMap());

    /// Возвращает false, если сценарий не может быть остановлен в текущий момент.
    virtual bool canStop();

public slots:
    /// Текущее состояние.
    virtual QString getState() const;

    /// Обработчик таймаута
    virtual void onTimeout();

private:
    /// Определяем новое состояние.
    void updateState(const QString &aSignal, const QVariantMap &aParameters);

    /// Выполнение команды.
    void execCommand();

private slots:
    /// Обработчик события.
    void onEvent(const SDK::PaymentProcessor::Event &aEvent);

protected:
    void timerEvent(QTimerEvent *aEvent);

private:
    IApplication *m_Application;
    QString m_DefaultScenario;
    Command m_Command;
    bool m_Active;
    bool m_NoGui;
    int m_InterfaceLockedTimer;
};

//---------------------------------------------------------------------------
