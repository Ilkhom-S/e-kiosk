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
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>
#include <Common/ILog.h>
#include <Common/SafeApplication.h>

// System
#include <DebugUtils/DebugUtils.h>
#include <SysUtils/ISysUtils.h>

// Project
#include <singleapplication.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <comutil.h>
#include <comdef.h>
#endif

static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#ifdef Q_OS_WIN
// Note: ExceptionFilter is Windows-specific, using SEH to dump callstack on
// unhandled exceptions. Adopted from TerminalClient repo for debugging crashes.
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS *aException)
{
    QStringList stack;
    DumpCallstack(stack, aException->ContextRecord);

    qCritical() << "Exited due to unknown exception. Callstack:\n" + stack.join("\n");

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

BasicApplication::BasicApplication(const QString &aName, const QString &aVersion, int aArgumentCount, char **aArguments,
                                   bool aUseSingleApp)
    : m_name(aName), m_version(aVersion), m_argumentCount(aArgumentCount), m_arguments(aArguments),
      m_useSingleApp(aUseSingleApp)
{
    // Parse provided argv for quick checks (e.g., 'test')
    QStringList args;
    for (int i = 0; i < aArgumentCount; ++i)
    {
        args << QString::fromUtf8(aArguments[i]);
    }
    // detect test mode from args first, then environment
    m_testMode = args.contains(QStringLiteral("test")) || qEnvironmentVariableIsSet("EKIOSK_TEST_MODE") ||
                 qEnvironmentVariableIsSet("TEST_MODE");

    // Инициализируем настройки из .ini файла на основе пути к исполняемому
    // файлу
    QFileInfo info(QString::fromLocal8Bit(m_arguments[0]));
    QString settingsFilePath =
        QDir::toNativeSeparators(info.absolutePath() + QDir::separator() + info.completeBaseName() + ".ini");
    m_settings.reset(new QSettings(ISysUtils::rmBOM(settingsFilePath), QSettings::IniFormat));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined(Q_OS_WIN)
    m_settings->setIniCodec("UTF-8");
#endif

    // Устанавливаем рабочий каталог
    m_workingDirectory = info.absolutePath();
    if (m_settings->contains("common/working_directory"))
    {
        QString directory = m_settings->value("common/working_directory").toString();
        m_workingDirectory = QDir::toNativeSeparators(QDir::cleanPath(
            (QDir::isAbsolutePath(directory) ? "" : (info.absolutePath() + QDir::separator())) + directory));
    }

    // Register singleton instance pointer
    setInstance();

    // Выставим уровень логирования из user.ini
    QString userFilePath = m_workingDirectory + QDir::separator() +
                           m_settings->value("common/user_data_path").toString() + QDir::separator() + "user.ini";
    QSettings userSettings(ISysUtils::rmBOM(userFilePath), QSettings::IniFormat);
    if (userSettings.contains("log/level"))
    {
        int level = userSettings.value("log/level").toInt();
        if (level < LogLevel::Off)
            level = LogLevel::Off;
        else if (level > LogLevel::Max)
            level = LogLevel::Max;
        ILog::setGlobalLevel(static_cast<LogLevel::Enum>(level));
    }

    // Initialize logger
    m_log = ILog::getInstance(aName.isEmpty() ? "BasicApplication" : aName, LogType::File);
    if (m_log)
    {
        m_log->setDestination(aName.isEmpty() ? "basic_app" : aName.toLower());
        m_log->setLevel(LogLevel::Normal); // Default to Normal level
    }

    // install Qt message handler that uses our logger
    qInstallMessageHandler(messageHandler);

    // Create SingleApplication for single-instance (freestanding mode)
    // Allow secondary instances so helper/test processes can run and report
    // state. The main application still exits early if not primary (see
    // apps/kiosk/main.cpp).
    // Skip in test mode to avoid conflicts with test execution
    if (!m_testMode && m_useSingleApp)
    {
        m_singleApp.reset(new SingleApplication(aArgumentCount, aArguments, true)); // true = allow secondary instances
    }

    // Logging header will be written later after QApplication is created

#ifdef Q_OS_WIN
    // Инициализация COM Security для работы с WMI
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    HRESULT hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
                                      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    if (FAILED(hr))
    {
        LOG(m_log, LogLevel::Error,
            QString("CoInitializeSecurity failed: %1.")
                .arg(QString::fromWCharArray((const wchar_t *)_com_error(hr).ErrorMessage())));
    }
#endif
}

//---------------------------------------------------------------------------
// Выводит стандартный заголовок в лог (вызывается после создания QApplication).
void BasicApplication::writeLogHeader()
{
    if (m_log)
    {
        m_log->write(LogLevel::Normal, "**********************************************************");
        m_log->write(LogLevel::Normal, QString("Application: %1").arg(getName()));
        m_log->write(LogLevel::Normal, QString("File: %1").arg(getFileName()));
        m_log->write(LogLevel::Normal, QString("Version: %1").arg(getVersion()));
        m_log->write(LogLevel::Normal, QString("Operating system: %1").arg(getOSVersion()));
        m_log->write(LogLevel::Normal, "**********************************************************");
    }
}

BasicApplication::~BasicApplication() = default;

// BasicApplication core implementations

BasicApplication *BasicApplication::s_instance = nullptr;

//---------------------------------------------------------------------------
// Устанавливает экземпляр приложения.
void BasicApplication::setInstance()
{
    s_instance = this;
}

//---------------------------------------------------------------------------
// Возвращает экземпляр приложения.
BasicApplication *BasicApplication::getInstance()
{
    return s_instance;
}

//---------------------------------------------------------------------------
// Возвращает имя приложения.
QString BasicApplication::getName() const
{
    if (!m_name.isEmpty())
        return m_name;
    return QCoreApplication::applicationName();
}

//---------------------------------------------------------------------------
// Возвращает версию приложения.
QString BasicApplication::getVersion() const
{
    if (!m_version.isEmpty())
        return m_version;
    return QCoreApplication::applicationVersion();
}

//---------------------------------------------------------------------------
// Возвращает имя исполняемого файла.
QString BasicApplication::getFileName() const
{
    // If QApplication exists, use its applicationFilePath
    if (QCoreApplication::instance())
    {
        return QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    }
    else
    {
        // Fallback: use the executable path from arguments
        if (m_argumentCount > 0)
        {
            return QFileInfo(QString::fromLocal8Bit(m_arguments[0])).fileName();
        }
        return QString(); // Empty string if no arguments
    }
}

//---------------------------------------------------------------------------
// Возвращает тип/версию операционной системы.
QString BasicApplication::getOSVersion() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return ISysUtils::getOSVersionInfo();
#else
    return QStringLiteral("Unknown OS");
#endif
}

//---------------------------------------------------------------------------
// Возвращает рабочий каталог приложения (может быть задан в .ini файле).
QString BasicApplication::getWorkingDirectory() const
{
    return m_workingDirectory;
}

//---------------------------------------------------------------------------
// Возвращает настройки приложения
QSettings &BasicApplication::getSettings() const
{
    return *m_settings;
}

//---------------------------------------------------------------------------
// Возвращает лог приложения.
ILog *BasicApplication::getLog() const
{
    return m_log;
}

//---------------------------------------------------------------------------
// Обработчик сообщений Qt для перенаправления в лог.
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    // Map Qt message types to our log levels
    LogLevel::Enum level;
    switch (type)
    {
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
    if (app && app->getLog())
    {
        app->getLog()->write(level, msg);
    }
    else
    {
        // Fallback to console output if no logger is available
        fprintf(stderr, "%s\n", qPrintable(msg));
    }

    // For fatal messages, abort
    if (type == QtFatalMsg)
    {
        abort();
    }
}

//---------------------------------------------------------------------------
// Возвращает true, если приложение запущено в тестовом режиме.
bool BasicApplication::isTestMode() const
{
    return m_testMode;
}

//---------------------------------------------------------------------------
// Возвращает true, если это первичный экземпляр приложения.
bool BasicApplication::isPrimaryInstance() const
{
    return m_singleApp->isPrimary();
}

//---------------------------------------------------------------------------
// Запускает detached процесс.
bool BasicApplication::startDetachedProcess(const QString &program, const QStringList &args)
{
    return QProcess::startDetached(program, args);
}

//---------------------------------------------------------------------------
// Определяет тестовый режим.
void BasicApplication::detectTestMode()
{
    // We can detect test mode either from provided arguments, which are not
    // available in this lightweight BasicApplication, or from environment vars
    m_testMode = qEnvironmentVariableIsSet("EKIOSK_TEST_MODE") || qEnvironmentVariableIsSet("TEST_MODE");

    // Note: Message handler is already installed in constructor
}

#ifdef Q_OS_WIN
// Note: Exception handling in notify() is Windows-specific due to Structured
// Exception Handling (SEH) with __try/__except. This catches low-level
// exceptions (e.g., access violations) during Qt event processing. On other
// platforms (Linux/macOS), SEH is not available, and Qt uses different
// exception/signal handling, so this is omitted for cross-platform
// compatibility. Adopted from TerminalClient repo, where the codebase was
// Windows-only. TODO: Implement signal-based crash handling on Unix platforms
// for full agnosticism.
bool SafeQApplication::notify(QObject *aReceiver, QEvent *aEvent)
{
    __try
    {
        return reinterpret_cast<QApplication *>(this)->notify(aReceiver, aEvent);
    }
    __except (ExceptionFilter(GetExceptionInformation()))
    {
        abort();
    }
}
#else
// Cross-platform implementation for non-Windows platforms
// Note: SEH (__try/__except) is Windows-specific. On Unix platforms,
// Qt handles exceptions differently and we rely on Qt's built-in
// exception safety mechanisms.
bool SafeQApplication::notify(QObject *aReceiver, QEvent *aEvent)
{
    return QApplication::notify(aReceiver, aEvent);
}
#endif

//---------------------------------------------------------------------------
// Specialization for SingleApplication
template <>
BasicQtApplication<SingleApplication>::BasicQtApplication(const QString &aName, const QString &aVersion,
                                                          int &aArgumentCount, char **aArguments)
    : BasicApplication(aName, aVersion, aArgumentCount, aArguments,
                       false), // Don't create SingleApplication in BasicApplication
      mQtApplication(
          aArgumentCount, aArguments, true,
          SingleApplication::Mode::ExcludeAppPath) // allowSecondary = true, exclude app path to avoid conflicts
{
    mQtApplication.setApplicationName(aName);
    mQtApplication.setApplicationVersion(aVersion);

    QFileInfo fileInfo(mQtApplication.applicationFilePath());

    // Now that Qt application is created, write the log header
    writeLogHeader();
}
