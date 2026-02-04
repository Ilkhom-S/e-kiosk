# Updater

**🔧 Developer Documentation** - Technical details for building and developing the Updater application.

**📖 For administrators and operators**: See [User Guide](../../docs/apps/updater.md) for configuration and operational guidance.

Command-line application for automated software updates and content deployment in the E-Kiosk system.

## Configuration

The Updater supports configuration through multiple sources:

### Configuration Files

Configuration is read from `updater.ini` files with platform-specific sections:

- **Generic/Common** (`updater.ini.in`): Platform-independent settings
- **Platform-specific**: Linux, Windows, macOS specific overrides

#### Configuration Sections

**`[component]`**

- `optional`: Comma-separated list of optional components (e.g., `logo,numcapacity`)

**`[directory]`**

- `ignore`: Comma-separated list of directories to exclude from updates (e.g., `tclib`)

**`[bits]`**

- `ignore`: Disable BITS on Windows (`true`/`false`)
- `priority`: BITS download priority (integer value)

**`[validator]`** (Platform-specific)

- `required_files`: Comma-separated list of required files for integrity checks
  - Linux: `controller,ekiosk,watchdog,libbill_acceptors.so`
  - Windows: `controller.exe,ekiosk.exe,watchdog.exe,bill_acceptors.dll`
  - macOS: `controller.app,ekiosk.app,watchdog.app,libbill_acceptors.dylib`

### Command Line Arguments

Command-line parameters can override configuration file settings:

## Building

### Prerequisites

- Qt 5.15+ or Qt 6.x with Widgets module
- CMake 3.16+
- C++11 compatible compiler
- UpdateEngine library
- WatchServiceClient library

### Build Commands

```bash
# Configure (macOS example)
cmake -S . -B build/macos-qt6 -DCMAKE_PREFIX_PATH=/usr/local/opt/qt6

# Build
cmake --build build/macos-qt6 --target updater

# Run
./build/macos-qt6/bin/updater.app/Contents/MacOS/updater --command=config --server=https://update.example.com
```

### Platform-Specific Notes

- **macOS**: Uses Qt6 from Homebrew (`/usr/local/opt/qt6`)
- **Linux**: Uses system Qt6 packages
- **Windows**: Uses MSVC with Qt6, supports BITS for background downloads

## Running

### Command Line Interface

The Updater combines configuration file settings with command-line parameters. Command-line arguments take precedence over configuration file settings. It supports the following commands:

#### Config Command

Download and deploy configuration files:

```bash
./updater --command=config --server=https://update.example.com --conf=config-id --md5=checksum
```

#### UserPack Command

Download user packages and content:

```bash
./updater --command=userpack --server=https://update.example.com --application=app-id --md5=checksum
```

#### Update Command

Perform full application update:

```bash
./updater --command=update --server=https://update.example.com --workdir=/opt/ekiosk --components=core,ui
```

#### Integrity Command

Check system file integrity:

```bash
./updater --command=integrity --server=https://update.example.com
```

#### BITS Command (Windows)

Manage background downloads using Windows BITS:

```bash
./updater --command=bits
```

## Command Line Options

- `--command`: Command to execute (config/userpack/update/integrity/bits) **[Required]**
- `--server`: Update server URL **[Required]**
- `--workdir`: Working directory for operations
- `--application`: Application/component identifier
- `--conf`: Configuration identifier
- `--md5`: MD5 checksum for verification
- `--components`: Comma-separated component list
- `--point`: Point identifier
- `--proxy`: Proxy server URL
- `--accept-keys`: Accepted signature keys
- `--no-bits`: Disable BITS on Windows
- `--no-restart`: Don't restart after userpack update
- `--destination-subdir`: Subdirectory for deployment

## Configuration Loading

Configuration is loaded from `updater.ini` files in the following order:

1. **Generic template** (`updater.ini.in`): Platform-independent defaults
2. **Platform-specific template**: Linux/Windows/macOS specific settings merged with generic
3. **Runtime configuration**: `updater.ini` in working directory (generated from templates)

**Precedence**: Command-line arguments override configuration file settings.

### Configuration Processing

- Settings are loaded using Qt's `QSettings` class
- Configuration values are applied during command execution
- Platform-specific sections override generic settings
- Command-line parameters can override any configuration value

```mermaid
flowchart TB
    subgraph Updater["Updater Application"]
        subgraph Core["Core Components"]
            UpdaterApp["UpdaterApp<br/>- Command processing<br/>- WatchService communication<br/>- State management"]
            Updater["Updater Engine<br/>- Download management<br/>- File verification<br/>- Deployment logic"]
        end
        subgraph UI["User Interface"]
            SplashScreen["SplashScreen<br/>- Progress display<br/>- User feedback"]
        end
        subgraph Communication["Communication"]
            WatchServiceClient["WatchService Client<br/>- Status reporting<br/>- Command coordination"]
            ReportBuilder["ReportBuilder<br/>- Progress tracking<br/>- Result reporting"]
        end
        UpdaterApp --> Updater
        UpdaterApp --> SplashScreen
        UpdaterApp --> WatchServiceClient
        UpdaterApp --> ReportBuilder
    end

    Updater --> UpdateServer["Update Server<br/>- File downloads<br/>- Version info"]
    Updater --> WatchService["WatchService<br/>- Command source<br/>- Status updates"]
```

## Key Files

- `main.cpp`: Application entry point
- `UpdaterApp.h/cpp`: Main application logic and state management
- `SplashScreen.h/cpp`: Progress display and user interface
- `SplashScreen.ui`: Qt Designer UI definition

## Dependencies

- `UpdateEngine` - Core update functionality
- `WatchServiceClient` - Communication with WatchService
- `ReportBuilder` - Progress reporting
- Qt Widgets module - UI components
- SingleApplication - Prevent multiple instances

## Implementation Notes

- **Single Instance**: Uses SingleApplication to prevent multiple updater instances
- **State Machine**: Implements Download/Deploy/Finish states for update process
- **WatchService Integration**: Communicates with WatchService for coordination and status
- **BITS Support**: On Windows, uses Background Intelligent Transfer Service for downloads
- **Self-Updating**: Can restart from temporary directory to update itself
- **Error Handling**: Comprehensive error codes and reporting
- **Splash Screen**: Shows progress during long operations

## Update Process Flow

1. **Initialization**: Parse command-line arguments, establish WatchService connection
2. **Download Phase**: Download files from update server with progress reporting
3. **Verification**: Validate file integrity using MD5 checksums and signatures
4. **Deployment**: Extract/update files in target directories
5. **Completion**: Report results back to WatchService, exit with appropriate code

## Platform Support

| Platform | Status | Notes                  |
| -------- | ------ | ---------------------- |
| Windows  | ✅     | Full support with BITS |
| Linux    | ✅     | Full support           |
| macOS    | ✅     | Full support           |

## Error Codes

The application exits with specific codes to indicate operation results:

- `0`: Success
- `1`: Error running from temporary directory
- `2`: Cannot connect to WatchService
- `3`: Unknown command specified
- `4`: Another instance already running
- `5`: Unknown error occurred
- `6`: Network error during download
- `7`: Parse error in server response
- `8`: Deployment error (file copy/extraction)
- `9`: Operation aborted by WatchService
- `10`: Update blocked by server
- `11`: Integrity check failed
- `12`: Work in progress (asynchronous operation)

## Related Documentation

- **📖 [User Guide](../../docs/apps/updater.md)**: Operational guidance and troubleshooting
- **[WatchService Documentation](../WatchService/README.md)**: Service that controls updates
- **[UpdateEngine Documentation](../../include/UpdateEngine/)**: Core update library
- **[Configuration Reference](../../docs/configuration-reference.md)**: Update server settings
