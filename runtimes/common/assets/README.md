# Runtime Assets

Central repository for application runtime assets that are copied to the output directory at build time. This supports both embedded resources (via Qt .qrc files) and modifiable runtime files.

## Hybrid Asset System

EKiosk uses a hybrid approach for asset management:

### 1. Embedded Resources (.qrc)

- **Location**: `apps/EKiosk/src/*.qrc`
- **Usage**: Static resources compiled into the executable
- **Purpose**: Icons, built-in styles, fallback assets
- **Access**: Via Qt resource system (`:/` prefix)
- **Benefit**: Portable, included in binary

### 2. Runtime Assets (Folder-Based)

- **Location**: `runtimes/common/assets/{data,user}/`
- **Usage**: Dynamic files copied at build time
- **Purpose**: Modifiable data, user preferences, runtime defaults
- **Access**: Direct filesystem access from `build/bin/{data,user}/`
- **Benefit**: Updateable without recompilation

## Folder Structure

```
runtimes/common/assets/
├── README.md              # This file
├── data/                  # Read-only application data
│   ├── README.md
│   ├── config/           # Default configurations
│   ├── db/               # Database templates
│   └── locale/           # Localization data
└── user/                 # User-modifiable runtime data
    ├── README.md
    ├── cache/            # Application cache
    ├── preferences/      # User settings
    ├── sessions/         # Session data
    └── custom/           # User content
```

## Asset Categories

### Data Folder - Read-Only Defaults

The `data/` folder contains application-provided, read-only files:

- Configuration templates
- Database schemas
- Localization resources
- Asset catalogs
- Metadata and defaults

See `data/README.md` for details and examples.

### User Folder - Modifiable Runtime Data

The `user/` folder contains writable runtime data:

- User preferences
- Cached data
- Session information
- User-provided content
- Runtime configurations

See `user/README.md` for details and examples.

## Build-Time Copying

### How It Works

1. **Configuration Phase** (cmake)
   - Asset copying is configured but files are NOT copied yet
   - Preserves fast configuration times

2. **Build Phase** (cmake --build)
   - When you build an app (e.g., `cmake --build . --target ekiosk`)
   - All assets from this directory are automatically copied to the output folder
   - File changes are detected, so only modified files are re-copied

3. **Output Location**
   - Assets are copied to: `build/macos-qt6/bin/`
   - Directory structure is preserved:
     ```
     bin/
     ├── data/
     │   ├── config/
     │   ├── db/
     │   └── locale/
     └── user/
         ├── cache/
         ├── preferences/
         └── sessions/
     ```

## Adding Assets

### Adding Data Files

For application defaults:

```bash
mkdir -p runtimes/common/assets/data/config
cat > runtimes/common/assets/data/config/new-defaults.json << 'EOF'
{ "setting": "value" }
EOF
```

### Adding User Templates

For runtime-modifiable data:

```bash
mkdir -p runtimes/common/assets/user/preferences
cat > runtimes/common/assets/user/preferences/template.json << 'EOF'
{ "user_setting": "default_value" }
EOF
```

## Asset Paths in Code

Access copied assets at runtime (relative to executable):

```cpp
// Read-only data files
QString configPath = QCoreApplication::applicationDirPath() + "/data/config/settings.json";

// User-modifiable files
QString userPrefsPath = QCoreApplication::applicationDirPath() + "/user/preferences/user.json";
```

## Notes

- Assets are shared across all applications (placed in runtimes/common/)
- For app-specific assets, create dedicated folders inside each app's source directory
- Use CMake variable `${CMAKE_RUNTIME_OUTPUT_DIRECTORY}` to reference the output location
- **data/** → Commit all files to git
- **user/** → Commit templates; runtime copies are dynamic
- See individual README files in `data/` and `user/` for detailed integration examples
