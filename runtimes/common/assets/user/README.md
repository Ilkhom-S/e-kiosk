# User Folder

## Purpose

The `user/` folder is a writable directory for user-modifiable runtime data. Unlike `data/`, files in this folder can be created, modified, and deleted by the application or end users at runtime. This folder is copied at build time and serves as a template for the runtime environment.

## Examples

- User preferences and settings
- Custom configurations
- Cached data
- User-generated content
- Device-specific overrides
- Session data
- Stored credentials/tokens

## Directory Structure

```
runtimes/common/assets/user/
├── README.md                    # This file
├── cache/
│   └── .gitkeep
├── preferences/
│   └── default-prefs.json
├── sessions/
│   └── .gitkeep
└── custom/
    └── .gitkeep
```

## Usage in C++ Code

### Reading User Data

```cpp
// Assuming user folder copied to build/bin/user/
QFile userPrefs("user/preferences/default-prefs.json");
if (userPrefs.open(QIODevice::ReadOnly)) {
    QJsonDocument doc = QJsonDocument::fromJson(userPrefs.readAll());
    QJsonObject prefs = doc.object();
    userPrefs.close();
}
```

### Writing User Data

```cpp
// Write/update user preferences
QString userPath = QCoreApplication::applicationDirPath() + "/user/preferences";
QDir().mkpath(userPath);

QFile userFile(userPath + "/custom-settings.json");
if (userFile.open(QIODevice::WriteOnly)) {
    QJsonObject settings;
    settings["theme"] = "light";
    settings["volume"] = 75;

    QJsonDocument doc(settings);
    userFile.write(doc.toJson());
    userFile.close();
}
```

### Creating Runtime Subdirectories

```cpp
// Create user-specific cache directory
QString cachePath = QCoreApplication::applicationDirPath() + "/user/cache";
if (!QDir(cachePath).exists()) {
    QDir().mkpath(cachePath);
}

// Store cache files
QFile cacheFile(cachePath + "/session_" + sessionId + ".dat");
cacheFile.open(QIODevice::WriteOnly);
// ... write cache data ...
cacheFile.close();
```

## Best Practices

### 1. Use `.gitkeep` for Empty Directories

Placeholder files ensure directories are tracked in git even when empty:

```bash
touch runtimes/common/assets/user/cache/.gitkeep
touch runtimes/common/assets/user/sessions/.gitkeep
```

### 2. Include Default Templates

Provide sensible defaults for user-modifiable files:

```json
// runtimes/common/assets/user/preferences/default-prefs.json
{
  "language": "en",
  "theme": "dark",
  "auto_start": false,
  "log_level": "INFO"
}
```

### 3. Separate Concerns

Organize user data by function:

```
user/
├── preferences/      # User settings
├── cache/           # Temporary cached data
├── sessions/        # Active session data
├── licenses/        # License keys/files
└── custom/          # User-uploaded/custom content
```

### 4. Handle File Permissions

On Unix-like systems, set appropriate permissions:

```cpp
#ifdef Q_OS_UNIX
    QFile::setPermissions(fileName,
        QFileDevice::ReadOwner | QFileDevice::WriteOwner |
        QFileDevice::ReadGroup | QFileDevice::ReadOther);
#endif
```

## Build Behavior

At build time, when the `ekiosk` target is built:

1. CMake invokes `ek_copy_assets()`
2. All files in `runtimes/common/assets/user/` are copied to `build/bin/user/`
3. Directory structure is preserved
4. Subsequent builds only copy if files have changed (dependency tracking)

## Adding Runtime Behavior

### Initialize Default User Data

```cpp
// In application startup
void initializeUserFolder() {
    QString userPath = QCoreApplication::applicationDirPath() + "/user";

    // Create subdirectories if needed
    QDir().mkpath(userPath + "/cache");
    QDir().mkpath(userPath + "/sessions");

    // Load or create default preferences
    QFile prefs(userPath + "/preferences/default-prefs.json");
    if (!prefs.exists()) {
        // Create default settings
        QJsonObject defaults;
        defaults["initialized"] = QDateTime::currentDateTime().toString();
        // ... populate defaults ...

        QJsonDocument doc(defaults);
        prefs.open(QIODevice::WriteOnly);
        prefs.write(doc.toJson());
        prefs.close();
    }
}
```

## Migration from Build to Runtime

The `user/` folder provides templates. At runtime:

1. Application loads files from `user/` directory
2. Modifications are written back to the same location
3. New files can be created dynamically
4. On upgrade, old `user/` folder persists (not overwritten)

### Example: Versioned User Data

```cpp
// Load with fallback to defaults
QString userSettingsFile = userPath + "/preferences/app-settings.json";
QJsonObject settings;

if (QFile(userSettingsFile).exists()) {
    // Load existing user settings
    QFile f(userSettingsFile);
    f.open(QIODevice::ReadOnly);
    settings = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
} else {
    // Load defaults from template
    QFile defaults(userPath + "/preferences/default-prefs.json");
    defaults.open(QIODevice::ReadOnly);
    settings = QJsonDocument::fromJson(defaults.readAll()).object();
    defaults.close();
}
```

## Relationship with Data Folder

- **`data/`** → Read-only application defaults (committed to git)
- **`user/`** → User-modifiable runtime data (templates in git, runtime copies are dynamic)

Both folders are copied at build time to `build/bin/{data,user}` for immediate availability.
