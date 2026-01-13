// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>

// Project
#include <SingleApplication>

static void messageHandler(QtMsgType type, const QMessageLogContext &context,
                           const QString &msg);

BasicApplication::BasicApplication(const QString &aName,
                                   const QString &aVersion, int aArgumentCount,
                                   char **aArguments)
    : m_name(aName), m_version(aVersion) {
  // Parse provided argv for quick checks (e.g., 'test')
  QStringList args;
  for (int i = 0; i < aArgumentCount; ++i) {
    args << QString::fromUtf8(aArguments[i]);
  }
  // detect test mode from args first, then environment
  m_testMode = args.contains(QStringLiteral("test")) ||
               qEnvironmentVariableIsSet("EKIOSK_TEST_MODE") ||
               qEnvironmentVariableIsSet("TEST_MODE");

  // install simple logger
  qInstallMessageHandler(messageHandler);

  // Create SingleApplication for single-instance (freestanding mode)
  m_singleApp.reset(new SingleApplication(
      aArgumentCount, aArguments, false)); // false = no secondary instances
}

BasicApplication::~BasicApplication() = default;

// BasicApplication core implementations

BasicApplication *BasicApplication::s_instance = nullptr;

void BasicApplication::setInstance() { s_instance = this; }

BasicApplication *BasicApplication::getInstance() { return s_instance; }

QString BasicApplication::getName() const {
  if (!m_name.isEmpty())
    return m_name;
  return QCoreApplication::applicationName();
}

QString BasicApplication::getVersion() const {
  if (!m_version.isEmpty())
    return m_version;
  return QCoreApplication::applicationVersion();
}

QString BasicApplication::getFileName() const {
  return QFileInfo(QCoreApplication::applicationFilePath()).fileName();
}

QString BasicApplication::getOSVersion() const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
  return QSysInfo::prettyProductName();
#else
  return QStringLiteral("Unknown OS");
#endif
}

QString BasicApplication::getWorkingDirectory() const {
  // Check settings for override
  if (m_settings) {
    QString wd = m_settings->value("General/WorkingDirectory").toString();
    if (!wd.isEmpty())
      return wd;
  }
  return QCoreApplication::applicationDirPath();
}

QSettings &BasicApplication::getSettings() const {
  if (!m_settings) {
    QString dir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    QString file = dir + QDir::separator() + getName() + ".ini";
    m_settings.reset(new QSettings(file, QSettings::IniFormat));
  }
  return *m_settings;
}

ILog *BasicApplication::getLog() const { return m_log; }

void messageHandler(QtMsgType type, const QMessageLogContext &context,
                    const QString &msg) {
  Q_UNUSED(context);
  const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
  QString txt = QString("%1: %2\n").arg(timestamp, msg);

  QString dir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(dir);
  QFile f(dir + QDir::separator() + "ekiosk.log");
  if (f.open(QIODevice::Append | QIODevice::Text)) {
    f.write(txt.toUtf8());
    f.close();
  }

  // Also output to console
  fprintf(stderr, "%s", qPrintable(txt));
}

bool BasicApplication::isTestMode() const { return m_testMode; }

bool BasicApplication::isPrimaryInstance() const {
  return m_singleApp->isPrimary();
}

bool BasicApplication::startDetachedProcess(const QString &program,
                                            const QStringList &args) {
  return QProcess::startDetached(program, args);
}

void BasicApplication::detectTestMode() {
  // We can detect test mode either from provided arguments, which are not
  // available in this lightweight BasicApplication, or from environment vars
  m_testMode = qEnvironmentVariableIsSet("EKIOSK_TEST_MODE") ||
               qEnvironmentVariableIsSet("TEST_MODE");

  // install simple logger
  qInstallMessageHandler(messageHandler);
}
