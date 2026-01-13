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
#include <Common/ILog.h>

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

  // Initialize logger
  m_log = ILog::getInstance(aName.isEmpty() ? "BasicApplication" : aName, LogType::File);
  if (m_log) {
    m_log->setDestination(aName.isEmpty() ? "basic_app" : aName.toLower());
    m_log->setLevel(LogLevel::Normal); // Default to Normal level
  }

  // install Qt message handler that uses our logger
  qInstallMessageHandler(messageHandler);

  // Register singleton instance pointer
  setInstance();

  // Create SingleApplication for single-instance (freestanding mode)
  // Allow secondary instances so helper/test processes can run and report
  // state. The main application still exits early if not primary (see
  // apps/kiosk/main.cpp).
  m_singleApp.reset(new SingleApplication(
      aArgumentCount, aArguments, true)); // true = allow secondary instances
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

  // Map Qt message types to our log levels
  LogLevel::Enum level;
  switch (type) {
    case QtDebugMsg:
      level = LogLevel::Debug;
      break;
    case QtInfoMsg:
    case QtWarningMsg:
      level = LogLevel::Warning;
      break;
    case QtCriticalMsg:
    case QtFatalMsg:
      level = LogLevel::Error;
      break;
    default:
      level = LogLevel::Normal;
      break;
  }

  // Get the application logger if available
  BasicApplication *app = BasicApplication::getInstance();
  if (app && app->getLog()) {
    app->getLog()->write(level, msg);
  } else {
    // Fallback to console output if no logger is available
    fprintf(stderr, "%s\n", qPrintable(msg));
  }

  // For fatal messages, abort
  if (type == QtFatalMsg) {
    abort();
  }
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

  // Note: Message handler is already installed in constructor
}
