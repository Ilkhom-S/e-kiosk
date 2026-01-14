// Qt
#include <Common/QtHeadersBegin.h>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>

class TestBaseApplication : public QObject {
    Q_OBJECT

  private:
    // Helper to construct the path to test helper binaries in the same folder
    QString programPath(const QString &name) const {
        QString exe = name;
#if defined(Q_OS_WIN)
        exe += QStringLiteral(".exe");
#endif
        return QDir(QCoreApplication::applicationDirPath()).filePath(exe);
    }

  private slots:
    void detectTestMode_arg() {
        QProcess proc;
        proc.setProgram("CheckTestMode");
        proc.setArguments({"test"});
        proc.start();
        QVERIFY(proc.waitForFinished(2000));
        QCOMPARE(proc.readAllStandardOutput().trimmed(), "1");
    }

    void detectTestMode_env() {
        QProcess proc;
        proc.setProgram(programPath("CheckTestMode"));
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("EKIOSK_TEST_MODE", "1");
        proc.setProcessEnvironment(env);
        proc.start();
        QVERIFY(proc.waitForFinished(2000));
        QCOMPARE(proc.readAllStandardOutput().trimmed(), "1");
    }

    void singleInstance_prevents_second() {
        // Start the server helper that stays alive and reports primary instance
        QProcess server;
        server.setProgram(programPath("InstanceHelper"));
        server.start();
        QVERIFY(server.waitForReadyRead(2000));
        QCOMPARE(server.readAllStandardOutput().trimmed(), "1");

        // Start a second process which should report not-primary (0) and exit
        // immediately
        QProcess client;
        client.setProgram(programPath("InstanceHelper"));
        client.setArguments({"once"});
        client.start();
        QVERIFY(client.waitForStarted(2000));
        QVERIFY(client.waitForFinished(2000));
        QCOMPARE(client.readAllStandardOutput().trimmed(), "0");

        // Stop the server by creating stop file
        QString stopFile = QDir::tempPath() + QDir::separator() + QStringLiteral("instance-stop-") +
                           QString::number(server.processId());
        QFile f(stopFile);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.close();

        QVERIFY(server.waitForFinished(2000));
    }
};

#include "TestBaseApplication.moc"

int main(int argc, char *argv[]) {
    // Create a real Qt application and BasicApplication so tests exercise the
    // same environment
    QApplication qtApp(argc, argv);
    BasicApplication app(QString(), QString(), argc, argv);
    TestBaseApplication tc;
    return QTest::qExec(&tc, argc, argv);
}
