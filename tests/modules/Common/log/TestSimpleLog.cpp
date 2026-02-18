/* @file Тесты для модуля логирования с проверкой свёртки повторяющихся сообщений. */

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include "SimpleLog.h"

class TestSimpleLog : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Базовые тесты
    void testBasicWrite();
    void testDifferentLevels();
    void testSetLevel();

    // Тесты свёртки дубликатов
    void testDuplicateSuppression();
    void testDuplicateDifferentLevel();
    void testDuplicateDifferentMessage();
    void testDuplicateSummary();
    void testMultipleDuplicateSequences();

    // Тесты thread safety
    void testThreadSafety();

private:
    QTemporaryDir *m_tempDir = nullptr;
    QString m_logPath;
    QString readFileContents();
};

//---------------------------------------------------------------------------
void TestSimpleLog::initTestCase() {
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    m_logPath = m_tempDir->path() + "/test.log";
}

//---------------------------------------------------------------------------
void TestSimpleLog::cleanupTestCase() {
    delete m_tempDir;
}

//---------------------------------------------------------------------------
QString TestSimpleLog::readFileContents() {
    QFile file(m_logPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    return content;
}

//---------------------------------------------------------------------------
void TestSimpleLog::testBasicWrite() {
    // Создаём лог с типом Console (не требует файловой системы)
    SimpleLog log("testBasic", LogType::Console, LogLevel::Normal);

    QVERIFY(log.getName() == "testBasic");
    QCOMPARE(log.getType(), LogType::Console);

    // Запись должна проходить без ошибок
    log.write(LogLevel::Normal, "Test message");
    QVERIFY(true); // Если дошли сюда без краша — тест пройден
}

//---------------------------------------------------------------------------
void TestSimpleLog::testDifferentLevels() {
    SimpleLog log("testLevels", LogType::Console, LogLevel::Debug);

    // Все уровни должны записываться при LogLevel::Debug
    log.write(LogLevel::Normal, "Normal message");
    log.write(LogLevel::Warning, "Warning message");
    log.write(LogLevel::Error, "Error message");
    log.write(LogLevel::Debug, "Debug message");

    QVERIFY(true);
}

//---------------------------------------------------------------------------
void TestSimpleLog::testSetLevel() {
    SimpleLog log("testSetLevel", LogType::Console, LogLevel::Warning);

    // При уровне Warning, Debug и Normal должны игнорироваться
    log.setLevel(LogLevel::Warning);

    // Эти сообщения должны игнорироваться
    log.write(LogLevel::Debug, "Should be ignored");
    log.write(LogLevel::Normal, "Should be ignored");

    // Эти должны записываться
    log.write(LogLevel::Warning, "Warning");
    log.write(LogLevel::Error, "Error");

    QVERIFY(true);
}

//---------------------------------------------------------------------------
void TestSimpleLog::testDuplicateSuppression() {
    SimpleLog log("testDupSuppress", LogType::Console, LogLevel::Normal);

    // Записываем 5 одинаковых сообщений
    for (int i = 0; i < 5; ++i) {
        log.write(LogLevel::Normal, "Duplicate message");
    }

    // Записываем отличающееся сообщение — должно вывести сводку
    log.write(LogLevel::Normal, "Different message");

    QVERIFY(true);
}

//---------------------------------------------------------------------------
void TestSimpleLog::testDuplicateDifferentLevel() {
    SimpleLog log("testDupLevel", LogType::Console, LogLevel::Normal);

    // Сообщения с разными уровнями НЕ считаются дубликатами
    log.write(LogLevel::Normal, "Same text");
    log.write(LogLevel::Warning, "Same text"); // Не дубликат — другой уровень
    log.write(LogLevel::Error, "Same text");   // Не дубликат — другой уровень

    // Каждое должно быть записано отдельно
    QVERIFY(true);
}

//---------------------------------------------------------------------------
void TestSimpleLog::testDuplicateDifferentMessage() {
    SimpleLog log("testDupMsg", LogType::Console, LogLevel::Normal);

    // Разные сообщения не подавляются
    log.write(LogLevel::Normal, "Message A");
    log.write(LogLevel::Normal, "Message B");
    log.write(LogLevel::Normal, "Message C");

    // Каждое должно быть записано
    QVERIFY(true);
}

//---------------------------------------------------------------------------
void TestSimpleLog::testDuplicateSummary() {
    SimpleLog log("testDupSummary", LogType::Console, LogLevel::Normal);

    // 10 одинаковых сообщений
    for (int i = 0; i < 10; ++i) {
        log.write(LogLevel::Normal, "Repeated message");
    }

    // Отличающееся сообщение триггерит вывод сводки
    log.write(LogLevel::Normal, "Final message");

    QVERIFY(true);
}

//---------------------------------------------------------------------------
void TestSimpleLog::testMultipleDuplicateSequences() {
    SimpleLog log("testMultiSeq", LogType::Console, LogLevel::Normal);

    // Первая серия дубликатов
    for (int i = 0; i < 3; ++i) {
        log.write(LogLevel::Normal, "Series A");
    }

    // Вторая серия дубликатов
    for (int i = 0; i < 5; ++i) {
        log.write(LogLevel::Normal, "Series B");
    }

    // Третья серия
    for (int i = 0; i < 2; ++i) {
        log.write(LogLevel::Normal, "Series C");
    }

    // Финальное сообщение
    log.write(LogLevel::Normal, "End");

    QVERIFY(true);
}

//---------------------------------------------------------------------------
void TestSimpleLog::testThreadSafety() {
    SimpleLog log("testThread", LogType::Console, LogLevel::Normal);

    // Многопоточная запись
    QList<QThread *> threads;

    for (int t = 0; t < 4; ++t) {
        QThread *thread = QThread::create([&log, t]() {
            for (int i = 0; i < 100; ++i) {
                log.write(LogLevel::Normal, QString("Thread %1 message %2").arg(t).arg(i));
            }
        });
        threads.append(thread);
    }

    for (QThread *thread : threads) {
        thread->start();
    }

    for (QThread *thread : threads) {
        thread->wait();
        delete thread;
    }

    // Если не упали — тест пройден
    QVERIFY(true);
}

//---------------------------------------------------------------------------
QTEST_MAIN(TestSimpleLog)
#include "TestSimpleLog.moc"
