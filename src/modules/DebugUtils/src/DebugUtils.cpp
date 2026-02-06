//---------------------------------------------------------------------------
// Modern DebugUtils implementation using Boost.Stacktrace

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QtGlobal>

#include <DebugUtils/DebugUtils.h>

#ifdef Q_OS_WIN
#define NOMINMAX            // HACK for QDateTime Qt 5.0.0
#define _WIN32_WINNT 0x0601 // Windows 7+ for Boost.Stacktrace compatibility
#include <windows.h>
#endif

// Signal handling for Unix-like systems
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#define _GNU_SOURCE // Required for Boost.Stacktrace _Unwind_Backtrace
#include <csignal>
#endif

// Boost.Stacktrace for modern stack tracing
#include <boost/stacktrace.hpp>

//---------------------------------------------------------------------------
// Modern implementation using Boost.Stacktrace
void DumpCallstack(QStringList &aStack, void *aContext) {
    try {
        // Get stack trace, skip first frame (this function)
        auto st = boost::stacktrace::stacktrace(1, 32);

        aStack.clear();

        for (size_t i = 0; i < st.size(); ++i) {
            const auto &frame = st[i];

            QString frameStr;

            // Address
            frameStr += QString("0x%1").arg((quintptr)frame.address(), 0, 16);

            // Function name (demangled)
            if (!frame.name().empty()) {
                frameStr += QString(" (%1)").arg(QString::fromStdString(frame.name()));
            }

            // Source location if available
            if (!frame.source_file().empty()) {
                frameStr += QString(" at %1:%2")
                                .arg(QString::fromStdString(frame.source_file()))
                                .arg(frame.source_line());
            }

            aStack.append(frameStr);
        }

        // If no frames captured, provide fallback
        if (aStack.isEmpty()) {
            aStack.append("No stack trace available (Boost.Stacktrace)");
        }
    } catch (const std::exception &e) {
        aStack.clear();
        aStack.append(QString("Failed to get stack trace: %1").arg(e.what()));
    }
}

//---------------------------------------------------------------------------
// Enhanced exception handler with crash logging
void SetUnhandledExceptionsHandler(TExceptionHandler aHandler) {
#ifdef Q_OS_WIN
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
    _set_abort_behavior(0, _CALL_REPORTFAULT);

    // Set Windows unhandled exception filter
    SetUnhandledExceptionFilter(aHandler);

    // Also set C++ terminate handler for better crash reporting
    std::set_terminate([]() {
        QStringList stack;
        DumpCallstack(stack);

        // Create crash log
        QString crashDir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/EKiosk";
        QDir().mkpath(crashDir);

        QString crashFile =
            crashDir + QString("/crash_%1.log")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

        QFile file(crashFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "EKiosk Crash Report\n";
            stream << "Time: " << QDateTime::currentDateTime().toString() << "\n";
            stream << "Using Boost.Stacktrace\n\n";
            stream << "Stack Trace:\n";

            for (const QString &frame : stack) {
                stream << frame << "\n";
            }

            stream << "\nEnd of crash report.\n";
        }

        // Call original handler if provided
        // Note: In terminate handler, we can't call the original Windows handler directly
        // This would need more sophisticated implementation

        std::abort();
    });
#elif defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    // For macOS and Linux, set up signal handlers for common crash signals
    static TExceptionHandler s_originalHandler = aHandler;

    auto signalHandler = [](int sig) {
        QStringList stack;
        DumpCallstack(stack);

        // Create crash log
        QString crashDir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/EKiosk";
        QDir().mkpath(crashDir);

        QString crashFile =
            crashDir + QString("/crash_%1.log")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

        QFile file(crashFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "EKiosk Crash Report\n";
            stream << "Time: " << QDateTime::currentDateTime().toString() << "\n";
            stream << "Signal: " << sig << "\n";
            stream << "Using Boost.Stacktrace\n\n";
            stream << "Stack Trace:\n";

            for (const QString &frame : stack) {
                stream << frame << "\n";
            }

            stream << "\nEnd of crash report.\n";
        }

        // Call original handler if provided
        if (s_originalHandler) {
            s_originalHandler(sig);
        }

        // Re-raise the signal to ensure proper termination
        signal(sig, SIG_DFL);
        raise(sig);
    };

    // Set up handlers for common crash signals
    signal(SIGSEGV, signalHandler); // Segmentation fault
    signal(SIGABRT, signalHandler); // Abort
    signal(SIGBUS, signalHandler);  // Bus error
    signal(SIGILL, signalHandler);  // Illegal instruction
    signal(SIGFPE, signalHandler);  // Floating point exception
    signal(SIGTERM, signalHandler); // Termination request

    // Also set C++ terminate handler
    std::set_terminate([]() {
        QStringList stack;
        DumpCallstack(stack);

        // Create crash log
        QString crashDir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/EKiosk";
        QDir().mkpath(crashDir);

        QString crashFile =
            crashDir + QString("/crash_%1.log")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

        QFile file(crashFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "EKiosk Crash Report (std::terminate)\n";
            stream << "Time: " << QDateTime::currentDateTime().toString() << "\n";
            stream << "Using Boost.Stacktrace\n\n";
            stream << "Stack Trace:\n";

            for (const QString &frame : stack) {
                stream << frame << "\n";
            }

            stream << "\nEnd of crash report.\n";
        }

        std::abort();
    });
#else
#error The method SetUnhandledExceptionsHandler is not implemented for the current platform.
#endif // Q_OS_WIN
}

//---------------------------------------------------------------------------
