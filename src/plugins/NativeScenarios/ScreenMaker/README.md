# ScreenMaker Plugin - Native Scenario for UI Screenshot Creation

ScreenMaker is a native scenario plugin that allows users to capture and annotate UI screenshots.

## Features

- **Screenshot Capture**: Capture the current UI display
- **Rectangle Drawing**: Draw rectangular annotations on the screenshot
- **Annotation Colors**: Green border for full screen, red rectangles for selections
- **Save Functionality**: Save annotated screenshots to disk
- **Clipboard Support**: Copy screenshots to clipboard for quick sharing

## Main Class Instantiation

When the plugin loads, Qt creates an instance of `ScreenMakerPluginFactory` (via `Q_PLUGIN_METADATA`). The factory provides:

- Plugin metadata (name, description, author, version)
- Factory interface to create `MainScenarioPlugin` instances
- `MainScenarioPlugin` creates `MainScenario` scenario objects on demand

Flow: `ScreenMakerPluginFactory` → `MainScenarioPlugin` → `MainScenario`

## Configuration

ScreenMaker stores the following configuration via `MainScenarioPlugin`:

- `url`: Optional URL parameter (can be empty)
- Storage: Via `IEnvironment::saveConfiguration()`

## Usage / API Highlights

**Plugin Interface (`MainScenarioPlugin`):**

- `getPluginName()` - Returns "ScreenMaker"
- `getClassNames()` - Returns ["ScreenMaker"]
- `create(className)` - Creates `MainScenario` instance
- `getConfiguration()` - Retrieves stored settings
- `setConfiguration(params)` - Updates settings

**Scenario Interface (`MainScenario` extends `GUI::Scenario`):**

- `start(context)` - Displays drawing window with screenshot
- `stop()` - Hides drawing window
- `pause()` / `resume()` - Lifecycle methods
- `signalTriggered(signal)` - Handles user signals
- `canStop()` - Indicates if scenario can be stopped

## Integration

The plugin is loaded via the SDK plugin system:

```cpp
#include <SDK/Plugins/PluginFactory.h>
#include "ScreenMakerFactory.h"

// Plugin is registered via Q_PLUGIN_METADATA in ScreenMakerFactory.h
// Qt automatically discovers and instantiates ScreenMakerPluginFactory
```

## Testing

Tests for ScreenMaker are located in `tests/plugins/NativeScenarios/ScreenMaker/`.

Run tests with:

```bash
ctest -R screen_maker_scenario_test
```

## Platform Support

| Platform | Status | Notes          |
| -------- | ------ | -------------- |
| Windows  | Full   | Supported      |
| Linux    | TODO   | Not yet ported |
| macOS    | TODO   | Not yet ported |

## Migration Notes

**Refactored in v2.0:**

- Consolidated plugin factory: Merged `PluginLibraryDefinition.h` and `PluginFactoryDefinition.cpp` into `ScreenMakerFactory.h/cpp`
- Simplified CMakeLists.txt: Explicit sources, removed unused dependencies
- Qt modules: Now only uses Gui, Network, Concurrent (removed Quick/Qml)
- Removed: App, GUISDK, NetworkTaskManager dependencies

Headers remain in `src/` as this is a plugin-internal API (not used across modules).

## Further Reading

Implementation details: See `src/plugins/NativeScenarios/ScreenMaker/src/`
