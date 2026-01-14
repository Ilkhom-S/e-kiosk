// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

class TestQFileLogger : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        // Test initialization of file logger
        ILog *log = ILog::getInstance("TestQFileLogger", LogType::File);
        QVERIFY(log != nullptr);
        log->setDestination("test_log");
        log->setLevel(LogLevel::Debug);
    }

    void writeMessages() {
        ILog *log = ILog::getInstance("TestQFileLogger");
        QVERIFY(log != nullptr);

        // Write some debug messages
        for (int i = 0; i < 10; ++i) {
            log->write(LogLevel::Debug, QStringLiteral("Test debug message %1").arg(i));
        }

        // Check if log file exists in the expected location
        QString appDir = QCoreApplication::applicationDirPath();
        QString expectedPath = appDir + "/logs/" + QDate::currentDate().toString("yyyy.MM.dd ") + "test_log.log";
        QVERIFY(QFile::exists(expectedPath));
    }

    void testLogLevels() {
        ILog *log = ILog::getInstance("TestQFileLogger");
        QVERIFY(log != nullptr);

        // Test different log levels
        log->write(LogLevel::Normal, "Info message");
        log->write(LogLevel::Warning, "Warning message");
        log->write(LogLevel::Error, "Error message");

        // Debug should be written since level is Debug
        log->write(LogLevel::Debug, "Debug message");

        // Trace should not be written if level is Debug
        log->write(LogLevel::Trace, "Trace message");
    }
};

QTEST_MAIN(TestQFileLogger)

#include "TestQFileLogger.moc"
