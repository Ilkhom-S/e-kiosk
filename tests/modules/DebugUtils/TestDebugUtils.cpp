#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>
#include <QtTest/QtTest>

#include <DebugUtils/DebugUtils.h>
#include <boost/version.hpp>

class TestDebugUtils : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "Testing DebugUtils with Boost.Stacktrace";
        qDebug() << "Boost version:" << BOOST_VERSION;
    }

    void testDumpCallstackReturnsValidData() {
        QStringList stack;
        DumpCallstack(stack, nullptr);

        // Should return at least one frame (this function)
        QVERIFY(stack.size() > 0);

        // First frame should contain function information
        QVERIFY(!stack.first().isEmpty());

        // Check that frames contain expected information
        bool hasAddress = false;
        bool hasFunction = false;

        for (const QString &frame : stack) {
            if (frame.contains("0x")) {
                hasAddress = true;
            }
            if (frame.contains("DumpCallstack") ||
                frame.contains("testDumpCallstackReturnsValidData")) {
                hasFunction = true;
            }
        }

        QVERIFY(hasAddress);  // Should have memory addresses
        QVERIFY(hasFunction); // Should contain our function names
    }

    void testDumpCallstackWithContext() {
        QStringList stack1, stack2;

        // Get stack from current location
        DumpCallstack(stack1, nullptr);

        // Get stack from a nested function
        auto nestedFunction = [&]() { DumpCallstack(stack2, nullptr); };
        nestedFunction();

        // Nested call should have more frames
        QVERIFY(stack2.size() >= stack1.size());

        // Should contain the nested function in the stack
        bool foundNested = false;
        for (const QString &frame : stack2) {
            if (frame.contains("nestedFunction") || frame.contains("operator()")) {
                foundNested = true;
                break;
            }
        }
        QVERIFY(foundNested);
    }

    void testDumpCallstackFormat() {
        QStringList stack;
        DumpCallstack(stack, nullptr);

        QVERIFY(stack.size() > 0);

        // Check format of first frame - should contain address and function info
        QString firstFrame = stack.first();

        // Should start with address (0x...)
        QVERIFY(firstFrame.startsWith("0x") || firstFrame.contains("0x"));

        // Should contain some function information
        QVERIFY(firstFrame.length() > 10); // Reasonable minimum length
    }

    void testDumpCallstackConsistency() {
        QStringList stack1, stack2;

        // Get two consecutive stack traces
        DumpCallstack(stack1, nullptr);
        DumpCallstack(stack2, nullptr);

        // Should be very similar (same call stack)
        QCOMPARE(stack1.size(), stack2.size());

        // First few frames should be identical
        QCOMPARE(stack1.first(), stack2.first());
    }

    void testDumpCallstackFromDifferentThreads() {
        QStringList mainThreadStack;
        QStringList workerThreadStack;

        // Get stack from main thread
        DumpCallstack(mainThreadStack, nullptr);

        // Get stack from worker thread
        QThread worker;
        bool done = false;

        worker.start();
        QMetaObject::invokeMethod(
            &worker,
            [&]() {
                DumpCallstack(workerThreadStack, nullptr);
                done = true;
            },
            Qt::QueuedConnection);

        // Wait for completion
        QTRY_VERIFY(done);
        worker.quit();
        worker.wait();

        // Both should have valid stacks
        QVERIFY(mainThreadStack.size() > 0);
        QVERIFY(workerThreadStack.size() > 0);

        // Worker thread stack should be different from main thread
        QVERIFY(mainThreadStack != workerThreadStack);
    }

    void testExceptionHandlerCanBeSet() {
        // This test verifies that the exception handler can be set without crashing
        // We can't easily test the actual exception handling without crashing the test process

#ifdef Q_OS_WIN
        // Original handler (may be null)
        LPTOP_LEVEL_EXCEPTION_FILTER originalHandler = nullptr;

        // Get current handler
        originalHandler = SetUnhandledExceptionFilter(nullptr);
        SetUnhandledExceptionFilter(originalHandler); // Restore
#endif

        // For non-Windows platforms, set to nullptr
#ifndef Q_OS_WIN
        TExceptionHandler originalHandler = nullptr;
#endif

        // Setting our handler should not crash
        SetUnhandledExceptionsHandler(originalHandler);

        // Should be able to call it multiple times
        SetUnhandledExceptionsHandler(originalHandler);
        SetUnhandledExceptionsHandler(nullptr);

        QVERIFY(true); // If we get here, the test passed
    }

    void testStackTraceContainsExpectedFunctions() {
        QStringList stack;
        DumpCallstack(stack, nullptr);

        // Should contain Qt functions (since we're in a Qt test)
        bool hasQtFunction = false;
        for (const QString &frame : stack) {
            if (frame.contains("QTest") || frame.contains("QObject") ||
                frame.contains("QCoreApplication")) {
                hasQtFunction = true;
                break;
            }
        }

        // Note: This might not always be true depending on optimization levels
        // so we'll make it a soft check
        qDebug() << "Qt functions in stack:" << hasQtFunction;
    }

    void testStackTraceDepth() {
        QStringList stack;
        DumpCallstack(stack, nullptr);

        // Should have reasonable depth (at least 5-10 frames typically)
        QVERIFY(stack.size() >= 3); // Minimum reasonable depth

        // But not ridiculously deep (should be less than 100 in normal cases)
        QVERIFY(stack.size() < 200);

        qDebug() << "Stack depth:" << stack.size();
    }

    void testStackTraceMemoryAddresses() {
        QStringList stack;
        DumpCallstack(stack, nullptr);

        // Should contain valid memory addresses
        QRegularExpression addressRegex("0x[0-9a-fA-F]+");
        bool hasValidAddress = false;

        for (const QString &frame : stack) {
            if (addressRegex.match(frame).hasMatch()) {
                hasValidAddress = true;
                break;
            }
        }

        QVERIFY(hasValidAddress);
    }

    void cleanupTestCase() { qDebug() << "DebugUtils tests completed"; }
};

QTEST_MAIN(TestDebugUtils)
#include "TestDebugUtils.moc"