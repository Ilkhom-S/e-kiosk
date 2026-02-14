/* @file Сценарий меню обслуживания. */

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize(scenarioName) {
    // Состояния
    ScenarioEngine.addState("main", { initial: true });
    ScenarioEngine.addState("done", { final: true });

    // Переходы между состояниями
    ScenarioEngine.addTransition("main", "done", "close");
}

//------------------------------------------------------------------------------
// Событие запуска.
function onStart() {}

// Событие остановки.
function onStop(aParams) {}

// Можно ли остановить.
function canStop() {
    Core.graphics.notify(EventType.TryStopScenario, {});

    return false;
}

//------------------------------------------------------------------------------
// Обработчик входа в состояние main.
function mainEnterHandler() {
    Core.graphics.show("ServiceMenu", { reset: true });
}
