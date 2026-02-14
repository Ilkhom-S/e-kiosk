# Scheduler Service (Current Implementation)

The Scheduler Service manages automatic task execution on schedule in the
EKiosk system.

## Overview

SchedulerService uses a task type registration approach with INI file
configuration. Each task is a class inheriting the `ITask` interface and
registered in the service through a factory function.

### Key Features

- **INI Configuration**: Task schedules stored in `scheduler.ini`
- **Factory Registration**: Task types registered via `registerTaskType<>()`
- **Timer-based Execution**: Tasks launched by QTimer in separate threads
- **Automatic Retries**: Support for restarting on failure
- **Special Modes**: `startup`, `first_run` for one-time execution

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│              SchedulerService                           │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Task Registry (Factory Map)                     │  │
│  │  "LogArchiver"      → LogArchiver::create()      │  │
│  │  "LogRotate"        → LogRotate::create()        │  │
│  │  "RunUpdater"       → RunUpdater::create()       │  │
│  │  "TimeSync"         → TimeSync::create()         │  │
│  │  "OnOffDisplay"     → OnOffDisplay::create()     │  │
│  │  "UpdateRemoteContent" → UpdateRemoteContent...  │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Task Items (from scheduler.ini)                 │  │
│  │  ┌──────────────┐  ┌──────────────┐             │  │
│  │  │ log_rotate   │  │ archive_logs │             │  │
│  │  │ time=00:01   │  │ time=00:45   │   ...       │  │
│  │  │ QTimer       │  │ QTimer       │             │  │
│  │  └──────────────┘  └──────────────┘             │  │
│  └──────────────────────────────────────────────────┘  │
│                                                          │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Execution Engine                                 │  │
│  │  - Creates task instance from factory            │  │
│  │  - Executes in separate thread                   │  │
│  │  - Monitors completion signals                   │  │
│  │  - Handles retries on failure                    │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## Configuration Files

### 1. `data/scheduler.ini` - Task Configuration

Main configuration file defining all scheduled tasks.

```ini
[log_rotate]
type=LogRotate
time=00:01
params=

[archive_logs]
type=LogArchiver
time=00:45
params=

[time_sync]
type=TimeSync
time=startup
params=

[update_ad]
type=UpdateRemoteContent
period=3600
params=ad

[update_logo]
type=UpdateRemoteContent
period=86400
params=logo

[update_to_last_version]
type=RunUpdater
time=first_run
params=client

[on_off_display]
type=OnOffDisplay
time=00:00
period=60
params=22:00;06:00;saver
```

#### Task Section Parameters

- **`type`** (required) - Task class name (must be registered)
- **`time`** - Launch time in `HH:MM` format or:
  - `startup` - Execute once at application startup
  - `first_run` - Execute once on first run (saved in user config)
- **`period`** - Re-launch interval in seconds (if `time` not specified)
- **`params`** - Parameter string passed to task constructor
- **`repeat_count_if_fail`** - Number of retries on error (default: 0)
- **`time_threshold`** - Random launch delay in seconds (randomization
  +0...N seconds)

### 2. `user/scheduler_config.ini` - Execution State

File for storing information about last task executions (managed automatically).

```ini
[log_rotate]
last_execute=2026.02.14 00:01:05

[archive_logs]
last_execute=2026.02.14 00:45:12

[time_sync]
last_execute=2026.02.14 09:23:45
```

#### Automatic Parameters

- **`last_execute`** - Last execution time in `YYYY.MM.DD HH:MM:SS` format

## Task Type Registration

All available task types are registered in the `SchedulerService` constructor:

```cpp
SchedulerService::SchedulerService(IApplication *aApplication)
    : QObject(nullptr), ILogable("SchedulerService"), mApplication(aApplication) {

    // Register task types
    registerTaskType<LogArchiver>("LogArchiver");
    registerTaskType<LogRotate>("LogRotate");
    registerTaskType<RunUpdater>("RunUpdater");
    registerTaskType<TimeSync>("TimeSync");
    registerTaskType<OnOffDisplay>("OnOffDisplay");
    registerTaskType<UpdateRemoteContent>("UpdateRemoteContent");
}
```

### Registration Mechanism

```cpp
template<typename T>
void registerTaskType(const QString &aTypeName) {
    mTaskFactories[aTypeName] = [](const QString &aName,
                                   const QString &aLogName,
                                   const QString &aParams) -> ITask* {
        return new T(aName, aLogName, aParams);
    };
}
```

When loading a task from an INI file:

1. Read `type` parameter from section
2. Find factory function in `mTaskFactories[type]`
3. Call factory with parameters: `name`, `logName`, `params`
4. Create task instance

## Built-in Tasks

Detailed documentation for all 6 built-in scheduler tasks is available
in [Scheduler Tasks Reference](scheduler-tasks-reference.md).

**Quick Reference Table**:

| Task                    | Class                 | Default Schedule     | Purpose                                 |
| ----------------------- | --------------------- | -------------------- | --------------------------------------- |
| **LogRotate**           | `LogRotate`           | `00:01` daily        | Close and rotate log files              |
| **LogArchiver**         | `LogArchiver`         | `00:45` daily        | Archive old logs to compressed files    |
| **TimeSync**            | `TimeSync`            | `startup`            | Synchronize system time with NTP/HTTP   |
| **RunUpdater**          | `RunUpdater`          | `first_run`          | Register update commands for components |
| **OnOffDisplay**        | `OnOffDisplay`        | `00:00` + 60s period | Manage power saving modes               |
| **UpdateRemoteContent** | `UpdateRemoteContent` | 3600s period         | Update remote service content           |

For access to configuration, execution flow, parameters, return values,
and schedule recommendations, see the
[complete task reference guide](scheduler-tasks-reference.md).

## Creating a New Task

### Step 1: Create Task Class

```cpp
/* @file Example task implementation. */

#pragma once

#include <SDK/PaymentProcessor/Core/ITask.h>
#include <Common/ILogable.h>

class MyCustomTask : public QObject,
                     public SDK::PaymentProcessor::ITask,
                     public ILogable {
    Q_OBJECT

public:
    /// Task constructor
    MyCustomTask(const QString &aName,
                 const QString &aLogName,
                 const QString &aParams);
    virtual ~MyCustomTask();

    /// Execute task
    virtual void execute();

    /// Stop task execution
    virtual bool cancel();

    /// Subscribe to task completion signal
    virtual bool subscribeOnComplete(QObject *aReceiver, const char *aSlot);

signals:
    /// Task completion signal
    void finished(const QString &aName, bool aComplete);

private:
    QString m_Params;  /// Parameters from configuration
    bool m_Canceled;   /// Cancellation flag
};
```

### Step 2: Implement Task

```cpp
/* @file Example task implementation. */

#include "MyCustomTask.h"
#include <Common/BasicApplication.h>
#include <System/IApplication.h>

MyCustomTask::MyCustomTask(const QString &aName,
                           const QString &aLogName,
                           const QString &aParams)
    : ITask(aName, aLogName, aParams)
    , ILogable(aLogName)
    , m_Params(aParams)
    , m_Canceled(false) {

    toLog(LogLevel::Normal, QString("Task initialized with params: %1").arg(m_Params));
}

MyCustomTask::~MyCustomTask() {}

void MyCustomTask::execute() {
    toLog(LogLevel::Normal, "Executing custom task");

    try {
        // Get access to application and services
        auto *app = dynamic_cast<IApplication*>(BasicApplication::getInstance());
        if (!app) {
            throw std::runtime_error("Application not available");
        }

        // Execute task logic
        bool success = performTaskLogic();

        // Signal completion
        emit finished(m_Name, success);

    } catch (const std::exception &e) {
        toLog(LogLevel::Error, QString("Task failed: %1").arg(e.what()));
        emit finished(m_Name, false);
    }
}

bool MyCustomTask::cancel() {
    m_Canceled = true;
    toLog(LogLevel::Warning, "Task cancelled");
    return true;
}

bool MyCustomTask::subscribeOnComplete(QObject *aReceiver, const char *aSlot) {
    return connect(this, SIGNAL(finished(const QString&, bool)),
                   aReceiver, aSlot) != nullptr;
}
```

### Step 3: Register in SchedulerService

Add to `SchedulerService.cpp` constructor:

```cpp
#include "../SchedulerTasks/MyCustomTask.h"

SchedulerService::SchedulerService(IApplication *aApplication)
    : /* ... */ {

    // Existing registrations
    registerTaskType<LogArchiver>("LogArchiver");
    registerTaskType<LogRotate>("LogRotate");
    // ...

    // New task
    registerTaskType<MyCustomTask>("MyCustomTask");
}
```

### Step 4: Add Configuration

In `data/scheduler.ini` file:

```ini
[my_custom_task]
type=MyCustomTask
time=03:00
params=example_parameter
repeat_count_if_fail=3
time_threshold=60
```

## Scheduling Modes

### 1. Time-based Launch

```ini
[daily_task]
type=TaskType
time=02:30         # Every day at 02:30
params=
```

### 2. Periodic Launch

```ini
[periodic_task]
type=TaskType
period=3600        # Every 3600 seconds (1 hour)
params=
```

### 3. Application Startup Launch

```ini
[startup_task]
type=TaskType
time=startup       # Once at startup
params=
```

### 4. First Run Launch

```ini
[first_run_task]
type=TaskType
time=first_run     # Once at first start (saved in user config)
params=
```

### 5. Combined Mode (time + period)

```ini
[combined_task]
type=TaskType
time=00:00         # First launch at 00:00
period=60          # Then every 60 seconds
params=
```

### 6. Launch Time Randomization

```ini
[randomized_task]
type=TaskType
time=03:00
time_threshold=300 # Launch in range 03:00:00 - 03:05:00
params=
```

## Error Handling

### Automatic Retries

```ini
[retry_task]
type=TaskType
time=02:00
repeat_count_if_fail=5  # Retry up to 5 times on error
params=
```

**Mechanism**:

1. Task returns `finished(name, false)` on error
2. SchedulerService receives signal and checks `repeat_count_if_fail`
3. If retries not exhausted, task is launched again after 1 minute
4. Retry counter increments with each attempt

### Error Logging

```cpp
void MyTask::execute() {
    try {
        // Task execution
        performWork();
        emit finished(m_Name, true);

    } catch (const std::exception &e) {
        // Error logging
        toLog(LogLevel::Error, QString("Task execution failed: %1").arg(e.what()));
        emit finished(m_Name, false);  // Signals error
    }
}
```

## Monitoring Task Execution

### Status Check via Logs

All tasks log their actions:

```txt
2026.02.14 00:01:05 [INFO] [SchedulerService] Starting task: log_rotate
2026.02.14 00:01:06 [INFO] [LogRotate] Log rotation completed
2026.02.14 00:01:06 [INFO] [SchedulerService] Task completed: log_rotate (success)

2026.02.14 00:45:12 [INFO] [SchedulerService] Starting task: archive_logs
2026.02.14 00:45:45 [INFO] [LogArchiver] Packed logs for date: 2026.02.13
2026.02.14 00:47:23 [INFO] [LogArchiver] Archive size check: 245 MB / 500 MB
2026.02.14 00:47:23 [INFO] [SchedulerService] Task completed: archive_logs (success)
```

### Last Execution Time Check

File `user/scheduler_config.ini` stores timestamps:

```ini
[log_rotate]
last_execute=2026.02.14 00:01:06

[archive_logs]
last_execute=2026.02.14 00:47:23
```

## Best Practices

### 1. Separation of Concerns

- Each task should have **one clearly defined function**
- Avoid creating "universal" tasks

### 2. Error Handling

```cpp
void MyTask::execute() {
    try {
        // ALWAYS emit finished signal
        bool success = doWork();
        emit finished(m_Name, success);

    } catch (...) {
        // NEVER swallow exceptions without signaling
        toLog(LogLevel::Error, "Task failed");
        emit finished(m_Name, false);
    }
}
```

### 3. Logging

- Log execution start
- Log key steps
- Log result (success/error)
- Use appropriate levels (`Normal`, `Warning`, `Error`)

### 4. Cancellation Handling

```cpp
bool MyTask::cancel() {
    m_Canceled = true;

    // Abort long running operations
    if (m_LongRunningOperation) {
        m_LongRunningOperation->cancel();
    }

    return true;
}

void MyTask::execute() {
    for (int i = 0; i < 1000 && !m_Canceled; ++i) {
        // Check cancellation flag in loops
        processItem(i);
    }

    emit finished(m_Name, !m_Canceled);
}
```

### 5. Execution Time Scheduling

- **Logs**: 00:01 (rotation), 00:45 (archiving) - minimal load
- **Updates**: During off-hours or with low priority
- **Time sync**: At startup or after connection loss
- **Power saving**: Check every minute or less frequently

### 6. Avoid Blocking Operations

```cpp
// BAD: Blocking operation in execute()
void MyTask::execute() {
    QThread::sleep(300);  // Blocks scheduler thread!
    emit finished(m_Name, true);
}

// GOOD: Asynchronous operation
void MyTask::execute() {
    auto *worker = new QThread();
    auto *task = new AsyncWorker();
    task->moveToThread(worker);

    connect(worker, &QThread::started, task, &AsyncWorker::process);
    connect(task, &AsyncWorker::finished, this, [this]() {
        emit finished(m_Name, true);
    });

    worker->start();
}
```

## Performance

### Multithreaded Execution

SchedulerService executes tasks in **separate threads** via `QThread`:

```cpp
// Internal SchedulerService implementation
QThread *thread = new QThread();
task->moveToThread(thread);

connect thread->started() → task->execute()
connect task->finished() → cleanup

thread->start();  // Does not block main thread
```

### Limitations

- **One task of same type at a time**: If task is still running, next launch
  is postponed
- **No prioritization**: Tasks execute in timer trigger order
- **No queue**: Missed tasks are not queued

## Dependencies

SchedulerService depends on:

- **SettingsService** - Reading `TerminalSettings`
- **EventService** - Sending events (e.g., Shutdown)
- **LogService** - Execution logging
- **Core Services** - Access to other services via `IApplication`

## Debugging

### Enable Detailed Logging

In `application.ini`:

```ini
[logs]
SchedulerService=debug
```

### Manual Task Launch

For testing, you can temporarily modify `scheduler.ini`:

```ini
[test_task]
type=MyCustomTask
time=startup      # Will execute immediately on startup
params=test_mode
```

### Simulate First Run

Delete task section from `user/scheduler_config.ini`:

```bash
# Delete [my_task] section from file
# Task with time=first_run will execute again
```

## See Also

- [Settings Service](settings.md) - Schedule configuration
- [Log Service](../modules/logging.md) - Task logging
- [Remote Service](remote.md) - Content updates

## Migration from Old API

If you have code using old callback-based API (from previous documentation),
you need to:

1. Create task class inheriting from `ITask`
2. Move callback logic to `execute()` method
3. Register task type in `SchedulerService`
4. Add configuration to `scheduler.ini`

**Was** (old non-existent API):

```cpp
schedulerService->scheduleTask("my_task", time, [](){ doWork(); });
```

**Became** (current implementation):

```cpp
// 1. Create MyTask.h / MyTask.cpp
class MyTask : public ITask { /*...*/ };

// 2. Register
registerTaskType<MyTask>("MyTask");

// 3. Add to scheduler.ini
[my_task]
type=MyTask
time=02:00
```
