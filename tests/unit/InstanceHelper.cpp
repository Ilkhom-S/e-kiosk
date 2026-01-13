// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  BasicApplication basic(QString(), QString(), argc, argv);

  QTextStream out(stdout);

  QStringList args;
  for (int i = 1; i < argc; ++i)
    args << QString::fromUtf8(argv[i]);

  bool primary = basic.isPrimaryInstance();
  out << (primary ? "1" : "0") << Qt::flush;

  // If this process is not the primary instance, exit immediately
  if (!primary) {
    return 0;
  }

  // Act as server: wait for stop-file to be created, or timeout after 5s
  QString stopFile = QDir::tempPath() + QDir::separator() +
                     QStringLiteral("instance-stop-") +
                     QString::number(QCoreApplication::applicationPid());
  const int maxMs = 5000;
  int waited = 0;
  while (waited < maxMs && !QFile::exists(stopFile)) {
    QThread::msleep(50);
    waited += 50;
  }
  if (QFile::exists(stopFile))
    QFile::remove(stopFile);

  return 0;
}
