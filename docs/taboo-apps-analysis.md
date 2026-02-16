# Taboo Apps (Forbidden Processes) Analysis

## Overview

The **taboo apps** feature allows the watchdog to monitor and terminate specific processes that should not run while the kiosk is active. This is important for:

- **Security**: Prevent unauthorized tools (ProcessHacker, debuggers, remote access)
- **Performance**: Kill resource-heavy apps (Chrome, Firefox, browsers)
- **Kiosk Mode Integrity**: Prevent window managers/explorers that break fullscreen
- **Attack Prevention**: Block known hacking tools (VCOM, etc.)

## Current Status

**DISABLED** - Code wrapped in `#if 0 // #40592` since migration

## Why It Was Disabled

### Critical Issue #1: Blocking Calls in Main Thread

The `checkForbiddenModules()` method runs in a **timer callback** (every 60 seconds by default), which executes in the main thread. However, it contains **three blocking operations**:

#### 1. ProcessEnumerator Constructor (All Platforms)

```cpp
ProcessEnumerator processes;  // Calls enumerate() inside constructor
```

**Problem**: `enumerate()` scans **all running processes** in the system:

- **macOS**: Calls `proc_listallpids()` twice + `proc_pidpath()` for each PID
- **Linux**: Reads entire `/proc` directory + `readlink()` for each process
- **Windows**: Calls `EnumProcesses()` + `GetModuleFileNameEx()` for each

**Impact**: On systems with 100+ processes, can take **10-50ms** or more. Blocks event loop during this time.

#### 2. waitpid() System Call (macOS/Linux)

```cpp
// From mac_processenumerator.cpp killInternal():
waitpid(static_cast<pid_t>(aPid), &status, WNOHANG);
usleep(100000);  // ⚠️ BLOCKS FOR 100ms!
waitpid(static_cast<pid_t>(aPid), &status, WNOHANG);
// ...
waitpid(static_cast<pid_t>(aPid), &status, 0);  // ⚠️ CAN BLOCK INDEFINITELY!
```

**Problem**:

- `usleep(100000)` **always blocks for 100ms**
- `waitpid(..., 0)` with no flags **blocks until process exits**
- Called multiple times if process doesn't respond to SIGTERM

**Impact**: Each kill can block **100ms to several seconds**. If killing multiple processes, total block time multiplies.

#### 3. WaitForSingleObject() (Windows)

```cpp
// From win_processenumerator.cpp waitForClose():
result = WaitForSingleObject(process, aTimeout) == WAIT_OBJECT_0;
// aTimeout = CProcessEnumerator::WaitExplorerExit = 3000ms
```

**Problem**: Blocks for up to **3 seconds** waiting for process to exit.

**Impact**: Killing `explorer.exe` alone can freeze watchdog UI for 3 seconds.

### Critical Issue #2: Timer Context

```cpp
m_CheckForbiddenTimer->start(m_CheckForbiddenTimeout);  // Default 60000ms = 60 seconds
connect(m_CheckForbiddenTimer.data(), &QTimer::timeout, this, &WatchService::checkForbiddenModules);
```

- Timer runs in **main thread** (event loop)
- Any blocking in callback **freezes entire application**
- Same issue we just fixed with `waitForStarted()` and `waitForFinished()`

### Issue #3: Missing Restart Logic

```cpp
void WatchService::startTerminatedModules() {
    m_CheckMemoryTimer->stop();  // ⚠️ This timer doesn't exist anymore!
    // ...
}
```

**Problem**: Code references `m_CheckMemoryTimer` which is not defined in current WatchService. This was likely lost during refactoring.

**Question**: Do we actually want to restart terminated processes when kiosk shuts down? Original intent unclear.

## Architecture Analysis

### Configuration Loading

```cpp
// From WatchService.cpp line 340:
QSettings userSettings(..."/user/user.ini"...);
if (userSettings.value("watchdog/taboo_enabled").toString() == "true") {
    settings.beginGroup("taboo");
    m_ForbiddenModules = settings.value("applications", "").toStringList();
    m_CheckForbiddenTimeout = settings.value("check_timeout", 60000).toInt();
    settings.endGroup();
}
```

**Design**: Two-level configuration

1. `user/user.ini`: Enable/disable feature (`watchdog/taboo_enabled=true`)
2. `watchdog.ini`: List of forbidden apps and check interval (`[taboo]` section)

**Current Config Files** (already exist):

- `watchdog.ini.win.in`: `explorer.exe,taskmgr.exe,processhacker.exe,iexplore.exe,browser.exe,chrome.exe,firefox.exe,opera.exe`
- `watchdog.ini.mac.in`: `Activity Monitor,Terminal,Safari,Firefox,Chrome,Opera`
- `watchdog.ini.linux.in`: `gnome-terminal,konsole,xterm,system-monitor`

### Process Matching Logic

```cpp
foreach(auto moduleName, m_ForbiddenModules) {
    it = std::find_if(it, processes.end(),
        [&moduleName](const ProcessEnumerator::ProcessInfo & aPInfo) -> bool {
            return moduleName.size() && aPInfo.path.contains(moduleName, Qt::CaseInsensitive);
        });
}
```

**Matching**: Case-insensitive **substring** match on full process path

- ✅ Flexible: `chrome` matches `/usr/bin/google-chrome` or `C:\Program Files\Google\Chrome\chrome.exe`
- ⚠️ Risk: `explorer` would also match `file-explorer-utility.exe` (false positive)

### Kill Strategy

1. **Find** all matching processes
2. **Kill** each one using `ProcessEnumerator::kill()`
3. **Store** killed process paths in `m_TerminatedModules`
4. **Log** results

## Platform-Specific Suggestions

### Windows - Recommended Forbidden List

```ini
; Critical security/performance threats for Windows kiosk
applications=explorer.exe,taskmgr.exe,processhacker.exe,procexp.exe,procexp64.exe,iexplore.exe,browser.exe,chrome.exe,firefox.exe,opera.exe,msedge.exe,cmd.exe,powershell.exe,regedit.exe,mmc.exe,control.exe,teamviewer.exe,anydesk.exe,vnc.exe
check_timeout=30000
```

**Rationale**:

- `explorer.exe`: Windows shell, provides taskbar/desktop (breaks fullscreen)
- `taskmgr.exe`: Task Manager (allows killing kiosk)
- `processhacker.exe`, `procexp.exe`: Process inspection tools
- Browsers: Heavy resources, not needed in kiosk
- `cmd.exe`, `powershell.exe`: Command shells (security risk)
- `regedit.exe`: Registry editor (security risk)
- Remote access tools: `teamviewer.exe`, `anydesk.exe`, `vnc.exe`

**Note**: Be careful with `explorer.exe` - killing it on Windows 7/8/10 can cause issues. Consider whitelist mode instead of killing on sight.

### macOS - Recommended Forbidden List

```ini
; Critical security/performance threats for macOS kiosk
applications=Activity Monitor.app,Terminal.app,Console.app,Safari.app,Firefox.app,Chrome.app,Opera.app,iTerm.app,System Preferences.app,System Settings.app,Remote Desktop.app,Screen Sharing.app
check_timeout=30000
```

**Rationale**:

- `Activity Monitor`: macOS task manager
- `Terminal`, `iTerm`, `Console`: Command-line access
- Browsers: Performance drain
- `System Preferences/Settings`: Allows changing kiosk config
- Remote access: `Remote Desktop`, `Screen Sharing`

**Note**: macOS app names include `.app` extension. Process paths are like `/Applications/Safari.app/Contents/MacOS/Safari`, so matching `Safari` will work.

### Linux - Recommended Forbidden List

```ini
; Critical security/performance threats for Linux kiosk
applications=gnome-terminal,konsole,xterm,rxvt,system-monitor,gnome-system-monitor,ksysguard,htop,firefox,chrome,chromium,opera,nautilus,dolphin,thunar,gparted,synaptic,ssh,vncviewer
check_timeout=30000
```

**Rationale**:

- Terminal emulators: `gnome-terminal`, `konsole`, `xterm`, `rxvt`
- System monitors: `gnome-system-monitor`, `ksysguard`, `htop`
- Browsers: `firefox`, `chrome`, `chromium`, `opera`
- File managers: `nautilus`, `dolphin`, `thunar`
- System tools: `gparted`, `synaptic`
- Remote access: `ssh`, `vncviewer`

## Recommendations

### ✅ What Should Be Implemented

1. **Async Process Enumeration**
   - Move `ProcessEnumerator::enumerate()` to background thread
   - Use `QFuture`/`QtConcurrent::run()` or `QThread`
   - Signal when enumeration complete, process kills in batches

2. **Non-Blocking Kill Operations**
   - Don't wait for processes to exit in timer callback
   - Send kill signal, continue immediately
   - Check process death status on next timer cycle

3. **Timeout Protection**
   - Maximum time budget per timer cycle (e.g., 50ms)
   - If budget exceeded, defer remaining kills to next cycle
   - Prevents any single timer call from blocking too long

4. **Smarter Matching**
   - Exact basename match OR path contains substring
   - Reduces false positives: `chrome.exe` matches only `chrome.exe`, not `chrome-helper.exe`

5. **Whitelist Option**
   - Config option: `kill_on_sight=false` → log warnings instead of killing
   - Useful for debugging or less aggressive enforcement

6. **Restart Logic Clarification**
   - Remove `startTerminatedModules()` if not needed
   - OR: Implement properly with watchdog shutdown signal

### ⚠️ What Should NOT Be Done

❌ **Don't kill critical system processes**

- Windows: Don't kill `csrss.exe`, `winlogon.exe`, `services.exe` (system crash)
- macOS: Don't kill `launchd`, `WindowServer`, `kernel_task` (system crash)
- Linux: Don't kill `systemd`, `init`, `Xorg` (system crash)

❌ **Don't use blocking waits in timer callbacks**

- No `sleep()`, `usleep()`, `QThread::msleep()`
- No `waitpid(..., 0)` without `WNOHANG`
- No `WaitForSingleObject()` with timeouts > 0

❌ **Don't enumerate processes synchronously**

- Scanning 100+ processes can take 10-50ms
- Use background thread or async approach

## Proposed Implementation Plan

### Phase 1: Fix Blocking Issues ✅ Critical

1. **Background Thread for Enumeration**

   ```cpp
   void WatchService::checkForbiddenModules() {
       // Launch async enumeration
       QFuture<QMap<PID, ProcessInfo>> future =
           QtConcurrent::run([]() {
               ProcessEnumerator pe;
               return pe.getProcesses();  // New getter, no iterator
           });

       // When done, check forbidden list
       QFutureWatcher<...> *watcher = new QFutureWatcher<...>(this);
       connect(watcher, &QFutureWatcher::finished, this, [this, watcher]() {
           processEnumerationComplete(watcher->result());
           watcher->deleteLater();
       });
       watcher->setFuture(future);
   }
   ```

2. **Non-Blocking Kill**

   ```cpp
   void WatchService::killForbiddenProcess(PID pid, QString name) {
       // Just send kill signal, don't wait
       #ifdef Q_OS_WIN
           HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
           if (h) {
               TerminateProcess(h, 1);
               CloseHandle(h);
           }
       #else
           ::kill(pid, SIGKILL);  // No wait
       #endif

       toLog(LogLevel::Normal, QString("Killed forbidden process: %1 (PID %2)").arg(name).arg(pid));
   }
   ```

3. **Timeout Budget**

   ```cpp
   void WatchService::processEnumerationComplete(QMap<PID, ProcessInfo> processes) {
       QElapsedTimer timer;
       timer.start();
       const qint64 maxBudgetMs = 50;  // Maximum 50ms in this handler

       foreach(auto moduleName, m_ForbiddenModules) {
           if (timer.elapsed() > maxBudgetMs) {
               // Budget exceeded, defer to next cycle
               break;
           }

           // Find and kill matching processes...
       }
   }
   ```

### Phase 2: Enhanced Configuration 📝 Nice to Have

1. **Exact Match Option**

   ```ini
   [taboo]
   applications=chrome.exe,firefox.exe
   match_mode=exact_basename  ; Options: exact_basename, path_contains, exact_path
   check_timeout=30000
   ```

2. **Kill Strategy**

   ```ini
   [taboo]
   applications=chrome.exe
   kill_mode=immediate  ; Options: immediate, graceful (SIGTERM first), log_only
   check_timeout=30000
   ```

3. **Per-Process Config**

   ```ini
   [taboo_chrome]
   executable=chrome.exe
   match_mode=exact_basename
   kill_mode=immediate

   [taboo_explorer]
   executable=explorer.exe
   match_mode=exact_basename
   kill_mode=log_only  ; Don't actually kill, just warn
   ```

### Phase 3: Remove Restart Logic 🗑️ Cleanup

Remove `startTerminatedModules()` and `m_TerminatedModules` - not needed:

- Kiosk shutting down anyway, no point restarting killed processes
- Adds complexity without clear benefit
- If user started Chrome manually, probably wanted it to stay dead

## Questions for You

Before I implement this, please confirm:

1. **Restart Logic**: Should we keep `startTerminatedModules()` to restart killed processes when watchdog shuts down? Or remove it?
   - My suggestion: **Remove it** (unnecessary complexity)

2. **Kill Strategy**: Should we kill immediately (SIGKILL) or try graceful first (SIGTERM)?
   - My suggestion: **SIGKILL immediately** for security (no chance for process to ignore)
3. **Explorer.exe on Windows**: Should we kill it? It's the Windows shell, killing might cause issues.
   - My suggestion: **Log-only mode for explorer.exe** by default, allow override in config

4. **Check Interval**: 60 seconds seems long. Should we make it shorter (e.g., 10-30 seconds)?
   - My suggestion: **30 seconds** for responsive monitoring without overhead

5. **Background Thread**: Use `QtConcurrent` or dedicated `QThread`?
   - My suggestion: **QtConcurrent::run()** (simpler, built-in thread pool)

6. **Platform Priority**: Which platform should we test first?
   - My suggestion: **Windows** (since you mentioned the list: explorer.exe, taskmgr.exe, etc.)

7. **User Enable**: Should this be enabled by default, or require `user/user.ini` opt-in?
   - My suggestion: **Disabled by default** (opt-in via `watchdog/taboo_enabled=true`)

## Testing Strategy

1. **Unit Tests**:
   - ProcessEnumerator finds processes correctly
   - Matching logic works (exact vs substring)
   - Kill signals sent correctly

2. **Integration Tests**:
   - Start watchdog → wait 30sec → verify timer fires
   - Launch forbidden app (e.g., notepad.exe) → verify killed within 30sec
   - Check logs for kill messages
   - Verify event loop stays responsive (no blocking)

3. **Platform-Specific**:
   - **Windows**: Test with `explorer.exe`, `notepad.exe`, `chrome.exe`
   - **macOS**: Test with `TextEdit.app`, `Safari.app`
   - **Linux**: Test with `gedit`, `firefox`

4. **Performance**:
   - Measure time spent in timer callback (should be < 10ms)
   - Test on system with 200+ processes
   - Verify no UI freeze during enumeration

## Risk Assessment (Taboo Apps)

| Risk                            | Severity    | Mitigation                                                   |
| ------------------------------- | ----------- | ------------------------------------------------------------ |
| Kill critical system process    | 🔴 Critical | Hardcoded blacklist: never kill `csrss.exe`, `systemd`, etc. |
| Block event loop                | 🟡 High     | Background thread + timeout budget                           |
| False positive kills            | 🟡 Medium   | Exact basename matching instead of substring                 |
| Performance on slow systems     | 🟢 Low      | 30-second interval, background thread                        |
| Config typo kills wrong process | 🟡 Medium   | Dry-run mode: log what would be killed                       |

## Final Summary

**Current Status**: Disabled due to blocking calls in timer callback (same issue as `waitForStarted()` we just fixed)

**Root Cause**:

- ProcessEnumerator blocks during enumerate (10-50ms)
- killInternal blocks during wait (100ms-3sec per process)
- Timer callback runs in main thread

**Solution**:
UI freeze during enumeration

## Risk Assessment

| Risk                            | Severity    | Mitigation                                                   |
| ------------------------------- | ----------- | ------------------------------------------------------------ |
| Kill critical system process    | 🔴 Critical | Hardcoded blacklist: never kill `csrss.exe`, `systemd`, etc. |
| Block event loop                | 🟡 High     | Background thread + timeout budget                           |
| False positive kills            | 🟡 Medium   | Exact basename matching instead of substring                 |
| Performance on slow systems     | 🟢 Low      | 30-second interval, background thread                        |
| Config typo kills wrong process | 🟡 Medium   | Dry-run mode: log what would be killed                       |

## Summary

**Current Status**: Disabled due to blocking calls in timer callback (same issue as `waitForStarted()` we just fixed)

**Root Cause**:

- ProcessEnumerator blocks during enumerate (10-50ms)
- killInternal blocks during wait (100ms-3sec per process)
- Timer callback runs in main thread

**Solution**:

- Move enumeration to background thread (QtConcurrent)
- Non-blocking kill (send signal, don't wait)
- Timeout budget per cycle (max 50ms)

**Benefit**: Secure kiosk environment, prevents unauthorized tools, improves performance

**Next Step**: Get your approval on design decisions above, then I'll implement Phase 1.

---

## Decisions (Feb 16, 2026)

- **Restart logic**: keep code for compatibility, do not call it (commented as deprecated).
- **Kill strategy**: use SIGKILL (no graceful wait).
- **explorer.exe**: log-only by default, allow override in config.
- **Interval**: 30 seconds default.
- **Concurrency**: use QtConcurrent for background checks.
- **Start platform**: macOS first; in dev mode log-only.
- **Defaults**: debug builds disabled/log-only by default; production builds enabled by default after validation.

**Ready to proceed with implementation.**
