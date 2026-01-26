

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#include <TargetConditionals.h>
#include <mach/mach.h>
#include <mach/task.h>
#include <mach/task_info.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/message.h>
#include <mach/vm_map.h>
#include <mach/error.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h> // getpid, sysconf
#include <string.h> // strerror
// Fallback definitions if not present
#ifndef TASK_BASIC_INFO
#define TASK_BASIC_INFO 20
#endif
#ifndef TASK_BASIC_INFO_COUNT
#define TASK_BASIC_INFO_COUNT (sizeof(struct task_basic_info) / sizeof(natural_t))
#endif
#ifndef KERN_SUCCESS
#define KERN_SUCCESS 0
#endif
#ifndef KERN_FAILURE
#define KERN_FAILURE 5
#endif
#ifndef CTL_HW
#define CTL_HW 6
#endif
#ifndef HW_MEMSIZE
#define HW_MEMSIZE 24
#endif
#endif

#if defined(__linux__)
#include <unistd.h> // getpid, sysconf
#include <string.h> // strerror
#include <sys/types.h>
#include <sys/sysinfo.h>
#endif

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QSysInfo>
#include <QtCore/QTextStream>
#include <Common/QtHeadersEnd.h>

#if defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#endif

// System
#include <SysUtils/ISysUtils.h>
#include <SysUtils/ISysUtils.h>
#include <sys/utsname.h>
#include <sys/utsname.h>

QString ISysUtils::getOSVersionInfo() {
    struct utsname uts;
    if (uname(&uts) == 0) {
        return QString("%1 %2 (%3)").arg(uts.sysname).arg(uts.release).arg(uts.machine);
    } else {
        return QSysInfo::prettyProductName();
    }
}

//---------------------------------------------------------------------------
// Удаляет BOM из файла, если он есть (UTF-8 BOM)
QString ISysUtils::rmBOM(const QString &aFile) {
    QFile file(aFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return aFile;
    }
    QByteArray data = file.readAll();
    file.close();

    static const QByteArray utf8BOM = QByteArray::fromHex("efbbbf");
    if (data.startsWith(utf8BOM)) {
        data = data.mid(3);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(data);
            file.close();
        }
    }
    return aFile;
}

//---------------------------------------------------------------------------
// Перезагрузка системы (Linux: reboot syscall, macOS: shutdown command)
int ISysUtils::systemReboot() {
#if defined(__linux__)
    sync();
    int ret = reboot(RB_AUTOBOOT);
    return ret;
#elif defined(__APPLE__)
    // On macOS, use shutdown command
    int ret = system("sudo shutdown -r now");
    return ret;
#else
#include <mach/vm_map.h>
    // Fallback for other Unix
    int ret = system("sudo shutdown -r now");
    return ret;
#endif
}

//---------------------------------------------------------------------------
// Выключение системы (Linux: reboot syscall with poweroff, macOS: shutdown command)
int ISysUtils::systemShutdown() {
#if defined(__linux__)
// Try to use the reboot syscall with RB_POWER_OFF (requires root)
#include <unistd.h>
#include <sys/reboot.h>
    sync();
    int ret = reboot(RB_POWER_OFF);
    return ret;
#elif defined(__APPLE__)
    // On macOS, use shutdown command
    int ret = system("sudo shutdown -h now");
    return ret;
#else
    // Fallback for other Unix
    int ret = system("sudo shutdown -h now");
    return ret;
#endif
}

//---------------------------------------------------------------------------
// Отключение скринсейвера для Linux (X11) и macOS
void ISysUtils::disableScreenSaver() {
#if defined(__APPLE__)
    // On macOS, use caffeinate to prevent sleep/screensaver
    // This will keep the system awake while the app is running
    int ret = system("caffeinate -dimsu &");
    Q_UNUSED(ret);
#elif defined(__linux__)
    // On Linux/X11, use xset to disable screensaver and DPMS
    // This works on Raspberry Pi OS with X11
    int ret1 = system("xset s off");
    int ret2 = system("xset -dpms");
    int ret3 = system("xset s noblank");
    Q_UNUSED(ret1);
    Q_UNUSED(ret2);
    Q_UNUSED(ret3);
#else
    qWarning() << "disableScreenSaver() not implemented for this Unix platform";
#endif
}

//---------------------------------------------------------------------------
// Включение/выключение дисплея (имитация активности для пробуждения)
void ISysUtils::displayOn(bool aOn) {
#if defined(__APPLE__)
    if (aOn) {
        // Prevent display sleep (caffeinate)
        int ret = system("caffeinate -u -t 2"); // -u: user activity, -t: seconds
        Q_UNUSED(ret);
    } else {
        // Turn display off (pmset)
        int ret = system("pmset displaysleepnow");
        Q_UNUSED(ret);
    }
#elif defined(__linux__)
    if (aOn) {
        // Wake up display: force DPMS on, simulate mouse move if possible
        int ret1 = system("xset dpms force on");
        int ret2 = system("xset s reset");
        // Try to move mouse with xdotool if available
        int ret3 = system("xdotool mousemove_relative 1 0; xdotool mousemove_relative -- -1 0");
        Q_UNUSED(ret1);
        Q_UNUSED(ret2);
        Q_UNUSED(ret3);
    } else {
        // Turn display off
        int ret = system("xset dpms force off");
        Q_UNUSED(ret);
    }
#else
    Q_UNUSED(aOn);
    qWarning() << "displayOn() not implemented for this Unix platform";
#endif
}

//---------------------------------------------------------------------------
// Принудительный запуск скринсейвера
void ISysUtils::runScreenSaver() {
#if defined(__APPLE__)
    // On macOS, use AppleScript to start the screensaver
    int ret = system("osascript -e 'tell application \"System Events\" to start current screen saver'");
    Q_UNUSED(ret);
#elif defined(__linux__)
    // On Linux/X11, use xset to activate screensaver
    int ret = system("xset s activate");
    Q_UNUSED(ret);
#else
    qWarning() << "runScreenSaver() not implemented for this Unix platform";
#endif
}

//---------------------------------------------------------------------------
// Установка системного времени (Linux: date, macOS: systemsetup)
void ISysUtils::setSystemTime(QDateTime aDateTime) noexcept(false) {
#if defined(__linux__)
    // Format: date MMDDhhmmYYYY.SS
    QDateTime dt = aDateTime.toLocalTime();
    QString cmd = QString("sudo date %1%2%3%4%5.%6")
                      .arg(dt.date().month(), 2, 10, QChar('0'))
                      .arg(dt.date().day(), 2, 10, QChar('0'))
                      .arg(dt.time().hour(), 2, 10, QChar('0'))
                      .arg(dt.time().minute(), 2, 10, QChar('0'))
                      .arg(dt.date().year())
                      .arg(dt.time().second(), 2, 10, QChar('0'));
    int ret = system(cmd.toUtf8().constData());
    if (ret != 0) {
        throw std::runtime_error("Failed to set system time (date command, Linux)");
    }
#elif defined(__APPLE__)
    // macOS: systemsetup -setusingnetworktime off; systemsetup -setdate; systemsetup -settime
    QDateTime dt = aDateTime.toLocalTime();
    QString dateCmd = QString("sudo systemsetup -setusingnetworktime off && sudo systemsetup -setdate %1/%2/%3 && sudo "
                              "systemsetup -settime %4:%5:%6")
                          .arg(dt.date().month(), 2, 10, QChar('0'))
                          .arg(dt.date().day(), 2, 10, QChar('0'))
                          .arg(dt.date().year())
                          .arg(dt.time().hour(), 2, 10, QChar('0'))
                          .arg(dt.time().minute(), 2, 10, QChar('0'))
                          .arg(dt.time().second(), 2, 10, QChar('0'));
    int ret = system(dateCmd.toUtf8().constData());
    if (ret != 0) {
        throw std::runtime_error("Failed to set system time (systemsetup, macOS)");
    }
#else
    Q_UNUSED(aDateTime);
    qWarning() << "setSystemTime() not implemented for this Unix platform";
#endif
}

//---------------------------------------------------------------------------
// Сон потока
void ISysUtils::sleep(int aMs) {
    Q_ASSERT(aMs > 0);
    QThread::msleep(aMs);
}

//---------------------------------------------------------------------------
// Получить описание последней системной ошибки (errno)
QString ISysUtils::getLastErrorMessage() {
    return getErrorMessage(errno);
}

//---------------------------------------------------------------------------
// Получить описание системной ошибки (errno)
QString ISysUtils::getErrorMessage(ulong aError, bool aNativeLanguage) {
    Q_UNUSED(aNativeLanguage);
    QString result = QString("error: %1").arg(aError);
    const char *msg = strerror(static_cast<int>(aError));
    if (msg && *msg) {
        result += QString(" (%1)").arg(QString::fromLocal8Bit(msg));
    }
    return result;
}

//---------------------------------------------------------------------------
// Получить количество памяти, используемое процессом (Unix/macOS)
bool ISysUtils::getProcessMemoryUsage(MemoryInfo &aMemoryInfo, const QProcess *aProcess) {
#if defined(__linux__)
    aMemoryInfo = MemoryInfo();
    qint64 pid = 0;
    if (aProcess) {
        pid = aProcess->processId();
    } else {
        pid = getpid();
    }
    QString statmPath = QString("/proc/%1/statm").arg(pid);
    QFile statm(statmPath);
    if (statm.open(QIODevice::ReadOnly)) {
        QByteArray line = statm.readLine();
        statm.close();
        QList<QByteArray> parts = line.split(' ');
        if (parts.size() >= 2) {
            long pageSize = sysconf(_SC_PAGESIZE);
            aMemoryInfo.total = 0;     // Not available from statm
            aMemoryInfo.totalUsed = 0; // Not available from statm
            aMemoryInfo.processUsed = parts[1].toLongLong() * pageSize;
            return true;
        }
    }
    return false;
#elif defined(__APPLE__)
    aMemoryInfo = MemoryInfo();
    pid_t pid = 0;
    if (aProcess) {
        pid = static_cast<pid_t>(aProcess->processId());
    } else {
        pid = getpid();
    }
    mach_port_t task;
    kern_return_t kr = KERN_FAILURE;
    if (pid == getpid()) {
        task = mach_task_self();
        kr = KERN_SUCCESS;
    } else {
        kr = task_for_pid(mach_task_self(), pid, &task);
    }
    if (kr == KERN_SUCCESS) {
        struct task_basic_info info;
        mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;
        if (task_info(task, TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
            aMemoryInfo.processUsed = info.resident_size;
            // Total and used system memory: use sysctl
            int mib[2] = {CTL_HW, HW_MEMSIZE};
            int64_t memsize = 0;
            size_t len = sizeof(memsize);
            if (sysctl(mib, 2, &memsize, &len, NULL, 0) == 0) {
                aMemoryInfo.total = memsize;
            }
            // Used memory: not trivial, so leave as 0
            return true;
        }
    }
    return false;
#else
    Q_UNUSED(aMemoryInfo);
    Q_UNUSED(aProcess);
    return false;
#endif
}

//--------------------------------------------------------------------------------
bool ISysUtils::bringWindowToFront(WId aWindow) {
    Q_UNUSED(aWindow)
    // Not implemented on Unix-like systems
    return false;
}

//--------------------------------------------------------------------------------
bool ISysUtils::bringWindowToFront(const QString &aWindowTitle) {
    Q_UNUSED(aWindowTitle)
    // Not implemented on Unix-like systems
    return false;
}

//--------------------------------------------------------------------------------
