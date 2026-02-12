# Data Folder

## Purpose

The `data/` folder contains read-only application data files that are copied at build time to the runtime output directory. These files support core application functionality and are typically not modified by end users.

## Examples

- Configuration templates
- Default settings files
- Database schemas
- Localization resources
- Asset catalogs
- Default device configurations

## Directory Structure

```
runtimes/common/assets/data/
├── README.md                    # This file
├── config/
│   └── default-settings.json
├── db/
│   └── schema.sql
└── locale/
    └── strings.json
```

## Usage in C++ Code

### Loading Data Files

```cpp
// Assuming data copied to build/bin/data/
QFile configFile("data/config/default-settings.json");
if (!configFile.open(QIODevice::ReadOnly)) {
    qWarning() << "Could not open config file";
    return;
}

QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
QJsonObject settings = doc.object();
configFile.close();
```

### Using Qt Resource Paths

If you need path-independent access:

```cpp
// On Linux/macOS
QString dataPath = QCoreApplication::applicationDirPath() + "/data";
QString configFile = dataPath + "/config/default-settings.json";
```

## Build Behavior

At build time, when the `ekiosk` target is built:

1. CMake invokes `ek_copy_assets()`
2. All files in `runtimes/common/assets/data/` are copied to `build/bin/data/`
3. Directory structure is preserved
4. File permissions and attributes are maintained

## Adding New Data Files

1. Create files in the appropriate subdirectory under `data/`
2. Commit changes to version control
3. Rebuild the application - `cmake --build build --target ekiosk`
4. Data files will be automatically copied to output directory

### Example: Add Default Settings

```bash
# Create new settings file
mkdir -p runtimes/common/assets/data/config
cat > runtimes/common/assets/data/config/my-settings.json << 'EOF'
{
    "app_version": "1.0.0",
    "theme": "dark",
    "language": "en"
}
EOF

# Rebuild
cmake --build build --target ekiosk
```

## Notes

- **Read-Only**: Data files are intended to be immutable during runtime
- **No .gitignore**: Track all data files in version control
- **Size Consideration**: Keep data files reasonably sized (large assets should use separate distribution)
- **Encoding**: Use UTF-8 for text files, JSON for configuration data

## Integration with User Folder

- `data/` → Application provided, read-only defaults
- `user/` → User-modifiable runtime data (see `user/README.md`)
