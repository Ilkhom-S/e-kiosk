# ScenarioEngine

Payment workflow execution engine.

## Purpose

Executes payment scenarios (workflows):

- Screen navigation
- Payment processing steps
- Device interaction coordination
- State management

## Architecture

```mermaid
flowchart TB
    ScenarioEngine --> IScenarioEngine["IScenarioEngine (interface)"]
    ScenarioEngine --> ScenarioRunner
    ScenarioEngine --> SceneManager
    ScenarioEngine --> Backend["ScenarioBackend (plugin)"]
    Backend --> UCS
    Backend --> Uniteller
    Backend --> Others["..."]
```

## Usage

```cpp
#include "ScenarioEngine/ScenarioEngine.h"

ScenarioEngine* engine = ScenarioEngine::instance();

// Load scenario
engine->loadScenario("payment_flow");

// Start execution
engine->start();

// Handle events
connect(engine, &ScenarioEngine::showScreen,
        graphics, &GraphicsEngine::displayScreen);
connect(engine, &ScenarioEngine::paymentComplete,
        this, &MyClass::onPaymentDone);
```

## Key Files

| File                 | Purpose           |
| -------------------- | ----------------- |
| `ScenarioEngine.h`   | Main engine       |
| `IScenarioBackend.h` | Backend interface |
| `SceneManager.h`     | Screen management |

## Scenario Flow

```mermaid
flowchart TB
    StartScene --> SelectService
    SelectService --> EnterAmount
    EnterAmount --> AcceptPayment
    AcceptPayment -->|"DeviceManager"| ProcessPayment
    ProcessPayment -->|"NetworkTaskManager"| PrintReceipt
    PrintReceipt -->|"DeviceManager"| Complete
```

## Dependencies

- `GraphicsEngine` module
- `DeviceManager` module
- `NetworkTaskManager` module
- `PPSDK` module

## Platform Support

| Platform | Status          |
| -------- | --------------- |
| Windows  | ✅ Full         |
| Linux    | 🔬 Experimental |
| macOS    | 🔬 Experimental |

## Qt5/Qt6 Scripting Compatibility

This module now supports both Qt5 (QtScript) and Qt6 (QJSEngine) for JavaScript scenarios.

- Qt5: Uses QScriptEngine/QScriptValue
- Qt6: Uses QJSEngine/QJSValue

All scenario scripting logic is handled internally and is transparent to modules/plugins using ScenarioEngine.

See [qt5-qt6-scripting-migration.md](../../../../docs/qt5-qt6-scripting-migration.md) for migration details and API notes.
