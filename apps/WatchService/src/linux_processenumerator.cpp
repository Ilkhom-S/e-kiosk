/* @file Реализация перечислителя процессов в системе для Linux. */

// STL
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <Common/QtHeadersEnd.h>

// System
#include <sys/wait.h>
#include <unistd.h>

// Project
#include <dirent.h>
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

    DIR *procDir = opendir("/proc");
    if (!procDir)
    {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(procDir)) != nullptr)
    {
        // Check if this is a process directory (numeric name)
        char *endptr;
        pid_t pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0' || pid <= 0)
        {
            continue; // Not a process directory
        }

        // Read process executable path from /proc/<pid>/exe
        QString exePath = QString("/proc/%1/exe").arg(pid);
        char buffer[PATH_MAX];
        ssize_t len = readlink(exePath.toLocal8Bit().constData(), buffer, sizeof(buffer) - 1);
        if (len > 0)
        {
            buffer[len] = '\0';
            ProcessInfo info;
            info.pid = static_cast<PID>(pid);
            info.path = QString::fromLocal8Bit(buffer);
            mProcesses.insert(info.pid, info);
        }
    }

    closedir(procDir);
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

//----------------------------------------------------------------------------</content>
<parameter name = "filePath"> / Users / ilkhom / Projects / Humo / e -
    kiosk / apps / WatchService / src / linux_processenumerator.cpp