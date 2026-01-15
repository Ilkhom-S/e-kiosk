/* @file Сценарий автоинкассации. */

//------------------------------------------------------------------------------
// Инициализация сценария.
function initialize(scenarioName) {
  // Состояния
  ScenarioEngine.addState("main", { initial: true });
  ScenarioEngine.addState("done", { final: true });

  // Переходы между состояниями
  ScenarioEngine.addTransition("main", "done", "close");
}

//-----------------------------------------------------------------------------
// Событие запуска.
function onStart() {}

// Событие остановки.
function onStop() {}

// Можно ли остановить.
function canStop() {
  return false;
}

//-----------------------------------------------------------------------------
// Обработчик входа в состояние main.
function mainEnterHandler(aParam) {
  if (aParam.signal == "resume") {
    // Если сценарий уже выполняется, просто закроем его.
    Core.postEvent(EventType.UpdateScenario, "close");
  } else {
    Core.graphics.show("AutoEncashment", { reset: true });
  }
}
