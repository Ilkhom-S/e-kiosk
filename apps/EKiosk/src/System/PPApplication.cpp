/* @file Класс приложения для PaymentProcessor. */

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QDir>
#include <QtCore/QThread>
#include <QtCore/QThreadPool>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtGui/QSessionManager>

#ifdef Q_OS_WIN
#define NOMINMAX
#include <Windows.h>
#endif

#include <Common/ExitCodes.h>

#include <SysUtils/ISysUtils.h>

#include "PPApplication.h"
#include "Services/ServiceController.h"
#include "System/SettingsConstants.h"
#include "UnhandledException.h"

namespace PP = SDK::PaymentProcessor;

//------------------------------------------------------------------------
PPApplication::PPApplication(const QString &aName,
                             const QString &aVersion,
                             int &aArgumentCount,
                             char **aArguments)
    : BasicQtApplication<SafeQApplication>(aName, aVersion, aArgumentCount, aArguments),
      m_Protection("PaymentProcessorProtection") {
    catchUnhandledExceptions();

    // TODO: DEPRECATED - This QSharedMemory-based single-instance protection is a temporary
    // workaround for macOS until SingleApplication (thirdparty/SingleApplication) is fixed to work
    // correctly on macOS. Once SingleApplication is verified working, replace this entire block
    // with SingleApplication logic. See: https://github.com/itay-grudev/SingleApplication
    // Проверяем, не запущен ли уже другой экземпляр приложения.
    try {
        // Пытаемся создать shared memory segment. Если segment уже существует, create() вернет
        // false.
        if (!m_Protection.create(1)) {
            // Segment существует. Может быть другой экземпляр или остаток от краша.
            // Пытаемся attach/detach, чтобы очистить segment от процесса, который уже закончился.
            m_Protection.attach();
            m_Protection.detach();

            // Пытаемся создать снова. Если успешно - это был мертвый segment;
            // если нет - значит кто-то другой его занимает.
            if (!m_Protection.create(1)) {
                m_Protection.detach();

                ILog::getInstance(aName)->write(LogLevel::Warning,
                                                "Another instance of application was terminated.");

                throw std::runtime_error(
                    "Can't run application, because another instance already running.");
            }
        }
    } catch (...) {
        // Отсоединяемся от shared memory даже при ошибке, чтобы не оставить мертвый segment.
        m_Protection.detach();
        throw;
    }

    SafeQApplication::setStyle("plastique");
    SafeQApplication::setQuitOnLastWindowClosed(false);

    // Перенаправляем логи.
    QString appDest = BasicQtApplication::getLog()->getDestination();
    ILog::getInstance("CryptEngine")->setDestination(appDest);
    ILog::getInstance("DatabaseProxy")->setDestination(appDest);
    ILog::getInstance("MessageQueue")->setDestination(appDest);
    ILog::getInstance("Exceptions")->setDestination(appDest);
    ILog::getInstance("MessageQueueClient")->setDestination(appDest);
    ILog::getInstance("Plugins")->setDestination(appDest);

    m_ServiceController = new ServiceController(this);

    connect(m_ServiceController, SIGNAL(exit(int)), SLOT(exit(int)));
    connect(this, SIGNAL(screenshot()), SLOT(onScreenshot()));

    // Парсим параметры командной строки.
    foreach (const QString &parameter, BasicQtApplication::getArguments()) {
        m_Arguments.insert(parameter.section('=', 0, 0), parameter.section('=', 1, 1));
    }

    connect(qApp,
            SIGNAL(commitDataRequest(QSessionManager &)),
            this,
            SLOT(closeBySystem_Request(QSessionManager &)),
            Qt::DirectConnection);
}

//------------------------------------------------------------------------
PPApplication::~PPApplication() {
    m_Protection.detach();

    delete m_ServiceController;
}

//------------------------------------------------------------------------
int PPApplication::exec() {
    // Устанавливаем обработчик системных событий.
    QAbstractEventDispatcher::instance()->installNativeEventFilter(this);

    // блокируем скринсейвер
    ISysUtils::disableScreenSaver();

    if (m_ServiceController->initializeServices()) {
        return BasicQtApplication::exec();
    }

    LOG(getLog(), LogLevel::Error, "Failed to initialize PaymentProcessor.");

    // Выводим подробный отчет о запуске сервисов.
    m_ServiceController->dumpFailureReport();

    return ExitCode::Error;
}

//------------------------------------------------------------------------
SDK::PaymentProcessor::ICore *PPApplication::getCore() {
    return m_ServiceController;
}

//------------------------------------------------------------------------
void PPApplication::qtMessageHandler(QtMsgType /*aType*/,
                                     const QMessageLogContext & /*aContext*/,
                                     const QString &aMessage) {
    static ILog *log = ILog::getInstance("QtMessages");

    log->write(LogLevel::Normal, aMessage);
}

//------------------------------------------------------------------------
QList<QImage> PPApplication::getScreenshot() {
    m_Screenshots.clear();

    if (QThread::currentThread() != this->thread()) {
        m_ScreenshotMutex.lock();

        emit screenshot();

        if (!m_ScreenshotCondition.wait(&m_ScreenshotMutex, 5000)) {
            LOG(getLog(), LogLevel::Error, "Failed to get screenshot, the application is busy.");
        }

        m_ScreenshotMutex.unlock();
    } else {
        onScreenshot();
    }

    return m_Screenshots;
}

//------------------------------------------------------------------------
void PPApplication::onScreenshot() {
    QMutexLocker locker(&m_ScreenshotMutex);

#ifdef Q_OS_WIN
    const auto screens = QGuiApplication::screens();

    for (auto screen : screens) {
        auto geometry = screen->geometry();
        screen->grabWindow(0, geometry.left(), geometry.top(), geometry.width(), geometry.height());
    }
#else
    // Screenshot command is not implemented on this platform.
#endif // Q_OS_WIN

    m_ScreenshotCondition.wakeAll();
}

//------------------------------------------------------------------------
QVariantMap PPApplication::getArguments() const {
    return m_Arguments;
}

//------------------------------------------------------------------------
IApplication::AppInfo PPApplication::getAppInfo() const {
    AppInfo inf;
    inf.version = BasicQtApplication::getVersion();
    inf.appName = getName();
    inf.configuration = getSettings().value("common/configuration").toString();
    return inf;
}

//------------------------------------------------------------------------
QSettings &PPApplication::getSettings() const {
    return BasicQtApplication::getSettings();
}

//------------------------------------------------------------------------
ILog *PPApplication::getLog() const {
    return BasicQtApplication::getLog();
}

//------------------------------------------------------------------------
QString PPApplication::getUserDataPath() const {
    QString dataDir = getSettings().contains(CSettings::UserDataPath)
                          ? getSettings().value(CSettings::UserDataPath).toString()
                          : "user";

    return QDir::cleanPath(QDir::fromNativeSeparators(
        QDir::isAbsolutePath(dataDir) ? dataDir
                                      : BasicApplication::getWorkingDirectory() + "/" + dataDir));
}

//------------------------------------------------------------------------
QString PPApplication::getPluginPath() const {
    QString pluginDir = getSettings().contains(CSettings::PluginsPath)
                            ? getSettings().value(CSettings::PluginsPath).toString()
                            : "plugins";

    return QDir::cleanPath(QDir::fromNativeSeparators(
        QDir::isAbsolutePath(pluginDir)
            ? pluginDir
            : BasicApplication::getWorkingDirectory() + "/" + pluginDir));
}

//------------------------------------------------------------------------
QString PPApplication::getUserPluginPath() const {
    return QString("%1/bin").arg(getUserDataPath());
}

//------------------------------------------------------------------------
bool PPApplication::nativeEventFilter(const QByteArray &aEventType,
                                      void *aMessage,
                                      qintptr *aResult) {
#ifdef Q_OS_WIN
    MSG *message = (MSG *)aMessage;

    if (message) {
        switch (message->message) {
        case WM_SYSCOMMAND:
            if (message->wParam == SC_SCREENSAVE) {
                return true;
            }

            if (message->wParam == SC_MONITORPOWER) {
                /*
                Nothing todo ;)

                Sets the state of the display. This command supports devices that have power-saving
                features, such as a battery-powered personal computer.

                The lParam parameter can have the following values:
                -1 (the display is powering on)
                1 (the display is going to low power)
                2 (the display is being shut off)
                */

                return true;
            }
            break;

        case WM_POWERBROADCAST:
            break;
        }
    }
#else
    // Handling display screensaver/power management is not implemented for this platform!
#endif // Q_OS_WIN

    return false;
}

//------------------------------------------------------------------------
void PPApplication::exit(int aResultCode) const {
    LOG(getLog(), LogLevel::Debug, QString("Exit application with %1 code.").arg(aResultCode));

    qApp->exit(aResultCode);
}

//------------------------------------------------------------------------
void PPApplication::closeBySystem_Request(QSessionManager &aSessionManager) {
    // блокируем остановку системы.
    aSessionManager.cancel();

    LOG(getLog(), LogLevel::Warning, "SHUTDOWN service by system request.");

    // останавливаем систему самостоятельно
    getCore()->getEventService()->sendEvent(PP::Event(PP::EEventType::Shutdown));
}

//------------------------------------------------------------------------
