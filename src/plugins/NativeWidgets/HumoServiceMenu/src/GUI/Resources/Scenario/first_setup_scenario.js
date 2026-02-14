// Scenario initializer
function initialize(scenarioName) {
    // Algorithm states
    ScenarioEngine.addState("main", { initial: true });
    ScenarioEngine.addState("done", { final: true });

    ScenarioEngine.addTransition("main", "done", "exit");
}

// Scenario finalizer
function finalize() {}

// State handlers
function mainEnterHandler() {
    Core.graphics.show("FirstSetup", { reset: true });
}

function mainExitHandler() {}
