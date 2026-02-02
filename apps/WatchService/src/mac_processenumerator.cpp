/* @file Реализация перечислителя процессов в системе для macOS. */

// STL
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// System
#include <sys/wait.h>
#include <unistd.h>

// Project
#include <errno.h>
#include <libproc.h>
#include "processenumerator.h"
#include <signal.h>

namespace CProcessEnumerator
{
    const int KillTimeoutMs = 3000; // 3 seconds timeout for graceful termination
} // namespace CProcessEnumerator

//----------------------------------------------------------------------------
void ProcessEnumerator::enumerate()
{
    mProcesses.clear();

    // Get list of all PIDs
    int pidCount = proc_listallpids(nullptr, 0);
    if (pidCount <= 0)
    {
        return;
    }

    pid_t *pids = new pid_t[pidCount];
    pidCount = proc_listallpids(pids, pidCount * sizeof(pid_t));

    for (int i = 0; i < pidCount; ++i)
    {
        pid_t pid = pids[i];
        if (pid <= 0)
        {
            continue;
        }

        // Get process path using proc_pidpath
        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        if (proc_pidpath(pid, pathBuffer, sizeof(pathBuffer)) > 0)
        {
            ProcessInfo info;
            info.pid = static_cast<PID>(pid);
            info.path = QString::fromLocal8Bit(pathBuffer);
            mProcesses.insert(info.pid, info);
        }
    }

    delete[] pids;
}

//----------------------------------------------------------------------------
bool ProcessEnumerator::killInternal(PID aPid, quint32 &aErrorCode) const
{
    aErrorCode = 0;

    // Send SIGTERM first for graceful termination
    if (::kill(static_cast<pid_t>(aPid), SIGTERM) == 0)
    {
        // Wait for the process to terminate gracefully
        int status;
        pid_t result = waitpid(static_cast<pid_t>(aPid), &status, WNOHANG);

        if (result == 0)
        {
            // Process is still running, wait a bit
            usleep(100000); // 100ms
            result = waitpid(static_cast<pid_t>(aPid), &status, WNOHANG);
        }

        if (result == 0)
        {
            // Process still running, send SIGKILL
            if (::kill(static_cast<pid_t>(aPid), SIGKILL) == 0)
            {
                // Wait for the process to be killed
                waitpid(static_cast<pid_t>(aPid), &status, 0);
                return true;
            }
            else
            {
                aErrorCode = errno;
                return false;
            }
        }
        else if (result == static_cast<pid_t>(aPid))
        {
            // Process terminated gracefully
            return true;
        }
        else
        {
            aErrorCode = errno;
            return false;
        }
    }
    else
    {
        aErrorCode = errno;
        return false;
    }
}

//----------------------------------------------------------------------------