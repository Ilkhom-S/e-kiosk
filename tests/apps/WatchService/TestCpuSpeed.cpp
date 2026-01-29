/* @file Test for CPUSpeed function. */

// STL
#include <iostream>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtDebug>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// Project
#include "CpuSpeed.h"

class TestCpuSpeed : public QObject
{
    Q_OBJECT

  private slots:
    void testCpuSpeedReturnsValidValue()
    {
        // Test that CPUSpeed returns a reasonable value
        unsigned speed = CPUSpeed();
        std::cout << "CPU Speed (valid value test): " << speed << " MHz" << std::endl;
        qDebug() << "CPU Speed (valid value test):" << speed << "MHz";

        // CPU speed should be between 100 MHz and 10 GHz (reasonable range)
        QVERIFY(speed >= 100);
        QVERIFY(speed <= 10000);
    }

    void testCpuSpeedIsConsistent()
    {
        // Test that CPUSpeed returns the same value on multiple calls
        unsigned speed1 = CPUSpeed();
        unsigned speed2 = CPUSpeed();
        std::cout << "CPU Speed consistency test - speed1: " << speed1 << " MHz, speed2: " << speed2 << " MHz"
                  << std::endl;
        qDebug() << "CPU Speed consistency test - speed1:" << speed1 << "MHz, speed2:" << speed2 << "MHz";

        QCOMPARE(speed1, speed2);
    }

    void testCpuSpeedAboveSlowPCTreshold()
    {
        // Test that CPU speed is above the slow PC threshold (1400 MHz)
        // This ensures the system is not considered "slow"
        unsigned speed = CPUSpeed();
        std::cout << "CPU Speed (slow PC threshold test): " << speed << " MHz (threshold: 1400 MHz)" << std::endl;
        qDebug() << "CPU Speed (slow PC threshold test):" << speed << "MHz (threshold: 1400 MHz)";

        // The threshold in WatchService.h is 1400 MHz
        QVERIFY(speed >= 1400);
    }
};

//----------------------------------------------------------------------------
QTEST_MAIN(TestCpuSpeed)
#include "TestCpuSpeed.moc"