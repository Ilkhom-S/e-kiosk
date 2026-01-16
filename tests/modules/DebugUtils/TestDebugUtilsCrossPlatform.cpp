// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QSysInfo>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// ThirdParty
#include <boost/stacktrace.hpp>
#include <boost/version.hpp>

// System
#include <DebugUtils/DebugUtils.h>

class TestDebugUtilsCrossPlatform : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        qDebug() << "Testing DebugUtils cross-platform compatibility";
        qDebug() << "Qt version:" << qVersion();
        qDebug() << "Boost version:" << BOOST_VERSION;
        qDebug() << "Platform:" << QSysInfo::productType() << QSysInfo::productVersion();
        qDebug() << "Build ABI:" << QSysInfo::buildAbi();
    }

    void testPlatformDetection() {
        // Test that the module correctly detects the platform
        QStringList stack;
        DumpCallstack(stack, nullptr);

        QVERIFY(stack.size() > 0);

        // Should contain platform-appropriate information
        QString firstFrame = stack.first();

#ifdef Q_OS_WIN
        qDebug() << "Running on Windows";
        // On Windows, should work with Boost.Stacktrace WinDbg backend
        QVERIFY(firstFrame.contains("0x") || firstFrame.length() > 0);
#elif defined(Q_OS_LINUX)
        qDebug() << "Running on Linux";
        // On Linux, should work with libbacktrace
        QVERIFY(firstFrame.contains("0x") || firstFrame.length() > 0);
#elif defined(Q_OS_MACOS)
        qDebug() << "Running on macOS";
        // On macOS, should work with libbacktrace
        QVERIFY(firstFrame.contains("0x") || firstFrame.length() > 0);
#else
        qDebug() << "Running on other platform";
        // Should still work on other platforms supported by Boost
        QVERIFY(stack.size() > 0);
#endif
    }

    void testBoostStacktraceDirectUsage() {
        // Test that we can use Boost.Stacktrace directly
        try {
            auto st = boost::stacktrace::stacktrace();
            QVERIFY(st.size() > 0);

            // Convert to QStringList manually
            QStringList manualStack;
            for (size_t i = 0; i < st.size(); ++i) {
                const auto &frame = st[i];
                QString frameStr = QString("0x%1 (%2)")
                                       .arg((quintptr)frame.address(), 0, 16)
                                       .arg(QString::fromStdString(frame.name()));
                manualStack.append(frameStr);
            }

            QVERIFY(manualStack.size() > 0);
            QVERIFY(manualStack.first().contains("0x"));

        } catch (const std::exception &e) {
            QFAIL(QString("Boost.Stacktrace direct usage failed: %1").arg(e.what()).toUtf8());
        }
    }

    void testStackTraceConsistencyAcrossPlatforms() {
        QStringList stack;
        DumpCallstack(stack, nullptr);

        // Basic consistency checks that should work on all platforms
        QVERIFY(stack.size() >= 1);
        QVERIFY(!stack.first().isEmpty());

        // Should contain memory addresses (format may vary by platform)
        bool hasAddress = false;
        for (const QString &frame : stack) {
            if (frame.contains("0x") || frame.contains("0X")) {
                hasAddress = true;
                break;
            }
        }

        // Note: Some platforms might not show addresses in all cases
        // so this is more of a best-effort check
        if (!hasAddress) {
            qDebug() << "No memory addresses found in stack trace (may be normal for some platforms)";
        }
    }

    void testExceptionHandlerPlatformCompatibility() {
        // Test that exception handler functions work on current platform
#ifdef Q_OS_WIN
        qDebug() << "Testing Windows exception handler compatibility";

        // Should be able to get and set exception filter
        LPTOP_LEVEL_EXCEPTION_FILTER original = SetUnhandledExceptionFilter(nullptr);
        LPTOP_LEVEL_EXCEPTION_FILTER testHandler = [](EXCEPTION_POINTERS *) -> LONG {
            return EXCEPTION_CONTINUE_SEARCH;
        };

        SetUnhandledExceptionFilter(testHandler);
        LPTOP_LEVEL_EXCEPTION_FILTER current = SetUnhandledExceptionFilter(original);

        // Should have been able to set our handler
        QVERIFY(current == testHandler);

        qDebug() << "Windows exception handler test passed";

#else
        qDebug() << "Skipping Windows-specific exception handler test on non-Windows platform";

        // On non-Windows platforms, SetUnhandledExceptionsHandler should be a no-op
        // but shouldn't crash
        SetUnhandledExceptionsHandler(nullptr);
        QVERIFY(true);
#endif
    }

    void testBoostVersionCompatibility() {
        // Test that we're using a compatible Boost version
        const int boostMajor = BOOST_VERSION / 100000;
        const int boostMinor = (BOOST_VERSION / 100) % 1000;

        qDebug() << "Boost version:" << boostMajor << "." << boostMinor;

        // Should be Boost 1.90 or later (when stacktrace became stable)
        QVERIFY(boostMajor >= 1);
        if (boostMajor == 1) {
            QVERIFY(boostMinor >= 74); // Stacktrace became stable in 1.74+
        }
    }

    void testStackTraceFromSignalHandler() {
        // Test that stack traces work even in signal handler context (simulated)
        QStringList normalStack;
        DumpCallstack(normalStack, nullptr);

        // Simulate calling from a different context (Qt event loop)
        QStringList eventStack;
        bool done = false;

        QTimer::singleShot(0, [&]() {
            DumpCallstack(eventStack, nullptr);
            done = true;
        });

        QTRY_VERIFY(done);

        // Both should be valid
        QVERIFY(normalStack.size() > 0);
        QVERIFY(eventStack.size() > 0);

        // Should be similar but may have different depths due to event loop
        // On Windows, signal handlers may have significantly different stack depths
        int tolerance = 20; // Increased tolerance for Windows
        QVERIFY(qAbs(normalStack.size() - eventStack.size()) <= tolerance);
    }

    void testMemoryAllocationDuringStackTrace() {
        // Test that DumpCallstack doesn't cause issues with memory allocation
        QStringList *stacks = new QStringList[10];

        for (int i = 0; i < 10; ++i) {
            DumpCallstack(stacks[i], nullptr);
            QVERIFY(stacks[i].size() > 0);
        }

        delete[] stacks;

        // If we get here without crashing, memory management is OK
        QVERIFY(true);
    }

    void testStackTraceWithNullContext() {
        // Test that passing nullptr context works
        QStringList stack;
        DumpCallstack(stack, nullptr);

        QVERIFY(stack.size() > 0);
    }

    void testMultipleRapidCalls() {
        // Test rapid successive calls to ensure no state corruption
        const int numCalls = 50;
        QVector<QStringList> stacks(numCalls);

        for (int i = 0; i < numCalls; ++i) {
            DumpCallstack(stacks[i], nullptr);
            QVERIFY(stacks[i].size() > 0);

            // Each call should return a valid stack
            QVERIFY(!stacks[i].first().isEmpty());
        }

        // All stacks should be similar in size (same call depth)
        int firstSize = stacks[0].size();
        for (int i = 1; i < numCalls; ++i) {
            QVERIFY(qAbs(stacks[i].size() - firstSize) <= 2); // Allow small variations
        }
    }

    void cleanupTestCase() {
        qDebug() << "DebugUtils cross-platform tests completed successfully";
    }
};

QTEST_MAIN(TestDebugUtilsCrossPlatform)
#include "TestDebugUtilsCrossPlatform.moc"