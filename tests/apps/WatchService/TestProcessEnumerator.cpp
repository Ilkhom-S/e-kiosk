/* @file Test for ProcessEnumerator class. */

// STL
#include <iostream>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtDebug>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// Project
#include "processenumerator.h"

class TestProcessEnumerator : public QObject
{
    Q_OBJECT

  private slots:
    void testProcessEnumeratorCanBeCreated()
    {
        // Test that ProcessEnumerator can be instantiated
        ProcessEnumerator enumerator;
        QVERIFY(&enumerator != nullptr);
    }

    void testProcessEnumeratorEnumeratesProcesses()
    {
        // Test that ProcessEnumerator can enumerate processes
        ProcessEnumerator enumerator;

        // Should have at least some processes (system processes)
        QVERIFY(enumerator.begin() != enumerator.end());

        // Count processes
        int processCount = 0;
        for (auto it = enumerator.begin(); it != enumerator.end(); ++it)
        {
            ++processCount;
            // Each process should have a valid PID (> 0)
            QVERIFY(it.value().pid > 0);
            // Each process should have a non-empty path
            QVERIFY(!it.value().path.isEmpty());
        }

        std::cout << "Process enumeration test: found " << processCount << " processes" << std::endl;
        qDebug() << "Process enumeration test: found" << processCount << "processes";

        // Should have at least 5 processes (very basic system)
        QVERIFY(processCount >= 5);
    }

    void testProcessEnumeratorContainsCurrentProcess()
    {
        // Test that the current process is in the enumeration
        ProcessEnumerator enumerator;

        // Get current process ID
        qint64 currentPid = QCoreApplication::applicationPid();

        // Check if current process is in the enumeration
        bool foundCurrentProcess = false;
        for (auto it = enumerator.begin(); it != enumerator.end(); ++it)
        {
            if (static_cast<qint64>(it.value().pid) == currentPid)
            {
                foundCurrentProcess = true;
                // Current process should have a valid path
                QVERIFY(!it.value().path.isEmpty());
                break;
            }
        }

        std::cout << "Current process test: PID " << currentPid << " found: " << (foundCurrentProcess ? "yes" : "no")
                  << std::endl;
        qDebug() << "Current process test: PID" << currentPid << "found:" << (foundCurrentProcess ? "yes" : "no");

        QVERIFY(foundCurrentProcess);
    }

    void testProcessEnumeratorKillInvalidProcess()
    {
        // Test that killing an invalid process returns false
        ProcessEnumerator enumerator;

        // Try to kill a non-existent process ID
        const ProcessEnumerator::PID invalidPid = 999999;
        quint32 errorCode = 0;

        bool result = enumerator.kill(invalidPid, errorCode);

        std::cout << "Invalid process kill test: result=" << (result ? "true" : "false") << ", error=" << errorCode
                  << std::endl;
        qDebug() << "Invalid process kill test: result=" << result << ", error=" << errorCode;

        // Should return false for invalid PID
        QVERIFY(!result);
        // Should have some error code
        QVERIFY(errorCode != 0);
    }

    void testProcessEnumeratorIteration()
    {
        // Test that iterator works correctly
        ProcessEnumerator enumerator;

        // Test forward iteration
        auto it = enumerator.begin();
        auto end = enumerator.end();

        if (it != end)
        {
            // First process should be valid
            QVERIFY(it.value().pid > 0);
            QVERIFY(!it.value().path.isEmpty());

            // Test increment
            ++it;
            if (it != end)
            {
                QVERIFY(it.value().pid > 0);
                QVERIFY(!it.value().path.isEmpty());
            }
        }
    }
};

//----------------------------------------------------------------------------

QTEST_MAIN(TestProcessEnumerator)
#include "TestProcessEnumerator.moc"