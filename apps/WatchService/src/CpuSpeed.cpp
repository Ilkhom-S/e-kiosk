/* @file Реализация грубого подсчета скорости процессора. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGlobal>
#include <Common/QtHeadersEnd.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_MAC
#include <sys/sysctl.h>
#endif

// Standard C/C++
#include <cstdio>
#include <cstdint>

// Constants for CPU speed detection
namespace
{
    const unsigned DefaultCpuSpeedMHz = 2000; // 2 GHz fallback
    const unsigned MinReasonableCpuSpeedMHz = 100;   // 100 MHz minimum
    const unsigned MaxReasonableCpuSpeedMHz = 10000; // 10 GHz maximum
} // namespace

//----------------------------------------------------------------------------
/**
 * @brief Get CPU speed in MHz using platform-specific methods
 *
 * This function detects CPU frequency using different methods depending on the platform:
 * - Windows: Uses RDTSC instruction with QueryPerformanceCounter for timing
 * - macOS: Uses sysctl to query hardware CPU frequency
 * - Linux: Parses /proc/cpuinfo for CPU frequency
 * - Other platforms: Returns a reasonable default value
 *
 * The result is cached after first call for performance.
 *
 * @return CPU speed in MHz, or default value if detection fails
 */
unsigned CPUSpeed()
{
    static unsigned speed = 0;

    if (speed == 0)
    {
#ifdef Q_OS_WIN
        // Windows implementation using RDTSC and QueryPerformanceCounter
        qint64 frequency;
        qint64 now, end;
        qint64 counter;

        if (QueryPerformanceFrequency((PLARGE_INTEGER)&frequency) &&
            QueryPerformanceCounter((PLARGE_INTEGER)&end))
        {
            end += frequency;
            counter = rdtsc();
            do
            {
                QueryPerformanceCounter((PLARGE_INTEGER)&now);
            } while (now < end);

            counter = rdtsc() - counter;
            if (counter > 0)
            {
                speed = unsigned(counter / 1000000);
            }
        }

        // Validate result
        if (speed < MinReasonableCpuSpeedMHz || speed > MaxReasonableCpuSpeedMHz)
        {
            speed = DefaultCpuSpeedMHz;
        }

#elif defined(Q_OS_MAC)
        // macOS implementation using sysctl
        size_t size = sizeof(speed);
        if (sysctlbyname("hw.cpufrequency", &speed, &size, nullptr, 0) == 0)
        {
            speed = unsigned(speed / 1000000); // Convert Hz to MHz
        }
        else
        {
            // Fallback: try alternative sysctl name
            uint64_t freq = 0;
            size = sizeof(freq);
            if (sysctlbyname("hw.cpufrequency", &freq, &size, nullptr, 0) == 0)
            {
                speed = unsigned(freq / 1000000); // Convert Hz to MHz
            }
            else
            {
                speed = DefaultCpuSpeedMHz; // Fallback for modern Macs
            }
        }

        // Validate result
        if (speed < MinReasonableCpuSpeedMHz || speed > MaxReasonableCpuSpeedMHz)
        {
            speed = DefaultCpuSpeedMHz;
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

        // Validate result
        if (speed == 0 || speed < MinReasonableCpuSpeedMHz || speed > MaxReasonableCpuSpeedMHz)
        {
            speed = DefaultCpuSpeedMHz; // Fallback for modern Linux systems
        }

#else
        // Fallback for unknown platforms
        speed = DefaultCpuSpeedMHz; // Assume 2 GHz for modern systems
#endif
    }

    return speed;
}

#ifdef Q_OS_WIN
// Windows-specific RDTSC function
inline qint64 rdtsc()
{
#ifdef _MSC_VER
    return __rdtsc();
#else
    // Fallback for other compilers - this is not accurate but better than nothing
    return 0;
#endif
}
#endif

//----------------------------------------------------------------------------
