# Scheduler Tasks Reference

Quick reference guide for the 6 core scheduler tasks in EKiosk.

## Task Overview

| Task                    | Type                  | Default Schedule | Purpose                                       |
| ----------------------- | --------------------- | ---------------- | --------------------------------------------- |
| **LogRotate**           | `LogRotate`           | `00:01` daily    | Close and rotate all log files                |
| **LogArchiver**         | `LogArchiver`         | `00:45` daily    | Archive old logs to compressed files          |
| **TimeSync**            | `TimeSync`            | `startup`        | Synchronize system time with NTP/HTTP servers |
| **RunUpdater**          | `RunUpdater`          | `first_run`      | Register update commands for components       |
| **OnOffDisplay**        | `OnOffDisplay`        | Optional (300s)  | Manage power saving modes (if configured)     |
| **UpdateRemoteContent** | `UpdateRemoteContent` | 3600s period     | Update remote service content                 |

---

## 1. LogRotate

**Files**:

- [LogRotate.h](../../apps/EKiosk/src/SchedulerTasks/LogRotate.h)
- [LogRotate.cpp](../../apps/EKiosk/src/SchedulerTasks/LogRotate.cpp)

### Purpose

Closes all active log files and rotates them (renames with current date).

### Configuration

```ini
[log_rotate]
type=LogRotate
time=00:01
params=
```

### Execution Flow

1. Get terminal service client
2. Execute `close_logs` command to close all log handles
3. Call `ILog::logRotateAll()` to rename files: `app.log` →
   `app.2026.02.14.log`

### Schedule Recommendation

- **When**: Daily at 00:01 (low traffic time)
- **Why**: Prepares logs for archiving at 00:45

### Return Value

- Always returns `true` (cannot fail)

---

## 2. LogArchiver

**Files**:

- [LogArchiver.h](../../apps/EKiosk/src/SchedulerTasks/LogArchiver.h)
- [LogArchiver.cpp](../../apps/EKiosk/src/SchedulerTasks/LogArchiver.cpp)

### Purpose

Archives old log files into compressed 7z archives and manages storage limits.

### Configuration

```ini
[archive_logs]
type=LogArchiver
time=00:45
params=
```

### Execution Flow

1. Read `TerminalSettings::getLogsMaxSize()` (in MB)
2. Scan `logs/` directory for dated log files
3. Group files by date (format: `YYYY.MM.DD`)
4. Pack each date group into `logs_YYYY.MM.DD.7z`
5. Delete source log files after successful packing
6. Remove oldest archives if total size exceeds limit

### Archive Settings

- **Format**: 7-Zip (`.7z`)
- **Compression level**: 7 (high)
- **Timeout**: 60 minutes
- **Recursive**: Yes

### Schedule Recommendation

- **When**: Daily at 00:45 (after LogRotate at 00:01)
- **Why**: Ensures logs are closed before archiving

### Return Value

- `true`: Archives created successfully
- `false`: Error (invalid config, directory missing, packing failed)

### Retry Strategy

```ini
repeat_count_if_fail=3  # Retry up to 3 times
```

---

## 3. TimeSync

**Files**:

- [TimeSync.h](../../apps/EKiosk/src/SchedulerTasks/TimeSync.h)
- [TimeSync.cpp](../../apps/EKiosk/src/SchedulerTasks/TimeSync.cpp)

### Purpose

Synchronizes system time with NTP/HTTP time servers.

### Configuration

```ini
[time_sync]
type=TimeSync
time=startup
params=
```

### Execution Flow

1. Read server list from `TerminalSettings::getTimeSyncHosts()`
2. Send parallel requests to ALL servers (NTP + HTTP)
3. Use first valid response received
4. Calculate local time offset
5. If offset > 5 minutes (300s), adjust system time
6. Rotate logs after time adjustment (to prevent timestamp confusion)

### Supported Protocols

#### NTP (Accurate)

- **URL format**: `ntp://pool.ntp.org`
- **Library**: qntp (NtpClient)
- **Timeout**: 10 seconds
- **Accuracy**: ±50ms

#### HTTP (Approximate)

- **URL format**: `http://worldtimeapi.org/api/ip` or `https://google.com`
- **Method**: GET request, read Date header
- **Timeout**: 30 seconds
- **Accuracy**: ±1-2 seconds

### Parameters

- **Desync limit**: 300 seconds (5 minutes)
- **Action**: If exceeded, call `ISysUtils::setSystem_Time()`

### Schedule Recommendation

- **When**: At application startup (`time=startup`)
- **Why**: Critical for payment system - timestamps must be accurate

### Return Value

- `true`: Time synchronized or offset within acceptable range
- `false`: All servers failed or timed out

### Error Handling

- Tries ALL servers in parallel
- Returns success on FIRST valid response
- Logs warnings if no servers respond

---

## 4. RunUpdater

**Files**:

- [RunUpdater.h](../../apps/EKiosk/src/SchedulerTasks/RunUpdater.h)
- [RunUpdater.cpp](../../apps/EKiosk/src/SchedulerTasks/RunUpdater.cpp)

### Purpose

Registers update commands with RemoteService to check for component updates.

### Configuration

```ini
[update_to_last_version]
type=RunUpdater
time=first_run
params=client
```

### Parameters Format

```text
<component_list>
```

**Examples**:

- `client` - Update only client component
- `client,server` - Update multiple components
- Empty string - Update all components

### Execution Flow

1. Read updater URLs from `TerminalSettings::getUpdaterUrls()` (requires 2 URLs)
2. Register command: `IRemoteService::registerUpdateCommand(Update, url1,
url2, params)`
3. RemoteService checks for updates on monitored URLs
4. If update available, downloads and applies it

### Schedule Recommendation

- **When**: First run (`time=first_run`)
- **Why**: Ensures application is up-to-date after initial installation

### Alternative Uses

```ini
# Check for updates daily
[daily_update_check]
type=RunUpdater
time=03:00
params=client
```

### Return Value

- `true`: Update command registered successfully
- `false`: Invalid configuration (missing URLs)

---

## 5. OnOffDisplay

**Status**: Optional (only created if `energySave` is configured in terminal settings)

**Files**:

- [OnOffDisplay.h](../../apps/EKiosk/src/SchedulerTasks/OnOffDisplay.h)
- [OnOffDisplay.cpp](../../apps/EKiosk/src/SchedulerTasks/OnOffDisplay.cpp)

### Purpose

Manages power saving by turning display on/off or activating screensaver.

### Configuration

This task is created dynamically by `SchedulerService::setupDisplayOnOff()` based on
terminal settings. Manual configuration in `scheduler.ini` is not required.

**Auto-generated parameters** (from `TerminalSettings::energySave()`):

```ini
[DisplayOnOff]
type=OnOffDisplay
period=300
params=<from_energySave_setting>
```

### Parameters Format

```text
<start_time>;<end_time>;<action_type>
```

**Components**:

1. **`<start_time>`** (HH:MM) - Power saving starts
2. **`<end_time>`** (HH:MM) - Power saving ends
3. **`<action_type>`** (optional):
   - Empty / `standby` → Turn off display (`ISysUtils::displayOn(false)`)
   - `saver` → Run screensaver (`ISysUtils::runScreenSaver()`)
   - `shutdown` → Shutdown system (send `Shutdown` event)

### Examples

#### Standby mode (turn off display)

```ini
params=22:00;06:00
# or explicitly
params=22:00;06:00;standby
```

#### Screensaver mode

```ini
params=23:00;07:00;saver
```

#### System shutdown

```ini
params=01:00;05:00;shutdown
```

### Execution Flow

1. Parse parameters to get time range and action type
2. Check if current time is within range (handles midnight crossover)
3. Check `GUIService::canShutdown()` permission
4. If in range and allowed:
   - **Standby**: Turn off display
   - **Saver**: Launch screensaver
   - **Shutdown**: Send shutdown event
5. If outside range: Turn on display

### Time Range Handling

#### Normal range (no midnight crossover)

```text
start=08:00, end=18:00
Active: 08:00 → 18:00
```

#### Overnight range (midnight crossover)

```text
start=22:00, end=06:00
Active: 22:00 → 23:59 + 00:00 → 06:00
```

### Schedule Recommendation

- **Enabled if**: `TerminalSettings::energySave()` is configured
- **Period**: 300 seconds (5 minutes) - automatic periodic checks
- **Why**: Periodic execution ensures timely activation/deactivation of display
  modes

### Return Value

- `true`: Action executed successfully
- `false`: Permission denied by GUIService

---

## 6. UpdateRemoteContent

**Files**:

- [UpdateRemoteContent.h](../../apps/EKiosk/src/SchedulerTasks/UpdateRemoteContent.h)
- [UpdateRemoteContent.cpp](../../apps/EKiosk/src/SchedulerTasks/UpdateRemoteContent.cpp)

### Purpose

Updates content from remote services (ads, logos, provider data).

### Configuration

```ini
[update_ad]
type=UpdateRemoteContent
period=3600
params=ad

[update_logo]
type=UpdateRemoteContent
period=86400
params=logo
```

### Execution Flow

1. Call `IRemoteService::updateContent()`
2. RemoteService downloads latest content from configured servers
3. Updates local cache
4. Content becomes available for display

### Parameters

- `params` - Content type identifier (currently not used in implementation,
  but available for future extension)

### Schedule Recommendations

#### Advertisements (frequent updates)

```ini
[update_ad]
type=UpdateRemoteContent
period=3600    # Every hour
params=ad
```

#### Logos/branding (infrequent updates)

```ini
[update_logo]
type=UpdateRemoteContent
period=86400   # Once per day
params=logo
```

#### Service provider data (moderate updates)

```ini
[update_services]
type=UpdateRemoteContent
period=7200    # Every 2 hours
params=services
```

### Return Value

- Always returns `true` (actual update success is handled by RemoteService)

---

## Complete Example Configuration

### `data/scheduler.ini`

```ini
# ===== Log Management =====
[log_rotate]
type=LogRotate
time=00:01
params=

[archive_logs]
type=LogArchiver
time=00:45
params=
repeat_count_if_fail=3

# ===== System Maintenance =====
[time_sync]
type=TimeSync
time=startup
params=

[update_to_last_version]
type=RunUpdater
time=first_run
params=client

# ===== Power Management =====
[on_off_display]
type=OnOffDisplay
time=00:00
period=60
params=22:00;06:00;saver

# ===== Content Updates =====
[update_ad]
type=UpdateRemoteContent
period=3600
params=ad

[update_logo]
type=UpdateRemoteContent
period=86400
params=logo

[update_services]
type=UpdateRemoteContent
period=7200
params=services
```

---

## Task Dependencies

```text
startup:
  └─> TimeSync (synchronize time first)

daily 00:01:
  └─> LogRotate (close and rotate logs)
      └─> 00:45: LogArchiver (archive rotated logs)

periodic (every 60s):
  └─> OnOffDisplay (check power saving status)

periodic (every 3600s):
  └─> UpdateRemoteContent [ad]

periodic (every 86400s):
  └─> UpdateRemoteContent [logo]
```

## Troubleshooting

### Task not executing

**Check**:

1. Task type registered in `SchedulerService.cpp`?
2. Configuration syntax correct in `scheduler.ini`?
3. Time format valid (`HH:MM` or `startup`/`first_run`)?
4. Check logs for errors: `SchedulerService`

### Task failing repeatedly

**Check**:

1. Review task logs for specific error messages
2. Verify required services are available (check dependencies)
3. Check permissions (file access, system time adjustment, etc.)
4. Increase `repeat_count_if_fail` if temporary failures expected

### Time-based tasks not running at expected time

**Check**:

1. System time correct? (Use TimeSync to verify)
2. Time zone configured correctly?
3. Check `user/scheduler_config.ini` for `last_execute` timestamp
4. Use `time_threshold` if exact timing not critical

### Task running but no visible effect

**Check**:

1. Task parameters format correct?
2. Required configuration present (e.g., updater URLs, time sync hosts)?
3. GUIService permissions (for OnOffDisplay)?
4. Enable debug logging to trace execution

---

## See Also

- [Scheduler Service Documentation](scheduler.md) - Full service documentation
- [Creating Custom Tasks](scheduler.md#creating-custom-tasks) - How to add
  new task types
- [SchedulerService.cpp](../../apps/EKiosk/src/Services/SchedulerService.cpp)
  - Task registration
