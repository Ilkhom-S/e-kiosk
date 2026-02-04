# Updater (Update Application)

## Overview

Updater is an automated command-line application that handles software updates for the E-Kiosk system. It operates silently in the background, downloading and installing updates without user interaction.

**📖 This document is for administrators and operators** - see [Developer Guide](../../apps/Updater/README.md) for technical implementation details.

## Purpose

- **Automated Updates**: Download and install software updates automatically
- **Configuration Management**: Retrieve and deploy configuration files
- **Content Updates**: Download user packages and advertising content
- **Integrity Verification**: Validate system integrity and file consistency
- **Background Operation**: Run updates without disrupting user operations

## Features

### Update Types

The Updater supports different types of update operations:

- **Application Updates**: Full software updates with integrity checking
- **Configuration Updates**: Download and deploy system configuration files
- **User Packages**: Download user-specific content and packages
- **Integrity Checks**: Verify system file integrity

### Operation Modes

- **Command-Driven**: Executed via command-line parameters by the WatchService
- **Silent Operation**: No user interface, runs in background
- **Progress Reporting**: Reports status back to WatchService
- **Error Handling**: Comprehensive error reporting and recovery

## Configuration

Updater uses a combination of configuration files and command-line parameters:

### Configuration Files

The application reads settings from `updater.ini` files located in the working directory:

- **Component Settings**: Define optional components and update behavior
- **Directory Exclusions**: Specify directories to exclude from updates
- **BITS Configuration**: Windows-specific background download settings
- **Validation Rules**: Required files for integrity verification

### Command-Line Parameters

All operations are controlled through command-line arguments passed by WatchService. These parameters can override configuration file settings when needed.

## Operation

### Normal Operation

1. **Command Execution**: Launched by WatchService with specific command parameters
2. **Connection**: Establishes connection to WatchService for status reporting
3. **Download**: Downloads required files from update server
4. **Verification**: Validates file integrity and signatures
5. **Deployment**: Applies updates to system
6. **Completion**: Reports results back to WatchService

### Supported Commands

- **config**: Download and deploy configuration files
- **userpack**: Download user packages and content
- **update**: Perform full application update
- **integrity**: Check system file integrity
- **bits**: Windows BITS-based download management

## Logging

### Log Files

Application logs are located at `logs/{yyyy.mm.dd} Updater.log` in the installation directory, where `{yyyy.mm.dd}` represents the current date in YYYY.MM.DD format.

For example, if installed in `/opt/ekiosk/`, logs would be at `/opt/ekiosk/logs/2026.02.04 Updater.log`.

### Debug Mode

Enable debug logging by setting:

```ini
[Logging]
Level=debug
File=updater_debug.log
```

## Troubleshooting

### Common Issues

#### Cannot Connect to WatchService

**Symptoms:** Updater exits with "NoWatchService" error

**Solutions:**

- Verify WatchService is running
- Check network connectivity
- Verify firewall settings allow local connections

#### Download Failures

**Symptoms:** Network errors during download

**Solutions:**

- Check internet connectivity
- Verify update server URL is accessible
- Check proxy settings if applicable

#### Deployment Errors

**Symptoms:** Files cannot be copied or permissions errors

**Solutions:**

- Verify write permissions to installation directory
- Check available disk space
- Ensure no applications are locking files

### Exit Codes

- **0**: Success
- **1**: Error running from temp directory
- **2**: No WatchService connection
- **3**: Unknown command
- **4**: Second instance running
- **5**: Unknown error
- **6**: Network error
- **7**: Parse error
- **8**: Deploy error
- **9**: Aborted
- **10**: Blocked
- **11**: Integrity check failed
- **12**: Work in progress

## Related Documentation

- **🔧 [Developer Guide](../../apps/Updater/README.md)**: Technical implementation details
- **[WatchService User Guide](watchservice.md)**: Main service that controls updates
- **[Configuration Reference](../configuration-reference.md)**: Update server configuration
