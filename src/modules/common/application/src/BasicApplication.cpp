#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>

#include <Common/BasicApplication.h>
#include <Common/ILog.h>
#include <Common/SafeApplication.h>

#include <DebugUtils/DebugUtils.h>
#include <SysUtils/ISysUtils.h>
#include <singleapplication.h>

#ifdef Q_OS_WIN
#include <comdef.h>
#include <comutil.h>
#include <windows.h>
#endif

static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#ifdef Q_OS_WIN
// Note: ExceptionFilter is Windows-specific, using SEH to dump callstack on
// unhandled exceptions. Adopted from TerminalClient repo for debugging crashes.
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS *aException) {
    QStringList stack;
    DumpCallstack(stack, aException->ContextRecord);

    qCritical() << "Exited due to unknown exception. Callstack:\n" + stack.join("\n");

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

BasicApplication::BasicApplication(const QString &aName,
                                   const QString &aVersion,
                                   int aArgumentCount,
                                   char **aArguments)
    : mName(aName), mVersion(aVersion), mArgumentCount(aArgumentCount), mArguments(aArguments) {
    // Parse provided argv for quick checks (e.g., 'test')
    QStringList args;
    for (int i = 0; i < aArgumentCount; ++i) {
        args << QString::fromUtf8(aArguments[i]);
    }
    // detect test mode from args first, then environment
    mTestMode = args.contains(QStringLiteral("test")) ||
                qEnvironmentVariableIsSet("EKIOSK_TEST_MODE") ||
                qEnvironmentVariableIsSet("TEST_MODE");

    // Инициализируем настройки из .ini файла на основе пути к исполняемому
    // файлу
    QFileInfo info(QString::fromLocal8Bit(mArguments[0]));
    QString basePath = info.absolutePath();

    // На macOS для app bundle нужно использовать директорию, содержащую .app,
    // а не внутреннюю структуру bundle
#ifdef Q_OS_MAC
    if (basePath.contains(".app/Contents/MacOS")) {
        // Убираем .app/Contents/MacOS из пути, чтобы получить директорию с .app
        int appBundleIndex = basePath.indexOf(".app/Contents/MacOS");
        if (appBundleIndex != -1) {
            QString appPath = basePath.left(appBundleIndex + 4); // +4 для ".app"
            // Получаем родительскую директорию .app
            QDir appDir(appPath);
            if (appDir.cdUp()) // Переходим в родительскую директорию
            {
                basePath = appDir.absolutePath();
            } else {
                basePath = appDir.absolutePath(); // fallback
            }
        }
    }
#endif

    QString settingsFilePath =
        QDir::toNativeSeparators(basePath + QDir::separator() + info.completeBaseName() + ".ini");

    mSettings.reset(new QSettings(ISysUtils::rmBOM(settingsFilePath), QSettings::IniFormat));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined(Q_OS_WIN)
    mSettings->setIniCodec("UTF-8");
#endif

    // Устанавливаем рабочий каталог
    mWorkingDirectory = basePath;
    if (mSettings->contains("common/working_directory")) {
        QString directory = mSettings->value("common/working_directory").toString();
        mWorkingDirectory = QDir::toNativeSeparators(QDir::cleanPath(
            (QDir::isAbsolutePath(directory) ? "" : (basePath + QDir::separator())) + directory));
    }

    // Register singleton instance pointer
    setInstance();

    // Выставим уровень логирования из user.ini
    QString userFilePath = mWorkingDirectory + QDir::separator() +
                           mSettings->value("common/user_data_path").toString() +
                           QDir::separator() + "user.ini";
    QSettings userSettings(ISysUtils::rmBOM(userFilePath), QSettings::IniFormat);
    if (userSettings.contains("log/level")) {
        int level = userSettings.value("log/level").toInt();
        if (level < LogLevel::Off)
            level = LogLevel::Off;
        else if (level > LogLevel::Max)
            level = LogLevel::Max;
        ILog::setGlobalLevel(static_cast<LogLevel::Enum>(level));
    }

    // Initialize logger
#ifdef QT_DEBUG
    mLog = ILog::getInstance(aName.isEmpty() ? "BasicApplication" : aName, LogType::Console);
#else
    mLog = ILog::getInstance(aName.isEmpty() ? "BasicApplication" : aName, LogType::File);
#endif
    if (mLog) {
        mLog->setDestination(aName.isEmpty() ? "basic_app" : aName.toLower());
        mLog->setLevel(LogLevel::Normal); // Default to Normal level
    }

    // install Qt message handler that uses our logger
    qInstallMessageHandler(messageHandler);

    // Logging header will be written later after QApplication is created

#ifdef Q_OS_WIN
    // Инициализация COM Security для работы с WMI
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    HRESULT hr = CoInitializeSecurity(nullptr,
                                      -1,
                                      nullptr,
                                      nullptr,
                                      RPC_C_AUTHN_LEVEL_DEFAULT,
                                      RPC_C_IMP_LEVEL_IMPERSONATE,
                                      nullptr,
                                      EOAC_NONE,
                                      nullptr);
    if (FAILED(hr)) {
        LOG(mLog,
            LogLevel::Error,
            QString("CoInitializeSecurity failed: %1.")
                .arg(QString::fromWCharArray((const wchar_t *)_com_error(hr).ErrorMessage())));
    }
#endif
}

//---------------------------------------------------------------------------
// Выводит стандартный заголовок в лог (вызывается после создания QApplication).
void BasicApplication::writeLogHeader() {
    if (mLog) {
        mLog->write(LogLevel::Normal, "**********************************************************");
        mLog->write(LogLevel::Normal, QString("Application: %1").arg(getName()));
        mLog->write(LogLevel::Normal, QString("File: %1").arg(getFileName()));
        mLog->write(LogLevel::Normal, QString("Version: %1").arg(getVersion()));
        mLog->write(LogLevel::Normal, QString("Operating system: %1").arg(getOSVersion()));
        mLog->write(LogLevel::Normal, "**********************************************************");
    }
}

BasicApplication::~BasicApplication() = default;

// BasicApplication core implementations

BasicApplication *BasicApplication::mInstance = nullptr;

//---------------------------------------------------------------------------
// Устанавливает экземпляр приложения.
void BasicApplication::setInstance() {
    mInstance = this;
}

//---------------------------------------------------------------------------
// Возвращает экземпляр приложения.
BasicApplication *BasicApplication::getInstance() {
    return mInstance;
}

//---------------------------------------------------------------------------
// Возвращает имя приложения.
QString BasicApplication::getName() const {
    if (!mName.isEmpty())
        return mName;
    return QCoreApplication::applicationName();
}

//---------------------------------------------------------------------------
// Возвращает версию приложения.
QString BasicApplication::getVersion() const {
    if (!mVersion.isEmpty())
        return mVersion;
    return QCoreApplication::applicationVersion();
}

//---------------------------------------------------------------------------
// Возвращает имя исполняемого файла.
QString BasicApplication::getFileName() const {
    // If QApplication exists, use its applicationFilePath
    if (QCoreApplication::instance()) {
        return QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    } else {
        // Fallback: use the executable path from arguments
        if (mArgumentCount > 0) {
            return QFileInfo(QString::fromLocal8Bit(mArguments[0])).fileName();
        }
        return QString(); // Empty string if no arguments
    }
}

//---------------------------------------------------------------------------
// Возвращает тип/версию операционной системы.
QString BasicApplication::getOSVersion() const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return ISysUtils::getOSVersionInfo();
#else
    return QStringLiteral("Unknown OS");
#endif
}

//---------------------------------------------------------------------------
// Возвращает рабочий каталог приложения (может быть задан в .ini файле).
QString BasicApplication::getWorkingDirectory() const {
    return mWorkingDirectory;
}

//---------------------------------------------------------------------------
// Возвращает настройки приложения
QSettings &BasicApplication::getSettings() const {
    return *mSettings;
}

//---------------------------------------------------------------------------
// Возвращает лог приложения.
ILog *BasicApplication::getLog() const {
    return mLog;
}

//---------------------------------------------------------------------------
// Обработчик сообщений Qt для перенаправления в лог.
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
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

//---------------------------------------------------------------------------
// Возвращает true, если приложение запущено в тестовом режиме.
bool BasicApplication::isTestMode() const {
    return mTestMode;
}

//---------------------------------------------------------------------------
// Запускает detached процесс.
bool BasicApplication::startDetachedProcess(const QString &program, const QStringList &args) {
    return QProcess::startDetached(program, args);
}

//---------------------------------------------------------------------------
// Определяет тестовый режим.
void BasicApplication::detectTestMode() {
    // We can detect test mode either from provided arguments, which are not
    // available in this lightweight BasicApplication, or from environment vars
    mTestMode =
        qEnvironmentVariableIsSet("EKIOSK_TEST_MODE") || qEnvironmentVariableIsSet("TEST_MODE");

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
bool SafeQApplication::notify(QObject *aReceiver, QEvent *aEvent) {
    __try {
        return reinterpret_cast<QApplication *>(this)->notify(aReceiver, aEvent);
    } __except (ExceptionFilter(GetExceptionInformation())) {
        abort();
    }
}
#else
// Cross-platform implementation for non-Windows platforms
// Note: SEH (__try/__except) is Windows-specific. On Unix platforms,
// exception safety mechanisms.
bool SafeQApplication::notify(QObject *aReceiver, QEvent *aEvent) {
    return QApplication::notify(aReceiver, aEvent);
}
#endif

//---------------------------------------------------------------------------
