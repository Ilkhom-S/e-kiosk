// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Log.h>

class TestQFileLogger : public QObject {
  Q_OBJECT

private slots:
  void initTestCase() {
    QString tmp = QDir::tempPath() + QDir::separator() + "ekiosk_test_log.txt";
    QFile::remove(tmp);
    QVERIFY(ek::Log::init(tmp, ek::LogLevel::Debug, 256, 3));
  }

  void writeAndRotate() {
    // Write many small messages to trigger rotation
    for (int i = 0; i < 200; ++i) {
      ek::Log::debug(QStringLiteral("Test message %1").arg(i));
    }
    QString tmp = QDir::tempPath() + QDir::separator() + "ekiosk_test_log.txt";
    QVERIFY(QFile::exists(tmp));

    // Ensure at least one rotated file exists
    bool rotated = false;
    for (int i = 1; i <= 3; ++i) {
      if (QFile::exists(tmp + "." + QString::number(i))) {
        rotated = true;
        break;
      }
    }
    QVERIFY(rotated);
  }
};

#include "TestQFileLogger.moc"
