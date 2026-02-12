/* @file Реализация перечислителя процессов в системе для macOS. */

#include <QtCore/QDir>
#include <QtCore/QMap>
#include <QtCore/QString>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <libproc.h>
#include <sys/wait.h>
#include <unistd.h>

#include "processenumerator.h"

namespace CProcessEnumerator {
const int KillTimeoutMs = 3000; // 3 seconds timeout for graceful termination
} // namespace CProcessEnumerator

//----------------------------------------------------------------------------
void ProcessEnumerator::enumerate() {
    m_Processes.clear();

    // Get list of all PIDs
    int pidCount = proc_listallpids(nullptr, 0);
    if (pidCount <= 0) {
        return;
    }

    auto *pids = new pid_t[pidCount];
    pidCount =
        static_cast<int>(proc_listallpids(pids, static_cast<size_t>(pidCount) * sizeof(pid_t)));

    for (int i = 0; i < pidCount; ++i) {
        pid_t pid = pids[i];
        if (pid <= 0) {
            continue;
        }

        // Get process path using proc_pidpath
        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        if (proc_pidpath(pid, pathBuffer, sizeof(pathBuffer)) > 0) {
            ProcessInfo info;
            info.pid = static_cast<PID>(pid);
            info.path = QString::fromLocal8Bit(pathBuffer);
            m_Processes.insert(info.pid, info);
        }
    }

    delete[] pids;
}

//----------------------------------------------------------------------------
bool ProcessEnumerator::killInternal(PID aPid, quint32 &aErrorCode) {
    aErrorCode = 0;

    // Send SIGTERM first for graceful termination
    if (::kill(static_cast<pid_t>(aPid), SIGTERM) == 0) {
        // Wait for the process to terminate gracefully
        int status = 0;
        pid_t result = waitpid(static_cast<pid_t>(aPid), &status, WNOHANG);

        if (result == 0) {
            // Process is still running, wait a bit
            usleep(100000); // 100ms
            result = waitpid(static_cast<pid_t>(aPid), &status, WNOHANG);
        }

        if (result == 0) {
            // Process still running, send SIGKILL
            if (::kill(static_cast<pid_t>(aPid), SIGKILL) == 0) {
                // Wait for the process to be killed
                waitpid(static_cast<pid_t>(aPid), &status, 0);
                return true;
            }
            aErrorCode = errno;
            return false;
        }
        if (result == static_cast<pid_t>(aPid)) {
            // Process terminated gracefully
            return true;
        }
        aErrorCode = errno;
        return false;
    }
    aErrorCode = errno;
    return false;
}

//----------------------------------------------------------------------------
// Base class method implementations
//----------------------------------------------------------------------------

ProcessEnumerator::ProcessEnumerator() {
    enumerate();
}

//----------------------------------------------------------------------------
ProcessEnumerator::const_iterator ProcessEnumerator::begin() const {
    return m_Processes.begin();
}

//----------------------------------------------------------------------------
ProcessEnumerator::const_iterator ProcessEnumerator::end() const {
    return m_Processes.end();
}

//----------------------------------------------------------------------------
bool ProcessEnumerator::kill(PID aPid, quint32 &aErrorCode) const {
    // On macOS, we don't have special handling for explorer-like processes
    // Just call the internal kill method
    return killInternal(aPid, aErrorCode);
}

//----------------------------------------------------------------------------
