/* @file Реализация грубого подсчета скорости процессора. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGlobal>
#include <Common/QtHeadersEnd.h>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#ifdef Q_OS_MACOS
#include <sys/sysctl.h>
#endif

// Standard C/C++
#include <cstdio>
#include <cstdint>

//----------------------------------------------------------------------------
unsigned CPUSpeed()
{
    static unsigned speed = 0;

    if (speed == 0)
    {
#ifdef Q_OS_WIN32
        // Windows implementation using RDTSC and QueryPerformanceCounter
        __int64 frequency;
        __int64 now, end;
        __int64 counter;

        QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
        QueryPerformanceCounter((PLARGE_INTEGER)&end);
        end += frequency;
        counter = rdtsc();
        do
        {
            QueryPerformanceCounter((PLARGE_INTEGER)&now);
        } while (now < end);

        counter = rdtsc() - counter;
        speed = unsigned(counter / 1000000);

#elif defined(Q_OS_MACOS) || defined(Q_OS_MAC)
        // macOS implementation using sysctl
        size_t size = sizeof(speed);
        if (sysctlbyname("hw.cpufrequency", &speed, &size, nullptr, 0) != 0)
        {
            // Fallback: try to get frequency from sysctl hw.freq
            uint64_t freq = 0;
            size = sizeof(freq);
            if (sysctlbyname("hw.cpufrequency", &freq, &size, nullptr, 0) == 0)
            {
                speed = unsigned(freq / 1000000); // Convert Hz to MHz
            }
            else
            {
                speed = 2000; // Fallback for modern Macs (2 GHz)
            }
        }
        else
        {
            speed = unsigned(speed / 1000000); // Convert Hz to MHz
        }

#elif defined(Q_OS_LINUX)
        // Linux implementation using /proc/cpuinfo
        FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
        if (cpuinfo)
        {
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), cpuinfo))
            {
                if (sscanf(buffer, "cpu MHz : %u", &speed) == 1)
                {
                    break;
                }
            }
            fclose(cpuinfo);
        }
        if (speed == 0)
        {
            speed = 2000; // Fallback for modern Linux systems
        }

#else
        // Fallback for unknown platforms
        speed = 2000; // Assume 2 GHz for modern systems
#endif
    }

    return speed;
}

#ifdef Q_OS_WIN32
// Windows-specific RDTSC function
__inline __int64 rdtsc()
{
    __asm rdtsc
}
#endif

//----------------------------------------------------------------------------
