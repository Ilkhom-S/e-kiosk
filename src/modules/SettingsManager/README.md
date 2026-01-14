# SettingsManager

Configuration management module.

## Purpose

Manages all application settings:

- INI file configuration
- Registry access (Windows)
- Environment variables
- Runtime settings cache

## Usage

```cpp
#include "SettingsManager/SettingsManager.h"

SettingsManager* sm = SettingsManager::instance();

// Read settings
QString url = sm->value("Network/ServerUrl").toString();
int timeout = sm->value("Network/Timeout", 30000).toInt();

// Write settings
sm->setValue("Terminal/Name", "Main Terminal");
sm->sync();  // Save to disk

// Watch for changes
connect(sm, &SettingsManager::valueChanged,
        this, &MyClass::onSettingChanged);
```

## Configuration Files

### Primary Config

```ini
; config/terminal.ini

[Terminal]
Id=TERM001
Name=Main Terminal
Location=Entrance

[Network]
ServerUrl=https://payment.example.com
Timeout=30000
RetryCount=3

[Graphics]
Backend=native_backend
FullScreen=true

[Logging]
Level=info
MaxFileSize=10485760
MaxFiles=5
```

### Settings Hierarchy

```text
1. Environment variables (highest priority)
2. Command line arguments
3. User config file
4. System config file
5. Default values (lowest priority)
```

## Key Files

| File                   | Purpose            |
| ---------------------- | ------------------ |
| `SettingsManager.h`    | Main class         |
| `ISettingsProvider.h`  | Provider interface |
| `IniProvider.cpp`      | INI file support   |
| `RegistryProvider.cpp` | Windows registry   |

## API Reference

```cpp
class SettingsManager {
    // Read
    QVariant value(const QString& key,
                   const QVariant& defaultValue = QVariant());

    // Write
    void setValue(const QString& key, const QVariant& value);

    // Check
    bool contains(const QString& key);
    QStringList allKeys();
    QStringList childKeys(const QString& group);

    // Persist
    void sync();

    // Groups
    void beginGroup(const QString& group);
    void endGroup();

signals:
    void valueChanged(const QString& key, const QVariant& value);
};
```

## Dependencies

- `Log` module
- Qt Core module

## Platform Support

| Platform | INI Files | Registry | Env Vars |
| -------- | --------- | -------- | -------- |
| Windows  | ✅        | ✅       | ✅       |
| Linux    | ✅        | N/A      | ✅       |
| macOS    | ✅        | N/A      | ✅       |
