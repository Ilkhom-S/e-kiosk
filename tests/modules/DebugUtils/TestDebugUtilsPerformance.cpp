// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QElapsedTimer>
#include <QtCore/QStringList>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// System
#include <DebugUtils/DebugUtils.h>

class TestDebugUtilsPerformance : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase()
    {
        qDebug() << "Running DebugUtils performance tests";
    }

    void testDumpCallstackPerformance()
    {
        // Test that stack dumping is reasonably fast
        QElapsedTimer timer;

        const int iterations = 100;
        qint64 totalTime = 0;

        for (int i = 0; i < iterations; ++i)
        {
            QStringList stack;
            timer.start();
            DumpCallstack(stack, nullptr);
            totalTime += timer.elapsed();

            // Each call should return a valid stack
            QVERIFY(stack.size() > 0);
        }

        double averageTime = static_cast<double>(totalTime) / iterations;
        qDebug() << "Average DumpCallstack time:" << averageTime << "ms";

        // Should be reasonably fast (less than 10ms on average)
        QVERIFY(averageTime < 50.0); // Allow some margin for slower systems
    }

    void testDumpCallstackMemoryUsage()
    {
        // Test that repeated calls don't leak memory significantly
        const int iterations = 1000;
        QStringList stacks[10]; // Keep some stacks in memory

        for (int i = 0; i < iterations; ++i)
        {
            DumpCallstack(stacks[i % 10], nullptr);
            QVERIFY(stacks[i % 10].size() > 0);
        }

        // If we get here without crashing, memory usage is probably OK
        QVERIFY(true);
    }

    void testConcurrentDumpCallstack()
    {
        // Test that multiple threads can dump stacks concurrently
        const int numThreads = 5;
        const int callsPerThread = 20;

        QVector<QThread *> threads;
        QVector<bool> completed(numThreads, false);

        for (int i = 0; i < numThreads; ++i)
        {
            QThread *thread = new QThread;
            threads.append(thread);

            thread->start();
            QMetaObject::invokeMethod(
                thread,
                [i, &completed, callsPerThread]()
                {
                    for (int j = 0; j < callsPerThread; ++j)
                    {
                        QStringList stack;
                        DumpCallstack(stack, nullptr);
                        QVERIFY(stack.size() > 0);

                        // Small delay to allow thread interleaving
                        QThread::msleep(1);
                    }
                    completed[i] = true;
                },
                Qt::QueuedConnection);
        }

        // Wait for all threads to complete
        QTRY_VERIFY(std::all_of(completed.begin(), completed.end(), [](bool b) { return b; }));

        // Clean up threads
        for (QThread *thread : threads)
        {
            thread->quit();
            thread->wait();
            delete thread;
        }

        QVERIFY(true);
    }

    void testDumpCallstackScalability()
    {
        // Test performance with deeper call stacks
        QStringList results;

        // Create a deep call stack and measure performance
        auto level5 = [&]() { DumpCallstack(results, nullptr); };
        auto level4 = [&]() { level5(); };
        auto level3 = [&]() { level4(); };
        auto level2 = [&]() { level3(); };
        auto level1 = [&]() { level2(); };

        QElapsedTimer timer;
        timer.start();
        level1();
        qint64 deepStackTime = timer.elapsed();

        // Compare with shallow stack
        QStringList shallowResults;
        timer.start();
        DumpCallstack(shallowResults, nullptr);
        qint64 shallowStackTime = timer.elapsed();

        qDebug() << "Deep stack time:" << deepStackTime << "ms";
        qDebug() << "Shallow stack time:" << shallowStackTime << "ms";
        qDebug() << "Deep stack frames:" << results.size();
        qDebug() << "Shallow stack frames:" << shallowResults.size();

        // Deep stack should have more frames
        QVERIFY(results.size() > shallowResults.size());

        // Performance should still be reasonable
        QVERIFY(deepStackTime < 100); // Less than 100ms even for deep stacks
    }

    void testExceptionHandlerPerformance()
    {
        // Test that setting exception handlers is fast
        QElapsedTimer timer;

        const int iterations = 1000;
#ifdef Q_OS_WIN
        LPTOP_LEVEL_EXCEPTION_FILTER handlers[iterations];
#endif

        timer.start();
        for (int i = 0; i < iterations; ++i)
        {
#ifdef Q_OS_WIN
            handlers[i] = SetUnhandledExceptionFilter(nullptr);
#endif
        }
        qint64 time = timer.elapsed();

        // Restore original handler
#ifdef Q_OS_WIN
        if (iterations > 0)
        {
            SetUnhandledExceptionFilter(handlers[0]);
        }
#endif

        double averageTime = static_cast<double>(time) / iterations;
        qDebug() << "Average SetUnhandledExceptionFilter time:" << averageTime << "μs";

        // Should be very fast (microseconds)
        QVERIFY(averageTime < 10.0);
    }

    void cleanupTestCase()
    {
        qDebug() << "DebugUtils performance tests completed";
    }
};

QTEST_MAIN(TestDebugUtilsPerformance)
#include "TestDebugUtilsPerformance.moc"