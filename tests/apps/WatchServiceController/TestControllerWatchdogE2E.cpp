/* @file End-to-end test: controller -> watchdog (real process)
   Проверяет, что реальный watchdog-процесс корректно завершается при получении
   команды Exit от контроллера.
*/

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <QtTest/QtTest>

#include <WatchServiceClient/Constants.h>
#include <WatchServiceClient/IWatchServiceClient.h>

#include "../../..//apps/WatchServiceController/src/WatchServiceController.h"

class TestControllerWatchdogE2E : public QObject {
    Q_OBJECT

private slots:
    void test_watchservicecontroller_stopService_terminates_watchdog();
};

static QString watchdogExecutablePath() {
#ifdef Q_OS_MACOS
    return QCoreApplication::applicationDirPath() + "/../bin/watchdog.app/Contents/MacOS/watchdog";
#else
    return QCoreApplication::applicationDirPath() + "/../bin/watchdog";
#endif
}

void TestControllerWatchdogE2E::test_watchservicecontroller_stopService_terminates_watchdog() {
    const QString watchdogPath = watchdogExecutablePath();
    QVERIFY2(QFile::exists(watchdogPath),
             qPrintable(QString("watchdog not found: %1").arg(watchdogPath)));

    // Create a minimal config that disables autostart of modules so watchdog exits quickly
    const QString tmpCfgPath = QDir::temp().absoluteFilePath(
        QString("watchdog-test-%1.ini")
            .arg(QLatin1String(
                QCryptographicHash::hash(QByteArray::number(QDateTime::currentMSecsSinceEpoch()),
                                         QCryptographicHash::Md5)
                    .toHex())));

    QFile cfg(tmpCfgPath);
    QVERIFY(cfg.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&cfg);
    out << "[module_ekiosk]\n";
    out << "autostart = false\n";
    out << "file = " << watchdogPath << "\n";
    out << "\n[taboo]\napplications =\n";
    out.flush();
    cfg.close();

    QProcess watchdog;
    watchdog.setProcessChannelMode(QProcess::MergedChannels);
    QStringList args;
    args << "--config" << tmpCfgPath;
    watchdog.start(watchdogPath, args);

    // Ensure watchdog started
    QTRY_VERIFY_WITH_TIMEOUT(watchdog.state() == QProcess::Running, 5000);

    // Create controller and force its internal client to connect
    WatchServiceController controller;
    QMetaObject::invokeMethod(&controller, "onCheck", Qt::DirectConnection);

    // Give the controller/client some time to connect
    QTest::qWait(200); // short wait; connection logic uses QTRY in other tests where needed

    // Now simulate user clicking "Stop service"
    QSignalSpy finishedSpy(&watchdog, SIGNAL(finished(int, QProcess::ExitStatus)));

    QMetaObject::invokeMethod(&controller, "onStopServiceClicked", Qt::DirectConnection);

    // Expect watchdog to exit after receiving Exit command
    QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 1, 5000);

    // Cleanup
    if (watchdog.state() == QProcess::Running) {
        watchdog.kill();
        watchdog.waitForFinished(2000);
    }

    QFile::remove(tmpCfgPath);
}

QTEST_MAIN(TestControllerWatchdogE2E)
#include "TestControllerWatchdogE2E.moc"
